# Echo.Kern - Tensor-Based Cognitive Kernel

This document describes the planned high-performance C/C++ kernel layer that will accelerate NodeCog's cognitive operations using GGML tensors.

## Status: 📋 PLANNED (Q1 2026)

The Echo.Kern kernel is **not yet implemented**. This document describes the architecture and specifications for the planned implementation.

## Overview

Echo.Kern is a high-performance cognitive computing kernel that:
- Implements cognitive primitives as GGML tensor operations
- Provides 10-1000x performance improvements over JavaScript
- Enables GPU acceleration for pattern matching and reasoning
- Maintains backward compatibility through N-API bridge
- Supports hardware acceleration (CUDA, Metal, OpenCL, BLAS)

## Architecture Layers

```
┌──────────────────────────────────────────────────────────┐
│  JavaScript API (Unchanged)                              │
│  - AtomSpace, NodeSpace, AttentionBank, etc.             │
└────────────────┬─────────────────────────────────────────┘
                 │ N-API Bridge
┌────────────────▼─────────────────────────────────────────┐
│  Echo.Kern (C++ Kernel)                                  │
│  - Bootstrap (kern_boot_stage*)                          │
│  - Hypergraph FS (hgfs_*)                                │
│  - Scheduler (dtesn_sched_*)                             │
│  - PLN Inference (pln_*)                                 │
│  - ESN Reservoir (esn_*)                                 │
│  - Distributed Sync (dist_*)                             │
└────────────────┬─────────────────────────────────────────┘
                 │ GGML Tensor API
┌────────────────▼─────────────────────────────────────────┐
│  GGML Backend                                            │
│  - Tensor operations                                     │
│  - Memory management                                     │
└────────────────┬─────────────────────────────────────────┘
                 │ Hardware Abstraction
┌────────────────▼─────────────────────────────────────────┐
│  Hardware (CPU/GPU/TPU)                                  │
└──────────────────────────────────────────────────────────┘
```

## Kernel Functions (35 Primitives)

### Bootstrap Functions (Stage 0-3)

#### kern_boot_stage0()
**Purpose**: Initialize GGML context and memory pool

**Signature**:
```c
int kern_boot_stage0(
    struct ggml_context **ctx,     // OUT: GGML context
    size_t mem_size                // IN: Memory pool size (bytes)
);
```

**Performance Target**: < 1ms

**Implementation**:
```c
int kern_boot_stage0(struct ggml_context **ctx, size_t mem_size) {
    struct ggml_init_params params = {
        .mem_size = mem_size,
        .mem_buffer = NULL,  // Heap allocation
        .no_alloc = false
    };
    
    *ctx = ggml_init(params);
    if (*ctx == NULL) return -1;
    
    return 0;
}
```

#### kern_boot_stage1_init_hypergraph_fs()
**Purpose**: Initialize hypergraph filesystem (AtomSpace allocator)

**Signature**:
```c
int kern_boot_stage1_init_hypergraph_fs(
    struct ggml_context *ctx,      // IN: GGML context
    struct hgfs_config *config     // IN: Configuration
);
```

**Performance Target**: < 2ms

**Configuration**:
```c
struct hgfs_config {
    uint32_t max_depth;            // Max membrane depth (P-systems)
    uint32_t initial_size;         // Initial atom capacity
    float growth_factor;           // Growth multiplier (e.g., 1.5)
};
```

#### kern_boot_stage2_init_scheduler()
**Purpose**: Initialize DTESN cognitive scheduler

**Signature**:
```c
int kern_boot_stage2_init_scheduler(
    struct dtesn_sched_context **sched,  // OUT: Scheduler context
    struct dtesn_config *config          // IN: Configuration
);
```

**Performance Target**: < 3ms

**Configuration**:
```c
struct dtesn_config {
    uint32_t reservoir_size;       // ESN reservoir neurons
    uint32_t tick_rate_us;         // Microseconds per tick
    float spectral_radius;         // Reservoir dynamics
    float leak_rate;               // State decay rate
};
```

#### kern_boot_stage3_init_cognitive_loop()
**Purpose**: Integrate scheduler with libuv event loop

**Signature**:
```c
int kern_boot_stage3_init_cognitive_loop(
    struct dtesn_sched_context *sched,  // IN: Scheduler
    uv_loop_t *loop,                    // IN: libuv loop
    uint64_t interval_us                // IN: Cycle interval (µs)
);
```

**Performance Target**: < 2ms

**Total Bootstrap**: < 10ms for all 4 stages

### Hypergraph Filesystem Functions (hgfs_*)

#### hgfs_alloc()
**Purpose**: Allocate hypergraph node as GGML tensor

