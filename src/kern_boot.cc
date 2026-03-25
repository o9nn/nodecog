#include "kern_boot.h"
#include "kern_scheduler.h"
#include <cstring>
#include <cstdlib>
#include "deps/ggml/include/ggml.h"
#include "deps/ggml/include/ggml-alloc.h"
#include "uv.h"

// Kernel version
#define KERN_VERSION_MAJOR 0
#define KERN_VERSION_MINOR 1
#define KERN_VERSION_PATCH 0

// Global state for kernel subsystems
namespace {
  struct ggml_context* g_ggml_ctx = nullptr;
  bool g_hypergraph_fs_initialized = false;
  struct dtesn_sched_context* g_scheduler = nullptr;
  bool g_cognitive_loop_initialized = false;
}

// DTESN scheduler context structure
struct dtesn_sched_context {
  // Reservoir configuration
  uint32_t reservoir_size;
  uint32_t tick_rate_us;
  
  // Attention thresholds
  double sti_threshold;
  double lti_threshold;
  
  // Flags
  bool enable_adaptive_scheduling;
  
  // GGML tensors for reservoir state (to be implemented)
  struct ggml_context* ctx;
  struct ggml_tensor* reservoir_state;
  
  // libuv handles for event-loop integration
  uv_timer_t   timer;
  bool         timer_initialized;

  // uv_prepare fires before each I/O poll – used to call dtesn_sched_tick().
  uv_prepare_t prepare_handle;
  bool         prepare_initialized;

  // uv_idle fires when the loop is idle – used to decay attention values.
  uv_idle_t    idle_handle;
  bool         idle_initialized;
};

// Stage 0: Minimal Bootstrap
int kern_boot_stage0(struct ggml_context** ctx, size_t mem_size) {
  // Validate parameters
  if (ctx == nullptr) {
    return -1;  // Invalid parameters
  }
  
  if (mem_size < 1024 * 1024) {  // Minimum 1MB
    return -1;  // Invalid parameters
  }
  
  // Initialize GGML context
  struct ggml_init_params params = {
    .mem_size   = mem_size,
    .mem_buffer = nullptr,  // Let GGML allocate
    .no_alloc   = false
  };
  
  struct ggml_context* ggml_ctx = ggml_init(params);
  if (ggml_ctx == nullptr) {
    return -3;  // GGML initialization failed
  }
  
  // Store global context
  g_ggml_ctx = ggml_ctx;
  
  // Return context to caller
  *ctx = ggml_ctx;
  
  return 0;  // Success
}

// Stage 1: Hypergraph Filesystem Initialization
int kern_boot_stage1_init_hypergraph_fs(struct ggml_context* ctx,
                                        struct hgfs_config* config) {
  // Validate parameters
  if (ctx == nullptr || config == nullptr) {
    return -1;  // Invalid parameters
  }
  
  // Check if context is valid (basic sanity check)
  if (ctx != g_ggml_ctx) {
    return -2;  // Context not initialized by stage0
  }
  
  // Initialize hypergraph filesystem
  // For now, this is a stub that will be expanded with actual tensor allocations
  
  // Set default values if not provided
  if (config->max_depth == 0) {
    config->max_depth = 16;  // Default membrane depth
  }
  
  if (config->initial_atom_capacity == 0) {
    config->initial_atom_capacity = 10000;  // Default 10K atoms
  }
  
  // TODO: Allocate tensor structures for hypergraph nodes and edges
  // This will be implemented in Phase 1 after we have the full design
  
  g_hypergraph_fs_initialized = true;
  
  return 0;  // Success
}

// Stage 2: DTESN Scheduler Initialization
int kern_boot_stage2_init_scheduler(struct dtesn_sched_context** sched,
                                    struct dtesn_config* config) {
  // Validate parameters
  if (sched == nullptr || config == nullptr) {
    return -1;  // Invalid parameters
  }
  
  // Check dependencies
  if (!g_hypergraph_fs_initialized) {
    return -2;  // Stage 1 must be completed first
  }
  
  // Allocate scheduler context
  struct dtesn_sched_context* ctx = 
    static_cast<struct dtesn_sched_context*>(
      malloc(sizeof(struct dtesn_sched_context)));
  
  if (ctx == nullptr) {
    return -2;  // Scheduler allocation failed
  }
  
  // Initialize scheduler configuration
  memset(ctx, 0, sizeof(struct dtesn_sched_context));
  ctx->reservoir_size = config->reservoir_size > 0 ? config->reservoir_size : 1000;
  ctx->tick_rate_us = config->tick_rate_us > 0 ? config->tick_rate_us : 5;
  ctx->sti_threshold = config->sti_threshold > 0 ? config->sti_threshold : 0.5;
  ctx->lti_threshold = config->lti_threshold > 0 ? config->lti_threshold : 0.3;
  ctx->enable_adaptive_scheduling = config->enable_adaptive_scheduling;
  ctx->ctx = g_ggml_ctx;
  ctx->timer_initialized   = false;
  ctx->prepare_initialized = false;
  ctx->idle_initialized    = false;
  
  // TODO: Initialize ESN reservoir as GGML tensors
  // This will be implemented in Phase 5 (ESN Reservoir)
  ctx->reservoir_state = nullptr;
  
  // Store global scheduler
  g_scheduler = ctx;
  
  // Return context to caller
  *sched = ctx;
  
  return 0;  // Success
}

// uv_prepare callback: runs dtesn_sched_tick() before each I/O poll.
static void sched_prepare_callback(uv_prepare_t* handle) {
  (void)handle;
  dtesn_sched_tick();
}

