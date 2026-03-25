// kern_hypergraph_fs.cc – Hypergraph filesystem implementation (Phase 1, Stage 1)
//
// Implements tensor-backed node allocation and edge management for the
// OpenCog AtomSpace hypergraph.  See kern_hypergraph_fs.h for the full API
// contract and design notes.

#include "kern_hypergraph_fs.h"
#include "deps/ggml/include/ggml.h"

#include <cstring>
#include <cstdio>
#include <cstdint>
#include <cstddef>

// ─── Internal global state ───────────────────────────────────────────────────

namespace {

// ── Edge table ────────────────────────────────────────────────────────────────
//
// A flat array of edge records.  Edges are never removed; instead the active
// flag is cleared when an endpoint is freed.  A simple linear counter tracks
// the next free slot.

struct HgfsEdge {
  void    *src;       // source node pointer
  void    *dst;       // destination node pointer
  uint32_t type;      // application-defined edge type
  float    weight;    // edge weight / confidence
  bool     active;    // true while both endpoints are alive
};

static HgfsEdge g_edges[HGFS_MAX_EDGES];
static uint32_t g_edge_count = 0;   // total slots ever used (monotonic)

// ── Free-list ─────────────────────────────────────────────────────────────────
//
// Pointers marked freed.  Kept as a flat array; membership tests are O(n) but
// n ≤ 8192 and the test is branch-predicted well in typical workloads.

static void    *g_freelist[HGFS_MAX_FREELIST];
static uint32_t g_freelist_count = 0;

// ── Helpers ──────────────────────────────────────────────────────────────────

static bool is_freed_internal(void *ptr) {
  for (uint32_t i = 0; i < g_freelist_count; ++i) {
    if (g_freelist[i] == ptr) return true;
  }
  return false;
}

// Invalidate all edges that reference a freed pointer.
static void invalidate_edges_for(void *ptr) {
  for (uint32_t i = 0; i < g_edge_count; ++i) {
    if (g_edges[i].active &&
        (g_edges[i].src == ptr || g_edges[i].dst == ptr)) {
      g_edges[i].active = false;
    }
  }
}

}  // namespace

// ─── Public API ──────────────────────────────────────────────────────────────

extern "C" {

// hgfs_alloc ──────────────────────────────────────────────────────────────────

void *hgfs_alloc(struct ggml_context *ctx, size_t size, uint32_t depth) {
  if (ctx == nullptr || size == 0) {
    return nullptr;
  }

  // Allocate a 1-D F32 tensor from the GGML arena.
  // Cast size_t → int64_t; GGML uses int64_t for element counts.
  struct ggml_tensor *tensor =
      ggml_new_tensor_1d(ctx, GGML_TYPE_F32, (int64_t)size);

  if (tensor == nullptr) {
    return nullptr;
  }

  // Record the membrane depth in the tensor's name field so tooling can
  // inspect the P-system hierarchy without extra metadata storage.
  // tensor->name is 64 bytes; "hgfs:d" (6) + up to 10 digits for uint32_t
  // + NUL = 17 bytes max, well within the buffer.
  snprintf(tensor->name, sizeof(tensor->name), "hgfs:d%u", depth);

  return static_cast<void *>(tensor);
}

// hgfs_free ───────────────────────────────────────────────────────────────────

void hgfs_free(struct ggml_context *ctx, void *ptr) {
  if (ctx == nullptr || ptr == nullptr) {
    return;
  }

  // Idempotent – don't double-insert.
  if (is_freed_internal(ptr)) {
    return;
  }

  if (g_freelist_count >= HGFS_MAX_FREELIST) {
    // Free-list full; silently drop.  In production this would trigger
    // attention-based GC, but for now we just return.
    return;
  }

  g_freelist[g_freelist_count++] = ptr;

  // Mark all edges that touched this node as inactive.
  invalidate_edges_for(ptr);
}

// hgfs_edge ───────────────────────────────────────────────────────────────────

int hgfs_edge(struct ggml_context *ctx,
              void *src, void *dst,
              uint32_t type, float weight) {
  if (ctx == nullptr || src == nullptr || dst == nullptr) {
    return -1;
  }

  // Refuse to create edges involving freed nodes.
  if (is_freed_internal(src) || is_freed_internal(dst)) {
    return -1;
  }

  if (g_edge_count >= HGFS_MAX_EDGES) {
    return -1;  // edge table full
  }

  HgfsEdge *e  = &g_edges[g_edge_count++];
  e->src    = src;
  e->dst    = dst;
  e->type   = type;
  e->weight = weight;
  e->active = true;

  return 0;
}

// hgfs_query_neighbors ────────────────────────────────────────────────────────

int hgfs_query_neighbors(struct ggml_context *ctx,
                         void *node,
                         uint32_t type_filter,
                         void **results,
                         int max_results) {
  if (ctx == nullptr || node == nullptr ||
      results == nullptr || max_results <= 0) {
    return -1;
  }

  int found = 0;
  for (uint32_t i = 0; i < g_edge_count && found < max_results; ++i) {
    const HgfsEdge *e = &g_edges[i];
    if (!e->active) continue;
    if (e->src != node) continue;
    if (type_filter != 0 && e->type != type_filter) continue;

    results[found++] = e->dst;
  }

  return found;
}

// hgfs_is_freed ───────────────────────────────────────────────────────────────

int hgfs_is_freed(void *ptr) {
  return is_freed_internal(ptr) ? 1 : 0;
}

// hgfs_reset ──────────────────────────────────────────────────────────────────

void hgfs_reset(void) {
  memset(g_edges, 0, sizeof(g_edges));
  g_edge_count    = 0;
  memset(g_freelist, 0, sizeof(g_freelist));
  g_freelist_count = 0;
}

}  // extern "C"
