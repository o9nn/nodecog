# Integration Guide

## Getting Started

This guide shows you how to integrate the Cognitive Synergy Engine into your application.

## Prerequisites

Before you begin, ensure you have:

- **Node.js v18+** with embeddable API support
- **CMake 3.10+** for building C++ components
- **C++17 compatible compiler** (GCC 7+, Clang 5+, MSVC 2017+)
- **Git** for cloning repositories

## Installation

### Option 1: Copy the Starter Template

The fastest way to get started:

```bash
# Copy the starter template to your project
cp -r cognitive-synergy-starter my-cognitive-app
cd my-cognitive-app

# Build the C++ embedder
mkdir build && cd build
cmake ..
make

# Test it works
./cognitive-embedder
```

### Option 2: Build from Node.js Source

To use the latest version integrated with Node.js:

```bash
# Clone Node.js with Cognitive Synergy Engine
git clone https://github.com/o9nn/nodecog
cd nodecog

# Build Node.js
./configure
make -j4

# The cognitive synergy engine is now built into Node.js
```

## Integration Paths

### Path A: JavaScript Integration (Recommended)

Use the JavaScript API for most applications.

#### Basic Setup

```javascript
const { createEngine } = require('./lib/cognitive_api');

// Create engine
const engine = createEngine({
  cognitiveTick: 5,
  attentionBased: true,
});

// Create isolates
const reasoning = engine.createIsolate('reasoning', { sti: 100 });
const perception = engine.createIsolate('perception', { sti: 80 });

// Use isolates
reasoning.boost(20);
console.log(`Reasoning STI: ${reasoning.getSTI()}`);
```

#### With NodeSpace Integration

Enable module dependency tracking:

```bash
# Run with ESM loader
node --experimental-loader ./lib/loader.mjs app.js
```

Your imports will be tracked automatically:

```javascript
// app.js
import express from 'express';
import lodash from 'lodash';

// These imports are now tracked in NodeSpace
// Attention values update automatically
```

### Path B: C++ Embedder Integration

For applications that need to embed Node.js.

#### Minimal Embedder

```cpp
#include "cognitive_platform.h"
#include "cognitive_synergy_engine.h"

int main() {
  // 1. Create platform
  cognitive::PlatformConfig platform_config;
  platform_config.worker_threads = 4;
  platform_config.cognitive_tick_ms = 5;
  
  cognitive::CognitivePlatform platform(platform_config);
  platform.Initialize();
  
  // 2. Create engine
  node::cognitive::CognitiveSynergyConfig engine_config;
  engine_config.attention_based_scheduling = true;
  
  node::cognitive::CognitiveSynergyEngine engine(engine_config);
  engine.Initialize();
  
  // 3. Install hooks
  platform.InstallCognitiveHooks(
    []() { /* prepare */ },
    []() { /* check */ },
    []() { /* tick */ },
    []() { /* idle */ }
  );
  
  // 4. Create isolates
  auto* agent = engine.CreateIsolate("agent");
  agent->SetSTI(100.0);
  
  // 5. Run
  return platform.Run();
}
```

#### CMake Integration

```cmake
cmake_minimum_required(VERSION 3.10)
project(MyApp)

# Find Node.js
set(NODE_ROOT "/path/to/nodecog")
include_directories(
  ${NODE_ROOT}/src
  ${NODE_ROOT}/deps/v8/include
  ${NODE_ROOT}/deps/uv/include
)

# Add sources
add_executable(myapp
  main.cc
  ${NODE_ROOT}/src/cognitive_synergy_engine.cc
)

# Link libraries
target_link_libraries(myapp
  pthread
  dl
)
```

## Common Integration Patterns

### Pattern 1: Multi-Agent System

Create isolates for different agents:

```javascript
const agents = {
  reasoner: engine.createIsolate('reasoner', { sti: 100, lti: 90 }),
  planner: engine.createIsolate('planner', { sti: 80, lti: 85 }),
  learner: engine.createIsolate('learner', { sti: 60, lti: 95 }),
};

// Agents communicate via SharedArrayBuffer
const mailbox = engine.createSharedBuffer(4096);

// Agent 1 sends
agents.reasoner.run(() => {
  const data = new Int32Array(mailbox);
  Atomics.store(data, 0, 42);
  Atomics.notify(data, 0);
});

// Agent 2 receives
agents.planner.run(() => {
  const data = new Int32Array(mailbox);
  Atomics.wait(data, 0, 0);
  const value = Atomics.load(data, 0);
});
```

### Pattern 2: Priority-Based Execution

