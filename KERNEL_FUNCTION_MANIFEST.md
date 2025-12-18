# Echo.Kern Function Manifest

## Overview

This manifest defines the kernel-level primitives that map OpenCog cognitive subsystems to high-performance C/C++ implementations using GGML tensors and llama.cpp backends. Each function represents a core cognitive operation that operates on tensorized hypergraph data structures.

---

## Core Kernel Primitives (10 Categories)

### 1. Boot & Initialization

#### `kern_boot_stage0()`
**Status**: Planned  
**Priority**: CRITICAL  
**Description**: Minimal bootstrap - initialize GGML context and allocate initial hypergraph tensor pool  
**Prototype**:
```c
int kern_boot_stage0(struct ggml_context **ctx, size_t mem_size);
```
**Parameters**:
- `ctx`: Output pointer to GGML context (caller receives ownership)
- `mem_size`: Initial memory pool size in bytes

**Returns**: 0 on success, negative error code on failure  
**Cleanup**: Caller must call `ggml_free(ctx)` on shutdown  
**Error Handling**: On failure, *ctx is set to NULL and error code indicates failure reason  
**Dependencies**: None (bare metal entry point)  
**Performance Target**: < 1ms initialization  
**Integration**: Called from Node.js process startup before V8 initialization

---

#### `kern_boot_stage1_init_hypergraph_fs()`
**Status**: Planned  
**Priority**: CRITICAL  
**Description**: Initialize hypergraph filesystem - allocator for AtomSpace tensors  
**Prototype**:
```c
int kern_boot_stage1_init_hypergraph_fs(struct ggml_context *ctx, 
                                         struct hgfs_config *config);
```
**Parameters**:
- `ctx`: GGML context from Stage0
- `config`: Hypergraph filesystem configuration (depth limits, membrane counts)

**Returns**: 0 on success, negative error code on failure  
**Dependencies**: `kern_boot_stage0`  
**Performance Target**: < 2ms initialization  
**Integration**: Provides memory allocator for AtomSpace nodes and links

---

#### `kern_boot_stage2_init_scheduler()`
**Status**: Planned  
**Priority**: CRITICAL  
**Description**: Initialize DTESN (Distributed Tensor Echo State Network) scheduler  
**Prototype**:
```c
int kern_boot_stage2_init_scheduler(struct dtesn_sched_context **sched,
                                     struct dtesn_config *config);
```
**Parameters**:
- `sched`: Output scheduler context
- `config`: Scheduler configuration (reservoir size, tick rate, attention thresholds)

**Returns**: 0 on success, negative error code on failure  
**Dependencies**: `kern_boot_stage1_init_hypergraph_fs`  
**Performance Target**: < 3ms initialization  
**Integration**: Cognitive Synergy Engine scheduler backend

---

#### `kern_boot_stage3_init_cognitive_loop()`
**Status**: Planned  
**Priority**: HIGH  
**Description**: Initialize event loop integration and cognitive cycle orchestration  
**Prototype**:
```c
int kern_boot_stage3_init_cognitive_loop(struct uv_loop_s *loop,
                                          struct dtesn_sched_context *sched,
                                          struct cognitive_loop_config *config);
```
**Parameters**:
- `loop`: libuv event loop
- `sched`: DTESN scheduler from Stage2
- `config`: Cognitive loop configuration (cycle time, attention decay rate)

**Returns**: 0 on success, negative error code on failure  
**Dependencies**: `kern_boot_stage2_init_scheduler`  
**Performance Target**: < 2ms initialization  
**Integration**: Wires cognitive scheduler into libuv event loop phases

---

### 2. Scheduling & Task Management

#### `dtesn_sched_tick()`
**Status**: Planned  
**Priority**: CRITICAL  
**Description**: Execute one scheduler tick - select and execute highest-attention task  
**Prototype**:
```c
int dtesn_sched_tick(struct dtesn_sched_context *sched, uint64_t timestamp_ns);
```
**Parameters**:
- `sched`: Scheduler context
- `timestamp_ns`: Current timestamp in nanoseconds