// uv_idle callback: decays attention values during idle time.
static void sched_idle_callback(uv_idle_t* handle) {
  (void)handle;
  dtesn_sched_decay_attention(0.99);
}

// Timer callback for cognitive scheduler ticks
static void scheduler_tick_callback(uv_timer_t* handle) {
  // TODO: Implement actual scheduler tick logic
  // For now, this is a stub
  (void)handle;  // Unused
}

// Stage 3: Event Loop Integration
int kern_boot_stage3_init_cognitive_loop(struct uv_loop_s* loop,
                                         struct dtesn_sched_context* sched,
                                         struct cognitive_loop_config* config) {
  // Validate parameters
  if (loop == nullptr || sched == nullptr || config == nullptr) {
    return -1;  // Invalid parameters
  }
  
  // Check dependencies
  if (sched != g_scheduler) {
    return -2;  // Scheduler not initialized by stage2
  }
  
  // Initialize cognitive loop timer
  int result = uv_timer_init(loop, &sched->timer);
  if (result != 0) {
    return -2;  // Event loop integration failed
  }
  
  sched->timer_initialized = true;
  
  // Set default values if not provided
  uint64_t cycle_time_ms = config->cycle_time_ms > 0 ? config->cycle_time_ms : 1000;
  
  // Start the cognitive loop timer
  result = uv_timer_start(&sched->timer,
                          scheduler_tick_callback,
                          cycle_time_ms,  // Initial delay
                          cycle_time_ms);  // Repeat interval
  
  if (result != 0) {
    // Note: uv_close is asynchronous; the caller must keep the sched context
    // alive until the close callback fires (or run the loop to drain it).
    uv_close(reinterpret_cast<uv_handle_t*>(&sched->timer), nullptr);
    sched->timer_initialized = false;
    return -3;  // Cognitive loop initialization failed
  }

  // ── uv_prepare: call dtesn_sched_tick() before each I/O poll ──────────────
  result = uv_prepare_init(loop, &sched->prepare_handle);
  if (result != 0) {
    // -4: uv_prepare handle initialization failed
    uv_timer_stop(&sched->timer);
    uv_close(reinterpret_cast<uv_handle_t*>(&sched->timer), nullptr);
    sched->timer_initialized = false;
    return -4;
  }

  result = uv_prepare_start(&sched->prepare_handle, sched_prepare_callback);
  if (result != 0) {
    // -5: uv_prepare_start failed
    uv_close(reinterpret_cast<uv_handle_t*>(&sched->prepare_handle), nullptr);
    uv_timer_stop(&sched->timer);
    uv_close(reinterpret_cast<uv_handle_t*>(&sched->timer), nullptr);
    sched->timer_initialized = false;
    return -5;
  }
  sched->prepare_initialized = true;

  // ── uv_idle: decay attention when the loop is otherwise idle ──────────────
  result = uv_idle_init(loop, &sched->idle_handle);
  if (result != 0) {
    // -6: uv_idle handle initialization failed
    uv_prepare_stop(&sched->prepare_handle);
    uv_close(reinterpret_cast<uv_handle_t*>(&sched->prepare_handle), nullptr);
    sched->prepare_initialized = false;
    uv_timer_stop(&sched->timer);
    uv_close(reinterpret_cast<uv_handle_t*>(&sched->timer), nullptr);
    sched->timer_initialized = false;
    return -6;
  }

  result = uv_idle_start(&sched->idle_handle, sched_idle_callback);
  if (result != 0) {
    // -7: uv_idle_start failed
    uv_close(reinterpret_cast<uv_handle_t*>(&sched->idle_handle), nullptr);
    uv_prepare_stop(&sched->prepare_handle);
    uv_close(reinterpret_cast<uv_handle_t*>(&sched->prepare_handle), nullptr);
    sched->prepare_initialized = false;
    uv_timer_stop(&sched->timer);
    uv_close(reinterpret_cast<uv_handle_t*>(&sched->timer), nullptr);
    sched->timer_initialized = false;
    return -7;
  }
  sched->idle_initialized = true;
  
  g_cognitive_loop_initialized = true;
  
  return 0;  // Success
}

// Cleanup function for DTESN scheduler
void dtesn_sched_destroy(struct dtesn_sched_context* sched) {
  if (sched == nullptr) {
    return;
  }
  
  // Stop and close timer if initialized
  if (sched->timer_initialized) {
    uv_timer_stop(&sched->timer);
    uv_close(reinterpret_cast<uv_handle_t*>(&sched->timer), nullptr);
  }

  // Stop and close uv_prepare handle if initialized
  if (sched->prepare_initialized) {
    uv_prepare_stop(&sched->prepare_handle);
    uv_close(reinterpret_cast<uv_handle_t*>(&sched->prepare_handle), nullptr);
  }

  // Stop and close uv_idle handle if initialized
  if (sched->idle_initialized) {
    uv_idle_stop(&sched->idle_handle);
    uv_close(reinterpret_cast<uv_handle_t*>(&sched->idle_handle), nullptr);
  }
  
  // TODO: Free GGML tensors when implemented
  
  // Free scheduler context
  free(sched);
  
  // Clear global reference
  if (sched == g_scheduler) {
    g_scheduler = nullptr;
    g_cognitive_loop_initialized = false;
  }
}

// Get kernel version information
void kern_get_version(uint16_t* major, uint16_t* minor, uint16_t* patch) {
  if (major != nullptr) *major = KERN_VERSION_MAJOR;
  if (minor != nullptr) *minor = KERN_VERSION_MINOR;
  if (patch != nullptr) *patch = KERN_VERSION_PATCH;
}