Control execution order via attention:

```javascript
// High-priority critical path
const critical = engine.createIsolate('critical', { sti: 150 });

// Medium-priority normal work
const normal = engine.createIsolate('normal', { sti: 50 });

// Low-priority background tasks
const background = engine.createIsolate('background', { sti: 10 });

// Critical runs first due to highest STI
// Attention automatically decays over time
```

### Pattern 3: Resource-Aware Scheduling

Monitor and adjust based on resources:

```javascript
// Monitor memory usage
setInterval(() => {
  agents.forEach(agent => {
    const memory = agent.getMemoryUsage();
    
    if (memory > threshold) {
      // High memory pressure - reduce priority
      agent.decay(10);
    } else {
      // Low memory - can boost if needed
      agent.boost(5);
    }
  });
}, 1000);
```

### Pattern 4: Attention-Based Caching

Use attention to decide what to cache:

```javascript
import { getModuleGraph } from './lib/loader.mjs';

// Get module attention values
const graph = getModuleGraph();

// Cache high-attention modules
graph.modules
  .filter(mod => mod.sti > 80)
  .forEach(mod => {
    preloadModule(mod.url);
    pinInMemory(mod.url);
  });

// Evict low-attention modules
graph.modules
  .filter(mod => mod.sti < 20)
  .forEach(mod => {
    evictFromCache(mod.url);
  });
```

## Advanced Integration

### Custom Scheduler

Implement custom scheduling policy:

```cpp
class MyScheduler : public node::cognitive::CognitiveScheduler {
 public:
  IsolateContext* SelectNextIsolate() override {
    // Your custom logic
    // Examples:
    // - Deadline scheduling
    // - Fair share
    // - Priority queues
    // - ML-based prediction
    
    return selected_isolate;
  }
};
```

### V8 Snapshots

Speed up isolate creation with snapshots:

```cpp
// Create snapshot with preloaded code
v8::StartupData CreateCustomSnapshot() {
  v8::SnapshotCreator creator;
  v8::Isolate* isolate = creator.GetIsolate();
  {
    v8::HandleScope scope(isolate);
    v8::Local<v8::Context> context = v8::Context::New(isolate);
    v8::Context::Scope context_scope(context);
    
    // Load your libraries
    CompileRun("const myLib = require('mylib');");
    
    creator.SetDefaultContext(context);
  }
  
  return creator.CreateBlob(
      v8::SnapshotCreator::FunctionCodeHandling::kClear);
}

// Use snapshot
v8::Isolate::CreateParams params;
params.snapshot_blob = &snapshot;
v8::Isolate* isolate = v8::Isolate::New(params);
```

### Worker Thread Integration

Cognitive control over Worker Threads:

```javascript
const { Worker } = require('worker_threads');

// Create worker with cognitive control
const worker = new Worker('./worker.js', {
  workerData: {
    isolateId: 'worker-1',
    initialSTI: 70,
  }
});

// Worker inherits cognitive scheduling
worker.on('message', (msg) => {
  // Handle results
});
```

### AtomSpace Integration

Connect to OpenCog AtomSpace:

```javascript
// Assuming OpenCog bindings are available
const { AtomSpace, AttentionBank } = require('opencog');

const atomspace = new AtomSpace();
const attention = new AttentionBank(atomspace);

// Sync attention between atoms and isolates
function syncAttention(atom, isolate) {
  const atomSTI = attention.getSTI(atom);
  isolate.setSTI(atomSTI);
}

// Create atom for each isolate
const concept = atomspace.addNode('CONCEPT', 'ReasoningAgent');
syncAttention(concept, reasoning);
```

## Performance Tuning

### Cognitive Tick Interval

```javascript
// Faster ticks = more responsive but higher overhead
const engine = createEngine({ cognitiveTick: 1 });  // 1ms

// Slower ticks = lower overhead but less responsive
const engine = createEngine({ cognitiveTick: 10 }); // 10ms

// Default (recommended)
const engine = createEngine({ cognitiveTick: 5 });  // 5ms
```

### Microtask Limits

```javascript
// Control how many microtasks run per slice
const engine = createEngine({
  maxMicrotasks: 50,   // Fewer = more context switches
                       // More = longer slices
});
```

### Worker Threads

```javascript
// Match to CPU cores for best performance
const engine = createEngine({
  workerThreads: require('os').cpus().length
});
```

### Memory Limits

```cpp
// Set per-isolate memory limits
v8::Isolate::CreateParams params;
params.constraints.set_max_old_space_size(100);  // 100MB
params.constraints.set_max_young_space_size(10); // 10MB
```