**Signature**:
```c
uint64_t hgfs_alloc(
    size_t feature_size,           // IN: Feature vector size
    uint32_t membrane_depth        // IN: P-system depth
);
```

**Returns**: Atom handle (64-bit ID)

**Performance Target**: ≤ 100ns

**Memory Layout**:
```c
struct hgfs_node {
    struct ggml_tensor *features;  // Feature vector
    float sti;                     // Short-term importance
    float lti;                     // Long-term importance
    float tv_strength;             // Truth value strength
    float tv_confidence;           // Truth value confidence
    uint32_t membrane_depth;       // Hierarchy level
    uint32_t type_id;              // Atom type
    char name[256];                // Atom name
};
```

#### hgfs_free()
**Purpose**: Deallocate hypergraph node

**Signature**:
```c
int hgfs_free(
    uint64_t handle,               // IN: Atom handle
    uint32_t membrane_depth        // IN: P-system depth
);
```

**Performance Target**: ≤ 50ns

#### hgfs_edge()
**Purpose**: Create hypergraph edge (link)

**Signature**:
```c
uint64_t hgfs_edge(
    uint64_t *sources,             // IN: Source atom handles
    size_t arity,                  // IN: Number of sources
    uint32_t type_id,              // IN: Link type
    float weight                   // IN: Link weight
);
```

**Returns**: Link handle

**Performance Target**: < 200ns

**Implementation**: Adjacency tensor representation

#### hgfs_query_neighbors()
**Purpose**: Query neighboring atoms (pattern match)

**Signature**:
```c
int hgfs_query_neighbors(
    uint64_t node,                 // IN: Query node
    uint32_t type_filter,          // IN: Link type (0 = any)
    uint64_t *results,             // OUT: Result handles
    size_t max_results             // IN: Max results
);
```

**Returns**: Number of results found

**Performance Target**: < 5µs for typical queries

### DTESN Scheduler Functions (dtesn_sched_*)

#### dtesn_sched_tick()
**Purpose**: Execute one scheduler tick

**Signature**:
```c
int dtesn_sched_tick(
    struct dtesn_sched_context *sched  // IN: Scheduler context
);
```

**Performance Target**: ≤ 5µs

**Algorithm**:
1. Update ESN reservoir state (matrix-vector multiply)
2. Compute attention scores (STI + LTI weighted)
3. Select highest-attention task (argmax)
4. Schedule task for execution
5. Apply attention decay

#### dtesn_sched_enqueue_task()
**Purpose**: Add task to scheduler queue

**Signature**:
```c
int dtesn_sched_enqueue_task(
    struct dtesn_sched_context *sched,  // IN: Scheduler
    struct task_descriptor *task        // IN: Task info
);
```

**Performance Target**: < 1µs

**Task Descriptor**:
```c
struct task_descriptor {
    void (*fn)(void *);            // Task function
    void *arg;                     // Arguments
    double sti;                    // Short-term importance
    double lti;                    // Long-term importance
    uint32_t membrane_depth;       // P-system level
    uint64_t deadline_ns;          // Real-time deadline (0=none)
};
```

#### dtesn_sched_update_attention()
**Purpose**: Update task attention values

**Signature**:
```c
int dtesn_sched_update_attention(
    struct dtesn_sched_context *sched,  // IN: Scheduler
    uint64_t task_id,                   // IN: Task ID
    double sti_delta,                   // IN: STI change
    double lti_delta                    // IN: LTI change
);
```

**Performance Target**: < 500ns

#### dtesn_sched_decay_attention()
**Purpose**: Apply attention decay to all tasks

**Signature**:
```c
int dtesn_sched_decay_attention(
    struct dtesn_sched_context *sched,  // IN: Scheduler
    float decay_rate                    // IN: Decay rate (0.0-1.0)
);
```

**Performance Target**: < 100µs for 10K tasks

**Implementation**: Vectorized tensor operation

### PLN Inference Functions (pln_*)

#### pln_deduce()
**Purpose**: Deduction rule (A→B, B→C ⊢ A→C)

**Signature**:
```c
int pln_deduce(
    uint64_t premise1,             // IN: A→B
    uint64_t premise2,             // IN: B→C
    uint64_t *conclusion           // OUT: A→C
);
```

**Performance Target**: < 10µs

**Algorithm**:
1. Get truth values of premises
2. Apply PLN deduction formula:
   - strength_AC = strength_AB * strength_BC
   - confidence_AC = confidence_AB * confidence_BC * k
3. Create conclusion atom
4. Set truth value

#### pln_induce()
**Purpose**: Induction rule (frequent A→B ⊢ general A→B)

