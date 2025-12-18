#ifndef SRC_KERN_BOOT_H_
#define SRC_KERN_BOOT_H_

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Forward declarations for GGML types
struct ggml_context;
struct uv_loop_s;

// Configuration for hypergraph filesystem
struct hgfs_config {
  // Maximum depth of P-system membranes
  uint32_t max_depth;
  
  // Initial number of atom slots
  size_t initial_atom_capacity;
  
  // Enable attention-based garbage collection
  bool enable_attention_gc;
};

// Configuration for DTESN scheduler
struct dtesn_config {
  // Size of echo state network reservoir
  uint32_t reservoir_size;
  
  // Scheduler tick rate in microseconds
  uint32_t tick_rate_us;
  
  // STI/LTI attention thresholds
  double sti_threshold;
  double lti_threshold;
  
  // Enable adaptive scheduling
  bool enable_adaptive_scheduling;
};

// Configuration for cognitive loop
struct cognitive_loop_config {
  // Cognitive cycle time in milliseconds
  uint64_t cycle_time_ms;
  
  // Attention decay rate per cycle
  double attention_decay_rate;
  
  // Enable automatic attention spreading
  bool enable_attention_spreading;
};

// Context for DTESN scheduler (opaque type)
struct dtesn_sched_context;

/**
 * Stage 0: Minimal Bootstrap
 * Initialize GGML context and allocate initial hypergraph tensor pool
 * 
 * @param ctx Output pointer to GGML context (caller receives ownership)
 * @param mem_size Initial memory pool size in bytes
 * @return 0 on success, negative error code on failure
 * 
 * Error codes:
 *  -1: Invalid parameters
 *  -2: Memory allocation failed
 *  -3: GGML initialization failed
 * 
 * Performance target: < 1ms initialization
 */
int kern_boot_stage0(struct ggml_context** ctx, size_t mem_size);

/**
 * Stage 1: Hypergraph Filesystem Initialization
 * Initialize hypergraph filesystem - allocator for AtomSpace tensors
 * 
 * @param ctx GGML context from Stage 0
 * @param config Hypergraph filesystem configuration
 * @return 0 on success, negative error code on failure
 * 
 * Error codes:
 *  -1: Invalid parameters
 *  -2: Context not initialized
 *  -3: Filesystem initialization failed
 * 
 * Performance target: < 2ms initialization
 */
int kern_boot_stage1_init_hypergraph_fs(struct ggml_context* ctx,
                                        struct hgfs_config* config);

/**
 * Stage 2: DTESN Scheduler Initialization
 * Initialize DTESN (Distributed Tensor Echo State Network) scheduler
 * 
 * @param sched Output scheduler context
 * @param config Scheduler configuration
 * @return 0 on success, negative error code on failure
 * 
 * Error codes:
 *  -1: Invalid parameters
 *  -2: Scheduler allocation failed
 *  -3: Reservoir initialization failed
 * 
 * Performance target: < 3ms initialization
 */
int kern_boot_stage2_init_scheduler(struct dtesn_sched_context** sched,
                                    struct dtesn_config* config);

/**
 * Stage 3: Event Loop Integration
 * Initialize event loop integration and cognitive cycle orchestration
 * 
 * @param loop libuv event loop
 * @param sched DTESN scheduler from Stage 2
 * @param config Cognitive loop configuration
 * @return 0 on success, negative error code on failure
 * 
 * Error codes:
 *  -1: Invalid parameters
 *  -2: Event loop integration failed
 *  -3: Cognitive loop initialization failed
 * 
 * Performance target: < 2ms initialization
 */
int kern_boot_stage3_init_cognitive_loop(struct uv_loop_s* loop,
                                         struct dtesn_sched_context* sched,
                                         struct cognitive_loop_config* config);

/**
 * Cleanup function for DTESN scheduler
 * 
 * @param sched Scheduler context to free
 */
void dtesn_sched_destroy(struct dtesn_sched_context* sched);

/**
 * Get kernel version information
 * 
 * @param major Output major version
 * @param minor Output minor version
 * @param patch Output patch version
 */
void kern_get_version(uint16_t* major, uint16_t* minor, uint16_t* patch);

#ifdef __cplusplus
}
#endif

#endif  // SRC_KERN_BOOT_H_