**Returns**: Number of tasks executed (0 if idle)  
**Dependencies**: `kern_boot_stage2_init_scheduler`  
**Performance Target**: ≤ 5µs per tick  
**Integration**: Called from libuv prepare phase

---

#### `dtesn_sched_enqueue_task()`
**Status**: Planned  
**Priority**: CRITICAL  
**Description**: Enqueue task with STI/LTI attention values  
**Prototype**:
```c
int dtesn_sched_enqueue_task(struct dtesn_sched_context *sched,
                              struct task_descriptor *task);

struct task_descriptor {
    void (*fn)(void *arg);
    void *arg;
    double sti;  // Short-term importance
    double lti;  // Long-term importance
    uint32_t membrane_depth;
    uint64_t deadline_ns;  // Real-time deadline (0 = none)
};
```
**Returns**: Task ID on success, negative error code on failure  
**Performance Target**: < 1µs enqueue latency  
**Integration**: Used by V8 isolate scheduler and agent orchestrator

---

#### `dtesn_sched_update_attention()`
**Status**: Planned  
**Priority**: HIGH  
**Description**: Update STI/LTI for running task based on resource usage  
**Prototype**:
```c
int dtesn_sched_update_attention(struct dtesn_sched_context *sched,
                                  uint64_t task_id, 
                                  double sti_delta, 
                                  double lti_delta);
```
**Performance Target**: < 500ns per update  
**Integration**: Called during task execution for adaptive scheduling

---

#### `dtesn_sched_decay_attention()`
**Status**: Planned  
**Priority**: MEDIUM  
**Description**: Apply attention decay to all scheduled tasks (forgetting)  
**Prototype**:
```c
int dtesn_sched_decay_attention(struct dtesn_sched_context *sched, 
                                 double decay_rate);
```
**Performance Target**: < 100µs for 10,000 tasks  
**Integration**: Called from libuv timer phase every cognitive cycle

---

### 3. Memory Management (Hypergraph Filesystem)

#### `hgfs_alloc()`
**Status**: Planned  
**Priority**: CRITICAL  
**Description**: Allocate memory as GGML tensor node in hypergraph  
**Prototype**:
```c
void *hgfs_alloc(size_t size, uint32_t depth);
```
**Parameters**:
- `size`: Size in bytes
- `depth`: Membrane depth (P-system hierarchy level)

**Returns**: Pointer to allocated memory, NULL on failure  
**Dependencies**: `kern_boot_stage1_init_hypergraph_fs`  
**Performance Target**: ≤ 100ns allocation  
**Integration**: Used by AtomSpace for node/link allocation

---

#### `hgfs_free()`
**Status**: Planned  
**Priority**: CRITICAL  
**Description**: Free hypergraph tensor node  
**Prototype**:
```c
void hgfs_free(void *ptr, uint32_t depth);
```
**Performance Target**: ≤ 50ns deallocation  
**Integration**: Attention-based garbage collection trigger

---

#### `hgfs_edge()`
**Status**: Planned  
**Priority**: CRITICAL  
**Description**: Create edge between hypergraph nodes (link)  
**Prototype**:
```c
int hgfs_edge(void *src, void *dst, uint8_t link_type, double weight);
```
**Parameters**:
- `src`, `dst`: Node pointers from hgfs_alloc
- `link_type`: Edge type (INHERITANCE, SIMILARITY, DEPENDS_ON, etc.)
- `weight`: Edge weight (truth value strength)

**Returns**: Edge ID on success, negative on failure  
**Performance Target**: < 200ns per edge  
**Integration**: AtomSpace link creation backend

---

#### `hgfs_query_neighbors()`
**Status**: Planned  
**Priority**: HIGH  
**Description**: Query neighbors of hypergraph node (pattern matching primitive)  
**Prototype**:
```c
int hgfs_query_neighbors(void *node, uint8_t link_type, 
                          void **results, size_t max_results);
```
**Returns**: Number of neighbors found  
**Performance Target**: < 5µs for degree-100 nodes  
**Integration**: Pattern matcher and inference engine backend

---