## Debugging

### Enable Verbose Logging

```cpp
cognitive::PlatformConfig config;
config.verbose = true;
```

### Chrome DevTools Integration

```javascript
// Enable inspector (if available)
if (engine.enableInspector) {
  engine.enableInspector(9229);
  console.log('Inspector running on port 9229');
  console.log('Connect via chrome://inspect');
}
```

### Statistics Monitoring

```javascript
setInterval(() => {
  const stats = engine.getStatistics();
  console.log('Engine Stats:', {
    isolates: stats.isolateCount,
    memory: (stats.totalMemory / 1024 / 1024).toFixed(1) + 'MB',
    avgSTI: stats.averageSTI.toFixed(2),
  });
}, 5000);
```

## Testing

### Unit Tests

```javascript
const assert = require('assert');
const { createEngine } = require('./lib/cognitive_api');

describe('Cognitive Synergy Engine', () => {
  let engine;
  
  beforeEach(() => {
    engine = createEngine({ cognitiveTick: 5 });
  });
  
  afterEach(() => {
    engine.destroy();
  });
  
  it('creates isolates', () => {
    const isolate = engine.createIsolate('test', { sti: 100 });
    assert.strictEqual(isolate.id, 'test');
    assert.strictEqual(isolate.getSTI(), 100);
  });
  
  it('manages attention', () => {
    const isolate = engine.createIsolate('test', { sti: 50 });
    isolate.boost(20);
    assert.strictEqual(isolate.getSTI(), 70);
    isolate.decay(30);
    assert.strictEqual(isolate.getSTI(), 40);
  });
});
```

### Integration Tests

```javascript
// Test multi-isolate coordination
it('coordinates multiple isolates', async () => {
  const isolate1 = engine.createIsolate('sender', { sti: 100 });
  const isolate2 = engine.createIsolate('receiver', { sti: 80 });
  
  const buffer = engine.createSharedBuffer(4);
  const data = new Int32Array(buffer);
  
  // Simulate communication
  Atomics.store(data, 0, 42);
  assert.strictEqual(Atomics.load(data, 0), 42);
});
```

## Troubleshooting

### Engine Won't Initialize

```javascript
try {
  engine.initialize();
} catch (error) {
  console.error('Initialization failed:', error);
  // Check:
  // - Node.js version (18+)
  // - Native bindings compiled
  // - Sufficient memory
}
```

### High Memory Usage

```javascript
// Check per-isolate memory
engine.getAllIsolates().forEach(isolate => {
  const mb = isolate.getMemoryUsage() / 1024 / 1024;
  console.log(`${isolate.id}: ${mb.toFixed(1)}MB`);
  
  if (mb > 100) {
    // Reduce LTI to limit heap size
    isolate.setLTI(Math.max(10, isolate.getLTI() - 20));
  }
});
```

### Slow Performance

```javascript
// 1. Check cognitive tick rate
// 2. Reduce microtask limit
// 3. Increase worker threads
// 4. Profile with Node.js profiler

const engine = createEngine({
  cognitiveTick: 10,      // Slower ticks
  maxMicrotasks: 50,      // Fewer per slice
  workerThreads: 8,       // More workers
});
```

## Production Deployment

### Recommendations

1. **Build Release Mode**
   ```bash
   cmake -DCMAKE_BUILD_TYPE=Release ..
   make
   ```

2. **Set Resource Limits**
   ```javascript
   // Limit total isolates
   const MAX_ISOLATES = 100;
   
   // Limit per-isolate memory
   const MAX_MEMORY_MB = 100;
   ```

3. **Monitor Metrics**
   ```javascript
   // Export to monitoring system
   setInterval(() => {
     const stats = engine.getStatistics();
     metrics.gauge('isolates.count', stats.isolateCount);
     metrics.gauge('isolates.memory', stats.totalMemory);
     metrics.gauge('isolates.avg_sti', stats.averageSTI);
   }, 10000);
   ```

4. **Handle Errors Gracefully**
   ```javascript
   process.on('uncaughtException', (error) => {
     console.error('Fatal error:', error);
     engine.destroy();
     process.exit(1);
   });
   ```

## Next Steps

- Read the [Architecture Overview](ARCHITECTURE.md)
- Check the [API Reference](API.md)
- Explore the [examples](../examples/)
- Join the community discussions

---

**Need Help?**
- File issues on GitHub
- Ask questions in discussions
- Check the documentation at [doc/opencog/](../../doc/opencog/)