**Signature**:
```c
int pln_induce(
    uint64_t *instances,           // IN: Observed instances
    size_t count,                  // IN: Number of instances
    uint64_t *general_rule         // OUT: General rule
);
```

**Performance Target**: < 50µs

#### pln_pattern_match()
**Purpose**: High-performance pattern matching

**Signature**:
```c
int pln_pattern_match(
    struct pattern_spec *pattern,  // IN: Pattern to match
    uint64_t *results,             // OUT: Matching atoms
    size_t max_results             // IN: Max results
);
```

**Performance Target**: < 100µs (500x faster than JS)

**Implementation**: GPU-accelerated graph traversal

### ESN Reservoir Functions (esn_*)

#### esn_init()
**Purpose**: Initialize Echo State Network reservoir

**Signature**:
```c
int esn_init(
    struct esn_context **ctx,      // OUT: ESN context
    uint32_t input_size,           // IN: Input dimension
    uint32_t reservoir_size,       // IN: Reservoir neurons
    uint32_t output_size,          // IN: Output dimension
    float spectral_radius,         // IN: Reservoir dynamics
    float leak_rate                // IN: State decay
);
```

**Performance Target**: < 5ms

#### esn_update()
**Purpose**: Update reservoir state

**Signature**:
```c
int esn_update(
    struct esn_context *ctx,       // IN: ESN context
    const float *input,            // IN: Input vector
    float *state                   // OUT: Updated state
);
```

**Performance Target**: < 1ms for typical sizes

**Implementation**: Sparse matrix-vector multiply (GGML op)

#### esn_compute_output()
**Purpose**: Compute output from reservoir state

**Signature**:
```c
int esn_compute_output(
    struct esn_context *ctx,       // IN: ESN context
    const float *state,            // IN: Reservoir state
    float *output                  // OUT: Output vector
);
```

**Performance Target**: < 500µs

#### esn_train()
**Purpose**: Train output weights (ridge regression)

**Signature**:
```c
int esn_train(
    struct esn_context *ctx,       // IN: ESN context
    const float *states,           // IN: Training states (matrix)
    const float *targets,          // IN: Target outputs (matrix)
    size_t samples,                // IN: Number of samples
    float regularization           // IN: Ridge parameter
);
```

**Performance Target**: < 100ms for typical datasets

### Distributed Sync Functions (dist_*)

#### dist_atomspace_sync()
**Purpose**: Synchronize atoms across nodes

**Signature**:
```c
int dist_atomspace_sync(
    struct dist_context *ctx,      // IN: Distribution context
    uint64_t *atom_handles,        // IN: Atoms to sync
    size_t count                   // IN: Number of atoms
);
```

**Performance Target**: < 1ms per 100 atoms

**Protocol**:
1. Serialize atoms (wire format)
2. Compute diff (changed atoms only)
3. Transmit to peers (TCP/IPC)
4. Receive updates from peers
5. Merge with conflict resolution
6. Apply to local AtomSpace

#### dist_attention_propagate()
**Purpose**: Propagate attention across nodes

**Signature**:
```c
int dist_attention_propagate(
    struct dist_context *ctx,      // IN: Distribution context
    uint64_t atom_handle,          // IN: Source atom
    float sti_delta,               // IN: STI change
    float lti_delta                // IN: LTI change
);
```

**Performance Target**: < 500µs

#### dist_conflict_resolve()
**Purpose**: Resolve concurrent modifications

**Signature**:
```c
int dist_conflict_resolve(
    uint64_t atom_handle,          // IN: Conflicted atom
    struct atom_version *local,    // IN: Local version
    struct atom_version *remote,   // IN: Remote version
    struct atom_version *merged    // OUT: Merged version
);
```

**Strategy**: Truth value merging or LWW based on config

**Performance Target**: < 10µs

## Performance Targets

### Comparison: JavaScript vs. Kernel

| Operation | JavaScript | Kernel Target | Improvement |
|-----------|-----------|---------------|-------------|
| Add Node | ~1-2 µs | < 5 µs | Tensor-based |
| Add Link | ~2-3 µs | < 10 µs | Tensor-based |
| Pattern Match (10 nodes) | ~50 ms | < 100 µs | **500x faster** |
| Pattern Match (100 nodes) | ~5 s | < 10 ms | **500x faster** |
| Attention Spread (1K atoms) | ~100 ms | < 100 µs | **1000x faster** |
| Attention Spread (10K atoms) | ~10 s | < 10 ms | **1000x faster** |
| Scheduler Tick | ~300 µs | ≤ 5 µs | **60x faster** |
| Memory Allocation | V8 GC | ≤ 100 ns | Custom allocator |
| Distributed Sync (100 atoms) | N/A | < 1 ms | New capability |
| PLN Inference | ~1-10 ms | < 10 µs | **100-1000x** |