### 4. Interrupts & Signals

#### `kern_register_interrupt_handler()`
**Status**: Planned  
**Priority**: HIGH  
**Description**: Register cognitive interrupt handler (async event → attention spike)  
**Prototype**:
```c
int kern_register_interrupt_handler(uint32_t interrupt_id,
                                     void (*handler)(void *ctx),
                                     void *context);
```
**Performance Target**: < 1µs handler dispatch  
**Integration**: V8 microtask queue and libuv async callbacks

---

#### `kern_trigger_interrupt()`
**Status**: Planned  
**Priority**: HIGH  
**Description**: Trigger cognitive interrupt (boost attention on related atoms)  
**Prototype**:
```c
int kern_trigger_interrupt(uint32_t interrupt_id, void *data);
```
**Performance Target**: < 2µs latency to attention update  
**Integration**: Error handling, I/O completion, user signals

---

### 5. System Calls (Cognitive Services)

#### `kern_syscall_atomspace_add_node()`
**Status**: Planned  
**Priority**: CRITICAL  
**Description**: System call to add node to AtomSpace  
**Prototype**:
```c
uint64_t kern_syscall_atomspace_add_node(const char *type, const char *name,
                                          struct truth_value *tv);
```
**Returns**: Atom handle (tensor node ID)  
**Performance Target**: < 5µs  
**Integration**: JavaScript AtomSpace.addNode() backend

---

#### `kern_syscall_atomspace_add_link()`
**Status**: Planned  
**Priority**: CRITICAL  
**Description**: System call to add link to AtomSpace  
**Prototype**:
```c
uint64_t kern_syscall_atomspace_add_link(const char *type, 
                                          uint64_t *targets, 
                                          size_t arity,
                                          struct truth_value *tv);
```
**Returns**: Atom handle (link ID)  
**Performance Target**: < 10µs  
**Integration**: JavaScript AtomSpace.addLink() backend

---

#### `kern_syscall_pattern_match()`
**Status**: Planned  
**Priority**: HIGH  
**Description**: Pattern matching system call (backward chaining)  
**Prototype**:
```c
int kern_syscall_pattern_match(struct pattern_template *pattern,
                                uint64_t *results, 
                                size_t max_results);
```
**Returns**: Number of matches found  
**Performance Target**: < 100µs for simple patterns  
**Integration**: PLN inference backend

---

### 6. I/O Operations

#### `kern_io_read_atomspace_snapshot()`
**Status**: Planned  
**Priority**: MEDIUM  
**Description**: Load AtomSpace from disk snapshot (GGUF format)  
**Prototype**:
```c
int kern_io_read_atomspace_snapshot(const char *path, 
                                     struct ggml_context *ctx);
```
**Performance Target**: < 100ms for 10MB snapshot  
**Integration**: Persistent knowledge base loading

---

#### `kern_io_write_atomspace_snapshot()`
**Status**: Planned  
**Priority**: MEDIUM  
**Description**: Save AtomSpace to disk snapshot (GGUF format)  
**Prototype**:
```c
int kern_io_write_atomspace_snapshot(const char *path, 
                                      struct ggml_context *ctx);
```
**Performance Target**: < 100ms for 10MB snapshot  
**Integration**: Knowledge base checkpointing

---

#### `kern_io_sync_distributed_atomspace()`
**Status**: Planned  
**Priority**: HIGH  
**Description**: Synchronize AtomSpace with remote nodes (IPC)  
**Prototype**:
```c
int kern_io_sync_distributed_atomspace(const char *remote_addr,
                                        uint16_t port);
```
**Performance Target**: < 10ms for 1000 atoms  
**Integration**: Distributed NodeSpace backend

---

### 7. Synchronization Primitives

#### `kern_sync_atomspace_lock()`
**Status**: Planned  
**Priority**: HIGH  
**Description**: Acquire lock on AtomSpace region (membrane-aware)  
**Prototype**:
```c
int kern_sync_atomspace_lock(uint64_t atom_handle, uint32_t membrane_id);
```
**Performance Target**: < 100ns (lock-free path)  
**Integration**: Multi-threaded cognitive agent coordination

