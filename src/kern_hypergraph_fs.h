#ifndef SRC_KERN_HYPERGRAPH_FS_H_
#define SRC_KERN_HYPERGRAPH_FS_H_

// kern_hypergraph_fs.h – Hypergraph filesystem API (Phase 1, Stage 1)
//
// Provides tensor-backed node allocation and edge management for the
// OpenCog AtomSpace hypergraph.  All allocations are bump-allocated from
// the GGML context that was initialised during boot Stage 0.
//
// Design notes
// ============
// • Nodes  – modelled as 1-D GGML F32 tensors.  The tensor's backing data
//   acts as the atom payload / feature vector.
// • Edges  – stored in a fixed-size global array (max HGFS_MAX_EDGES = 65536).
//   GGML contexts have no per-object free; freed nodes are tracked in a
//   separate global free-list so callers can query liveness.
// • Thread safety – none (kernel code runs in a single-threaded event loop).
//
// Performance targets
// ===================
//   hgfs_alloc             ≤ 100 ns
//   hgfs_free              ≤  50 ns
//   hgfs_edge              ≤ 200 ns
//   hgfs_query_neighbors   < 1 µs for 64 K edges

#include <stddef.h>
#include <stdint.h>

// Forward-declare rather than pulling in the full GGML header here so that
// consumers that only need the function signatures stay lean.
struct ggml_context;

#ifdef __cplusplus
extern "C" {
#endif

// ─── Constants ───────────────────────────────────────────────────────────────

/** Maximum number of live edges tracked globally. */
#define HGFS_MAX_EDGES    65536u

/** Maximum number of concurrently freed (dead) node pointers tracked. */
#define HGFS_MAX_FREELIST  8192u

// ─── API ─────────────────────────────────────────────────────────────────────

/**
 * hgfs_alloc – Allocate a hypergraph node backed by a 1-D GGML F32 tensor.
 *
 * Allocates a new 1-D tensor of type GGML_TYPE_F32 with @p size elements from
 * @p ctx.  The @p depth parameter records the P-system membrane depth and is
 * stored in the tensor's name field for later inspection.
 *
 * @param ctx   GGML context owning the memory pool.
 * @param size  Number of F32 elements in the tensor (node feature width).
 * @param depth Membrane depth in the P-system hierarchy (informational).
 * @return Pointer to the underlying ggml_tensor, or NULL on failure.
 *
 * Performance target: ≤ 100 ns (arena bump-allocation path).
 */
void *hgfs_alloc(struct ggml_context *ctx, size_t size, uint32_t depth);

/**
 * hgfs_free – Mark a hypergraph node as freed.
 *
 * GGML contexts do not support per-object deallocation; memory is reclaimed
 * only when the entire context is destroyed.  This function records @p ptr in
 * an internal free-list so that other subsystems can treat it as dead without
 * corrupting the arena.
 *
 * @param ctx  GGML context (used for validation; must not be NULL).
 * @param ptr  Pointer previously returned by hgfs_alloc().
 *
 * Performance target: ≤ 50 ns.
 */
void hgfs_free(struct ggml_context *ctx, void *ptr);

/**
 * hgfs_edge – Create a directed, typed, weighted edge between two nodes.
 *
 * Records a (src → dst, type, weight) edge in the global edge table.  Both
 * @p src and @p dst must be non-NULL and must NOT appear in the free-list.
 *
 * @param ctx    GGML context (used for validation).
 * @param src    Source node pointer (from hgfs_alloc).
 * @param dst    Destination node pointer (from hgfs_alloc).
 * @param type   Application-defined edge type (e.g. DEPENDS_ON = 1).
 * @param weight Edge weight / confidence value.
 * @return 0 on success, -1 if the edge table is full or parameters are invalid.
 *
 * Performance target: ≤ 200 ns.
 */
int hgfs_edge(struct ggml_context *ctx,
              void *src, void *dst,
              uint32_t type, float weight);

/**
 * hgfs_query_neighbors – Find all neighbours of a node matching a type filter.
 *
 * Scans the global edge table for edges whose source equals @p node and whose
 * type equals @p type_filter.  If @p type_filter is 0, all outgoing edges are
 * returned regardless of type.  Dead edges (either endpoint in free-list) are
 * silently skipped.
 *
 * @param ctx         GGML context (used for validation).
 * @param node        Source node to query.
 * @param type_filter Edge type to match; 0 means "all types".
 * @param results     Caller-supplied array of at least @p max_results pointers.
 * @param max_results Maximum number of neighbours to write into @p results.
 * @return Number of neighbours written to @p results, or -1 on error.
 *
 * Performance target: < 1 µs for up to HGFS_MAX_EDGES edges.
 */
int hgfs_query_neighbors(struct ggml_context *ctx,
                         void *node,
                         uint32_t type_filter,
                         void **results,
                         int max_results);

/**
 * hgfs_is_freed – Query whether a pointer is in the free-list.
 *
 * @param ptr  Pointer to test.
 * @return     Non-zero if @p ptr has been passed to hgfs_free(), 0 otherwise.
 */
int hgfs_is_freed(void *ptr);

/**
 * hgfs_reset – Reset all global hypergraph state (edges + free-list).
 *
 * Intended for unit-test teardown.  Does NOT free the GGML context.
 */
void hgfs_reset(void);

#ifdef __cplusplus
}
#endif

#endif  // SRC_KERN_HYPERGRAPH_FS_H_