### Hardware Acceleration

**CPU**:
- AVX2/AVX512: 2-4x speedup on x86
- NEON: 2-3x speedup on ARM
- Vectorized operations for tensor math

**GPU** (via CUDA/Metal):
- Pattern matching: 10-100x speedup
- Attention spreading: 50-200x speedup
- Matrix operations: 100-1000x speedup

**BLAS Libraries**:
- OpenBLAS: General CPU
- Intel MKL: Intel CPUs (2-3x over OpenBLAS)
- Apple Accelerate: Apple Silicon (2-3x)

## Integration with Node.js

### N-API Bridge

JavaScript calls kernel functions via N-API:

```javascript
// JavaScript API (unchanged)
const concept = atomspace.addNode('CONCEPT', 'JavaScript');

// Internally:
if (process.config.variables.use_kernel_backend) {
    // Call kernel via N-API
    const handle = kernelBindings.kern_atomspace_add_node(
        'CONCEPT',
        'JavaScript',
        null  // truth value
    );
    return new Atom(handle, this);
} else {
    // Fallback to JavaScript implementation
    return this._addNodeJS('CONCEPT', 'JavaScript', null);
}
```

### N-API Function Exports

```cpp
// src/cognitive_kernel_napi_bridge.cc
napi_value InitKernelBindings(napi_env env, napi_value exports) {
    // Bootstrap
    EXPORT_NAPI_FUNCTION(env, exports, "kernBootStage0", KernBootStage0);
    
    // Hypergraph FS
    EXPORT_NAPI_FUNCTION(env, exports, "hgfsAlloc", HgfsAlloc);
    EXPORT_NAPI_FUNCTION(env, exports, "hgfsFree", HgfsFree);
    EXPORT_NAPI_FUNCTION(env, exports, "hgfsEdge", HgfsEdge);
    EXPORT_NAPI_FUNCTION(env, exports, "hgfsQueryNeighbors", HgfsQueryNeighbors);
    
    // Scheduler
    EXPORT_NAPI_FUNCTION(env, exports, "dtesnSchedTick", DtesnSchedTick);
    EXPORT_NAPI_FUNCTION(env, exports, "dtesnSchedEnqueue", DtesnSchedEnqueue);
    
    // PLN
    EXPORT_NAPI_FUNCTION(env, exports, "plnDeduce", PlnDeduce);
    EXPORT_NAPI_FUNCTION(env, exports, "plnPatternMatch", PlnPatternMatch);
    
    // ESN
    EXPORT_NAPI_FUNCTION(env, exports, "esnInit", EsnInit);
    EXPORT_NAPI_FUNCTION(env, exports, "esnUpdate", EsnUpdate);
    
    // Distribution
    EXPORT_NAPI_FUNCTION(env, exports, "distAtomspaceSync", DistAtomspaceSync);
    
    return exports;
}

NODE_API_MODULE(cognitive_kernel, InitKernelBindings)
```

### Feature Flag

Enable kernel backend:

```bash
# Environment variable
NODE_OPENCOG_KERNEL_BACKEND=1 node app.js

# Or build configuration
./configure --experimental-kernel-backend
make
```

## Build System Integration

### GGML as Dependency

Add to `node.gyp`:

```python
{
    'target_name': 'node',
    'sources': [
        # ... existing sources
        
        # GGML
        'deps/ggml/ggml.c',
        'deps/ggml/ggml-alloc.c',
        
        # Kernel
        'src/kern_boot.cc',
        'src/kern_hypergraph_fs.cc',
        'src/kern_scheduler.cc',
        'src/kern_pln.cc',
        'src/kern_esn.cc',
        'src/kern_distributed.cc',
        'src/cognitive_kernel_napi_bridge.cc',
    ],
    'libraries': [
        '-lblas',  # or '-framework Accelerate' on macOS
    ],
    'conditions': [
        ['OS=="mac"', {
            'libraries': ['-framework Accelerate'],
        }],
        ['use_cuda==1', {
            'sources': ['deps/ggml/ggml-cuda.cu'],
            'libraries': ['-lcudart', '-lcublas'],
        }],
    ],
}
```

### Platform Support

- **Linux**: x86-64, ARM64, with CUDA optional
- **macOS**: x86-64, ARM64 (Apple Silicon), Metal optional
- **Windows**: x86-64, with CUDA optional

## Testing Strategy

### Unit Tests (C++)