---

#### `kern_sync_atomspace_unlock()`
**Status**: Planned  
**Priority**: HIGH  
**Description**: Release AtomSpace lock  
**Prototype**:
```c
int kern_sync_atomspace_unlock(uint64_t atom_handle, uint32_t membrane_id);
```
**Performance Target**: < 100ns  

---

#### `kern_sync_barrier()`
**Status**: Planned  
**Priority**: MEDIUM  
**Description**: Synchronization barrier for parallel cognitive agents  
**Prototype**:
```c
int kern_sync_barrier(uint32_t agent_count);
```
**Performance Target**: < 1µs per agent  
**Integration**: AgentOrchestrator batch execution

---

### 8. Timers & Temporal Reasoning

#### `kern_timer_create_cognitive_timer()`
**Status**: Planned  
**Priority**: MEDIUM  
**Description**: Create timer for cognitive cycle or agent scheduling  
**Prototype**:
```c
int kern_timer_create_cognitive_timer(uint64_t interval_ns, 
                                       void (*callback)(void *),
                                       void *context);
```
**Performance Target**: ± 1µs timer accuracy  
**Integration**: CognitiveLoop tick generation

---

#### `kern_timer_get_timestamp()`
**Status**: Planned  
**Priority**: LOW  
**Description**: Get high-resolution timestamp for attention decay  
**Prototype**:
```c
uint64_t kern_timer_get_timestamp(void);
```
**Performance Target**: < 20ns (RDTSC or clock_gettime)  
**Integration**: Temporal reasoning and event ordering

---

### 9. Protection & Security

#### `kern_protect_sandbox_create()`
**Status**: Planned  
**Priority**: MEDIUM  
**Description**: Create isolated sandbox for agent execution  
**Prototype**:
```c
int kern_protect_sandbox_create(struct sandbox_context **sandbox,
                                 struct sandbox_config *config);
```
**Performance Target**: < 5ms sandbox creation  
**Integration**: ESM agent arena isolation

---

#### `kern_protect_check_membrane_boundary()`
**Status**: Planned  
**Priority**: MEDIUM  
**Description**: Validate cross-membrane memory access  
**Prototype**:
```c
int kern_protect_check_membrane_boundary(void *ptr, uint32_t from_depth, 
                                          uint32_t to_depth);
```
**Performance Target**: < 100ns check  
**Integration**: P-system membrane security enforcement

---

### 10. ABI & Language Bindings

#### `kern_abi_get_version()`
**Status**: Planned  
**Priority**: LOW  
**Description**: Get kernel ABI version for compatibility checking  
**Prototype**:
```c
struct kern_version kern_abi_get_version(void);

struct kern_version {
    uint16_t major;
    uint16_t minor;
    uint16_t patch;
    const char *git_hash;
};
```
**Integration**: Dynamic library loading and version negotiation

---

#### `kern_abi_export_to_napi()`
**Status**: Planned  
**Priority**: HIGH  
**Description**: Export kernel functions to N-API for JavaScript access  
**Prototype**:
```c
int kern_abi_export_to_napi(napi_env env, napi_value exports);
```
**Integration**: N-API bridge for cognitive_napi_bridge.cc

---

## PLN (Probabilistic Logic Networks) Kernel Functions

### `pln_eval_tensor()`
**Status**: Planned  
**Priority**: HIGH  
**Description**: Evaluate PLN inference rule as GGML tensor operation  
**Prototype**:
```c
int pln_eval_tensor(struct ggml_tensor *premises, 
                    struct ggml_tensor *conclusion,
                    enum pln_rule rule_type);
```
**Performance Target**: < 10µs per inference  
**Integration**: InferenceAgent backend

---

### `pln_unify_graph()`
**Status**: Planned  
**Priority**: HIGH  
**Description**: Graph unification for pattern matching (tensor-based)  
**Prototype**:
```c
int pln_unify_graph(struct ggml_tensor *pattern, 
                    struct ggml_tensor *target,
                    struct unification_result *result);
```
**Performance Target**: < 50µs for 10-node patterns  
**Integration**: Pattern matcher kernel

