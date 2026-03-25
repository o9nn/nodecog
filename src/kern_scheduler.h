#ifndef SRC_KERN_SCHEDULER_H_
#define SRC_KERN_SCHEDULER_H_

// kern_scheduler.h – DTESN task-scheduler and attention-bank API (Phase 1, Stage 2)
//
// Provides a lightweight, global task scheduler and attention-allocation
// subsystem for the DTESN (Distributed Tensor Echo State Network) cognitive
// scheduler.  The implementation deliberately uses global state so that it can
// be called from C, libuv callbacks, and the cognitive loop without needing to
// thread a context pointer through every call site.
//
// Design notes
// ============
// • Task queue  – binary max-heap keyed on priority (double), capacity 1024.
// • Attention   – open-addressed hash table keyed on atom pointer, capacity 4096.
// • Thread safety – none; designed for single-threaded libuv event loop use.
//
// Performance targets
// ===================
//   dtesn_sched_enqueue_task    < 1 µs
//   dtesn_sched_tick            ≤ 5 µs
//   dtesn_sched_update_attention ≤ 1 µs
//   dtesn_sched_decay_attention  < 100 µs for 4096 atoms

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// ─── Constants ───────────────────────────────────────────────────────────────

/** Maximum number of tasks in the priority queue. */
#define DTESN_MAX_TASKS      1024u

/** Capacity of the attention hash table (must be power of 2). */
#define DTESN_ATTN_CAPACITY  4096u

// ─── API ─────────────────────────────────────────────────────────────────────

/**
 * dtesn_sched_enqueue_task – Push a task onto the max-priority heap.
 *
 * @param task_data  Opaque pointer to task payload (not owned, not freed).
 * @param priority   Scheduling priority; higher value = higher urgency.
 * @return 0 on success, -1 if the task queue is full (DTESN_MAX_TASKS).
 *
 * Performance target: < 1 µs (O(log N) heap insertion).
 */
int dtesn_sched_enqueue_task(void *task_data, double priority);

/**
 * dtesn_sched_tick – Process one scheduler tick.
 *
 * Pops the highest-priority task from the heap (if any), marks it as
 * processed, and increments the global tick counter.  Intended to be called
 * from a libuv uv_prepare_t callback before each I/O poll.
 *
 * Performance target: ≤ 5 µs.
 */
void dtesn_sched_tick(void);

/**
 * dtesn_sched_update_attention – Update the STI of an atom in the table.
 *
 * Looks up @p atom_ptr in the global attention hash table and adds
 * @p delta_sti to its current STI value.  If the atom is not yet in the
 * table it is inserted with STI = @p delta_sti.
 *
 * @param atom_ptr  Pointer used as the atom identifier key.
 * @param delta_sti Signed delta to add to the atom's STI.
 *
 * Performance target: ≤ 1 µs (O(1) amortised hash lookup).
 */
void dtesn_sched_update_attention(void *atom_ptr, double delta_sti);

/**
 * dtesn_sched_decay_attention – Apply multiplicative decay to all STI values.
 *
 * Multiplies every active atom's STI by @p decay_factor.  Intended to be
 * called from a libuv uv_idle_t callback (idle time maintenance).
 *
 * @param decay_factor  Value in (0, 1]; e.g. 0.99 applies 1 % decay per call.
 *
 * Performance target: < 100 µs for DTESN_ATTN_CAPACITY atoms.
 */
void dtesn_sched_decay_attention(double decay_factor);

/**
 * dtesn_sched_get_tick_count – Return the number of ticks executed so far.
 *
 * @return Monotonically increasing tick counter, incremented by each call to
 *         dtesn_sched_tick() that finds at least one task.
 */
uint64_t dtesn_sched_get_tick_count(void);

/**
 * dtesn_sched_get_attention – Query the current STI of an atom.
 *
 * @param atom_ptr  Atom pointer to look up.
 * @return Current STI, or 0.0 if the atom is not in the table.
 */
double dtesn_sched_get_attention(void *atom_ptr);

/**
 * dtesn_sched_reset – Reset all global scheduler state.
 *
 * Clears the task heap, attention table, and tick counter.
 * Intended for unit-test teardown only.
 */
void dtesn_sched_reset(void);

#ifdef __cplusplus
}
#endif

#endif  // SRC_KERN_SCHEDULER_H_