```cpp
// test/cctest/test_kern_boot.cc
TEST(KernBootTest, Stage0InitializesContext) {
    struct ggml_context *ctx = nullptr;
    int result = kern_boot_stage0(&ctx, 128 * 1024 * 1024);
    
    EXPECT_EQ(result, 0);
    EXPECT_NE(ctx, nullptr);
    
    ggml_free(ctx);
}
```

### Integration Tests (JavaScript)

```javascript
// test/parallel/test-opencog-kernel-backend.js
const assert = require('assert');
const { createCognitiveSystem } = require('lib/opencog');

// Requires NODE_OPENCOG_KERNEL_BACKEND=1
if (process.config.variables.use_kernel_backend) {
    const system = createCognitiveSystem();
    
    // Test kernel-backed operations
    const concept = system.atomspace.addNode('CONCEPT', 'Test');
    assert(concept.id > 0);
    
    // Performance test
    const start = process.hrtime.bigint();
    for (let i = 0; i < 10000; i++) {
        system.atomspace.addNode('CONCEPT', `node_${i}`);
    }
    const end = process.hrtime.bigint();
    const ns = Number(end - start);
    const avgNs = ns / 10000;
    
    console.log(`Average add node: ${avgNs}ns`);
    assert(avgNs < 5000); // < 5µs target
}
```

### Benchmark Suite

```javascript
// benchmark/opencog/pattern-match-kernel.js
const { createCognitiveSystem } = require('lib/opencog');

const system = createCognitiveSystem();

// Create test graph
for (let i = 0; i < 1000; i++) {
    const concept = system.atomspace.addNode('CONCEPT', `concept_${i}`);
    for (let j = 0; j < 10; j++) {
        const target = system.atomspace.addNode('CONCEPT', `target_${j}`);
        system.atomspace.addLink('INHERITANCE', [concept, target]);
    }
}

// Benchmark pattern match
const iterations = 100;
const start = process.hrtime.bigint();

for (let i = 0; i < iterations; i++) {
    const results = system.atomspace.patternMatch({
        type: 'INHERITANCE',
        outgoing: [{ type: 'CONCEPT' }, { type: 'CONCEPT' }]
    });
}

const end = process.hrtime.bigint();
const avgMs = Number(end - start) / 1000000 / iterations;

console.log(`Average pattern match: ${avgMs}ms`);
console.log(`Target: < 0.1ms (${avgMs < 0.1 ? 'PASS' : 'FAIL'})`);
```

## Implementation Timeline

### Phase 0 (Q1 2026) - 10-12 weeks

**Week 1-2**: Infrastructure
- Vendor GGML into `deps/ggml`
- Modify `node.gyp` build system
- Setup CI/CD for multi-platform builds

**Week 3-4**: Bootstrap (Stage 0-3)
- Implement `kern_boot_stage*()` functions
- GGML context initialization
- Basic validation tests

**Week 5-6**: Hypergraph FS
- Implement `hgfs_*()` functions
- Tensor-based node allocation
- Adjacency list for edges
- Pattern matching prototype

**Week 7-8**: DTESN Scheduler
- Implement `dtesn_sched_*()` functions
- ESN reservoir integration
- Attention-based scheduling
- Performance tuning

**Week 9**: PLN and ESN
- Implement `pln_*()` functions
- Implement `esn_*()` functions
- Inference algorithms
- Reservoir computing

**Week 10**: Distributed Sync
- Implement `dist_*()` functions
- Serialization protocol
- Conflict resolution

**Week 11**: N-API Bridge
- JavaScript bindings for all functions
- Feature flag integration
- Fallback to JS implementation

**Week 12**: Testing and Documentation
- Comprehensive unit tests
- Integration tests
- Performance benchmarks
- User documentation

## Resources Required

- **Engineers**: 2-3 senior C++ developers
- **Hardware**: GPU machines for CUDA development
- **CI/CD**: Multi-platform build infrastructure
- **Timeline**: 10-12 weeks full-time

## Success Criteria

- [ ] All 35 kernel functions implemented
- [ ] Performance targets met (within 2x acceptable)
- [ ] Zero regressions in JavaScript tests
- [ ] Multi-platform support (Linux, macOS, Windows)
- [ ] 90%+ test coverage
- [ ] Complete documentation
- [ ] Successful integration tests
- [ ] Benchmark suite passing

## References

- `KERNEL_FUNCTION_MANIFEST.md` - Complete function specifications
- `KERNEL_STATUS_REPORT.md` - Implementation status
- `KERNEL_INTEGRATION_GUIDE.md` - Integration matrix
- `DEVELOPMENT_ROADMAP.md` - Phase 0 details