---

## ESN (Echo State Network) Reservoir Functions

### `esn_reservoir_init()`
**Status**: Planned  
**Priority**: MEDIUM  
**Description**: Initialize ESN reservoir as GGML tensor  
**Prototype**:
```c
int esn_reservoir_init(struct ggml_context *ctx, 
                       struct esn_config *config,
                       struct ggml_tensor **reservoir);
```
**Performance Target**: < 10ms for 1000-neuron reservoir  
**Integration**: DTESN scheduler state representation

---

### `esn_reservoir_update()`
**Status**: Planned  
**Priority**: MEDIUM  
**Description**: Update reservoir state (recurrent dynamics)  
**Prototype**:
```c
int esn_reservoir_update(struct ggml_tensor *reservoir,
                         struct ggml_tensor *input,
                         struct ggml_tensor *output);
```
**Performance Target**: < 1ms per update (1000 neurons)  
**Integration**: Scheduler dynamics and learning

---

## MOSES (Meta-Optimizing Semantic Evolutionary Search) Functions

### `moses_evolve_program()`
**Status**: Future  
**Priority**: LOW  
**Description**: Evolve cognitive program using genetic operators  
**Prototype**:
```c
int moses_evolve_program(struct moses_population *pop,
                         struct fitness_function *fitness,
                         uint32_t generations);
```
**Integration**: Agent optimization and program synthesis

---

## Distributed AtomSpace Functions

### `dist_atomspace_sync()`
**Status**: Planned  
**Priority**: MEDIUM  
**Description**: Synchronize atoms across distributed nodes  
**Prototype**:
```c
int dist_atomspace_sync(struct dist_atomspace_context *ctx,
                        uint64_t *atom_handles, 
                        size_t count);
```
**Performance Target**: < 1ms per 100 atoms over network  
**Integration**: Distributed NodeSpace backend

---

### `dist_attention_propagate()`
**Status**: Planned  
**Priority**: MEDIUM  
**Description**: Propagate attention values across distributed nodes  
**Prototype**:
```c
int dist_attention_propagate(struct dist_atomspace_context *ctx,
                             uint64_t atom_handle, 
                             double sti_delta);
```
**Performance Target**: < 5ms propagation latency  
**Integration**: Distributed attention allocation

---

## Statistics & Monitoring

### `kern_stats_get_metrics()`
**Status**: Planned  
**Priority**: LOW  
**Description**: Get kernel performance metrics  
**Prototype**:
```c
int kern_stats_get_metrics(struct kern_metrics *metrics);

struct kern_metrics {
    uint64_t scheduler_ticks;
    uint64_t atoms_allocated;
    uint64_t atoms_freed;
    uint64_t attention_updates;
    uint64_t pattern_matches;
    double avg_tick_latency_ns;
    double avg_alloc_latency_ns;
};
```
**Integration**: Dashboard and monitoring

---

## Summary

| Category | Functions | Status | Priority |
|----------|-----------|--------|----------|
| Boot | 4 | Planned | CRITICAL |
| Scheduling | 4 | Planned | CRITICAL |
| Memory | 4 | Planned | CRITICAL |
| Interrupts | 2 | Planned | HIGH |
| Syscalls | 3 | Planned | CRITICAL |
| I/O | 3 | Planned | MEDIUM |
| Sync | 3 | Planned | HIGH |
| Timers | 2 | Planned | MEDIUM |
| Protection | 2 | Planned | MEDIUM |
| ABI | 2 | Planned | HIGH |
| PLN | 2 | Planned | HIGH |
| ESN | 2 | Planned | MEDIUM |
| MOSES | 1 | Future | LOW |
| Distributed | 2 | Planned | MEDIUM |
| Stats | 1 | Planned | LOW |
| **Total** | **35** | **0% Complete** | - |

---

**Document Version**: 1.0.0  
**Last Updated**: 2025-12-18  
**Status**: Manifest Complete - Implementation Pending  
**Target**: C99/C++17 with GGML + llama.cpp backends
