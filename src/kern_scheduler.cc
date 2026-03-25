// kern_scheduler.cc – DTESN task-scheduler and attention-bank (Phase 1, Stage 2)
//
// Implements a priority-queue task scheduler and an open-addressed hash-table
// attention bank.  All state is global so libuv callbacks and the cognitive
// loop can reach it without a context pointer.
//
// See kern_scheduler.h for the full API contract.

#include "kern_scheduler.h"

#include <cstring>
#include <cstdint>
#include <cstddef>

// ─── Internal structures ─────────────────────────────────────────────────────

namespace {

// ── Task max-heap ─────────────────────────────────────────────────────────────

struct Task {
  void  *data;
  double priority;
};

// Heap stored as a 1-based array (index 0 unused) for cleaner parent/child
// arithmetic.
static Task     g_heap[DTESN_MAX_TASKS + 1];  // +1 for 1-based indexing
static uint32_t g_heap_size = 0;

// Heap helpers ─────────────────────────────────────────────────────────────

static inline void heap_swap(uint32_t a, uint32_t b) {
  Task tmp   = g_heap[a];
  g_heap[a] = g_heap[b];
  g_heap[b] = tmp;
}

static void heap_sift_up(uint32_t idx) {
  while (idx > 1) {
    uint32_t parent = idx / 2;
    if (g_heap[parent].priority < g_heap[idx].priority) {
      heap_swap(parent, idx);
      idx = parent;
    } else {
      break;
    }
  }
}

static void heap_sift_down(uint32_t idx) {
  while (true) {
    uint32_t largest = idx;
    uint32_t left    = idx * 2;
    uint32_t right   = idx * 2 + 1;

    if (left  <= g_heap_size && g_heap[left].priority  > g_heap[largest].priority)
      largest = left;
    if (right <= g_heap_size && g_heap[right].priority > g_heap[largest].priority)
      largest = right;

    if (largest != idx) {
      heap_swap(largest, idx);
      idx = largest;
    } else {
      break;
    }
  }
}

// ── Attention hash table ──────────────────────────────────────────────────────
//
// Open-addressed table using linear probing.  The key is the raw atom pointer
// value; the value is the current STI (double).

struct AttnEntry {
  void  *atom_ptr;  // NULL ⇒ slot empty
  double sti;
  bool   active;
};

static AttnEntry g_attn[DTESN_ATTN_CAPACITY];
// Number of active entries; used by decay_attention for early exit when sparse.
static uint32_t  g_attn_active = 0;

// FNV-1a hash of the pointer value, folded to fit the table.
static inline uint32_t ptr_hash(void *ptr) {
  uintptr_t v = (uintptr_t)ptr;
  // FNV-1a 64-bit seed folded to 32-bit
  uint64_t h = UINT64_C(14695981039346656037);  // FNV-1a 64-bit offset basis
  for (int i = 0; i < (int)sizeof(uintptr_t); ++i) {
    h ^= (uint8_t)(v & 0xFF);
    h *= UINT64_C(1099511628211);
    v >>= 8;
  }
  return (uint32_t)(h ^ (h >> 32)) & (DTESN_ATTN_CAPACITY - 1);
}

static AttnEntry *attn_find_or_insert(void *atom_ptr) {
  if (atom_ptr == nullptr) return nullptr;

  uint32_t idx = ptr_hash(atom_ptr);
  for (uint32_t probe = 0; probe < DTESN_ATTN_CAPACITY; ++probe) {
    uint32_t   slot = (idx + probe) & (DTESN_ATTN_CAPACITY - 1);
    AttnEntry *e    = &g_attn[slot];

    if (!e->active) {
      // Empty slot – insert here.
      e->atom_ptr = atom_ptr;
      e->sti      = 0.0;
      e->active   = true;
      ++g_attn_active;
      return e;
    }
    if (e->atom_ptr == atom_ptr) {
      return e;  // found existing entry
    }
  }
  return nullptr;  // table full
}

// ── Tick counter ──────────────────────────────────────────────────────────────

static uint64_t g_tick_count = 0;

}  // namespace

// ─── Public API ──────────────────────────────────────────────────────────────

extern "C" {

// dtesn_sched_enqueue_task ────────────────────────────────────────────────────

int dtesn_sched_enqueue_task(void *task_data, double priority) {
  // NOTE: task_data is stored as an opaque pointer only; the caller retains
  // ownership and must keep the pointed-to data alive until the task has been
  // dequeued and processed by dtesn_sched_tick().  No copy is made here.
  if (g_heap_size >= DTESN_MAX_TASKS) {
    return -1;  // queue full
  }

  // Insert at next position (1-based), then sift up to restore heap property.
  uint32_t pos      = ++g_heap_size;
  g_heap[pos].data     = task_data;
  g_heap[pos].priority = priority;
  heap_sift_up(pos);

  return 0;
}

// dtesn_sched_tick ────────────────────────────────────────────────────────────

void dtesn_sched_tick(void) {
  if (g_heap_size == 0) {
    return;  // nothing to process
  }

  // The max-priority task is always at index 1 (root of the heap).
  // "Process" it: for now we just record that it ran (no-op payload
  // execution) and then remove it from the heap.
  // Future: dispatch task_data to appropriate handler.

  // Swap root with last element and sift down.
  heap_swap(1, g_heap_size);
  --g_heap_size;
  if (g_heap_size > 0) {
    heap_sift_down(1);
  }

  ++g_tick_count;
}

// dtesn_sched_update_attention ────────────────────────────────────────────────

void dtesn_sched_update_attention(void *atom_ptr, double delta_sti) {
  // A null atom_ptr is silently ignored: the caller may legitimately pass
  // a null sentinel to mean "no atom" without it being an error.
  if (atom_ptr == nullptr) return;

  AttnEntry *e = attn_find_or_insert(atom_ptr);
  if (e != nullptr) {
    e->sti += delta_sti;
  }
  // If the table is full (e == nullptr) the update is silently dropped.
  // Production code would trigger GC here.
}

// dtesn_sched_decay_attention ─────────────────────────────────────────────────

void dtesn_sched_decay_attention(double decay_factor) {
  // Early exit when the table is empty – common during startup.
  if (g_attn_active == 0) return;

  uint32_t seen = 0;
  for (uint32_t i = 0; i < DTESN_ATTN_CAPACITY && seen < g_attn_active; ++i) {
    if (g_attn[i].active) {
      g_attn[i].sti *= decay_factor;
      ++seen;
    }
  }
}

// dtesn_sched_get_tick_count ──────────────────────────────────────────────────

uint64_t dtesn_sched_get_tick_count(void) {
  return g_tick_count;
}

// dtesn_sched_get_attention ───────────────────────────────────────────────────

double dtesn_sched_get_attention(void *atom_ptr) {
  if (atom_ptr == nullptr) return 0.0;

  uint32_t idx = ptr_hash(atom_ptr);
  for (uint32_t probe = 0; probe < DTESN_ATTN_CAPACITY; ++probe) {
    uint32_t         slot = (idx + probe) & (DTESN_ATTN_CAPACITY - 1);
    const AttnEntry *e    = &g_attn[slot];

    if (!e->active) break;  // linear probe ended without finding
    if (e->atom_ptr == atom_ptr) return e->sti;
  }
  return 0.0;
}

// dtesn_sched_reset ───────────────────────────────────────────────────────────

void dtesn_sched_reset(void) {
  memset(g_heap,  0, sizeof(g_heap));
  g_heap_size  = 0;
  memset(g_attn,  0, sizeof(g_attn));
  g_attn_active = 0;
  g_tick_count  = 0;
}

}  // extern "C"
