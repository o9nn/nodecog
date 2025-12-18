# API Reference

## JavaScript API

### CognitiveSynergyEngine

Main engine class for managing V8 isolates with cognitive control.

#### Constructor

```javascript
const engine = new CognitiveSynergyEngine(config);
```

**Parameters:**
- `config` (Object): Configuration options
  - `cognitiveTick` (number): Cognitive loop tick interval in ms (default: 5)
  - `workerThreads` (number): Number of libuv worker threads (default: 4)
  - `maxMicrotasks` (number): Max microtasks per isolate per slice (default: 100)
  - `attentionBased` (boolean): Enable attention-based scheduling (default: true)
  - `monitoring` (boolean): Enable performance monitoring (default: true)

#### Methods

##### `initialize()`

Initialize the cognitive synergy engine.

```javascript
engine.initialize();
```

**Throws:** Error if already initialized

##### `destroy()`

Destroy the engine and all isolates.

```javascript
engine.destroy();
```

##### `createIsolate(id, options)`

Create a new V8 isolate with cognitive control.

```javascript
const isolate = engine.createIsolate('reasoning', {
  sti: 100,
  lti: 90,
});
```

**Parameters:**
- `id` (string): Unique identifier for the isolate
- `options` (Object): Isolate configuration
  - `sti` (number): Initial Short-Term Importance (default: 50)
  - `lti` (number): Initial Long-Term Importance (default: 50)

**Returns:** `IsolateContext` instance

**Throws:** Error if engine not initialized or isolate already exists

##### `destroyIsolate(id)`

Destroy an isolate.

```javascript
engine.destroyIsolate('reasoning');
```

**Parameters:**
- `id` (string): Isolate identifier

##### `getIsolate(id)`

Get an isolate by ID.

```javascript
const isolate = engine.getIsolate('reasoning');
```

**Parameters:**
- `id` (string): Isolate identifier

**Returns:** `IsolateContext` or `undefined`

##### `getAllIsolates()`

Get all isolates.

```javascript
const isolates = engine.getAllIsolates();
```

**Returns:** Array of `IsolateContext` instances

##### `getStatistics()`

Get engine statistics.

```javascript
const stats = engine.getStatistics();
console.log(stats);
// {
//   isolateCount: 4,
//   totalMemory: 234881024,
//   averageSTI: 62.5,
//   averageLTI: 70.0
// }
```

**Returns:** Object with statistics

##### `createSharedBuffer(size)`

Create a SharedArrayBuffer for inter-isolate communication.

```javascript
const buffer = engine.createSharedBuffer(1024 * 1024);  // 1MB
const data = new Float64Array(buffer);
```

**Parameters:**
- `size` (number): Buffer size in bytes

**Returns:** `SharedArrayBuffer`

---

### IsolateContext

Represents an isolated V8 execution context with cognitive control.

#### Properties

##### `id`

Unique identifier for the isolate (read-only).

```javascript
console.log(isolate.id);  // 'reasoning'
```

#### Methods

##### `getSTI()`

Get Short-Term Importance (execution priority).

```javascript
const sti = isolate.getSTI();
```

**Returns:** number (0-200 typical range)

##### `setSTI(value)`

Set Short-Term Importance.

```javascript
isolate.setSTI(120);
```

**Parameters:**
- `value` (number): New STI value

##### `getLTI()`

Get Long-Term Importance (memory budget).

```javascript
const lti = isolate.getLTI();
```

**Returns:** number (0-100 typical range)

##### `setLTI(value)`

Set Long-Term Importance.

```javascript
isolate.setLTI(85);
```

**Parameters:**
- `value` (number): New LTI value

##### `boost(amount)`

Increase attention (STI).

```javascript
isolate.boost(20);  // STI += 20
```

**Parameters:**
- `amount` (number): Amount to increase (default: 10)

##### `decay(amount)`

Decrease attention (STI).

```javascript
isolate.decay(15);  // STI -= 15 (minimum 0)
```

**Parameters:**
- `amount` (number): Amount to decrease (default: 10)

##### `getMemoryUsage()`

Get current memory usage in bytes.

```javascript
const bytes = isolate.getMemoryUsage();
console.log(`Memory: ${bytes / 1024 / 1024}MB`);
```

**Returns:** number (bytes)

##### `getCPUTime()`

Get accumulated CPU time in milliseconds.

```javascript
const ms = isolate.getCPUTime();
```

**Returns:** number (milliseconds)

---

## C++ API

### CognitivePlatform

Manages V8 platform and Node.js integration.

#### Constructor

```cpp
cognitive::PlatformConfig config;
config.worker_threads = 4;
config.cognitive_tick_ms = 5;
config.verbose = true;

cognitive::CognitivePlatform platform(config);
```

#### Methods

##### `Initialize()`

Initialize the platform.

```cpp
bool success = platform.Initialize();
```

**Returns:** `true` on success, `false` on failure

##### `Shutdown()`

Shutdown the platform.

```cpp
platform.Shutdown();
```

##### `GetV8Platform()`

Get the V8 platform.

```cpp
v8::Platform* v8_platform = platform.GetV8Platform();
```

##### `GetNodePlatform()`

Get the Node.js MultiIsolatePlatform.

```cpp
node::MultiIsolatePlatform* node_platform = platform.GetNodePlatform();
```

##### `GetLoop()`

Get the libuv event loop.

```cpp
uv_loop_t* loop = platform.GetLoop();
```

##### `CreateIsolate()`

Create a new V8 isolate.

```cpp
v8::Isolate* isolate = platform.CreateIsolate();
```

##### `CreateEnvironment(isolate, context, args, exec_args)`

Create a Node.js environment for an isolate.

```cpp
std::vector<std::string> args = {"node", "app.js"};
std::vector<std::string> exec_args = {};

node::Environment* env = platform.CreateEnvironment(
    isolate, context, args, exec_args);
```

##### `InstallCognitiveHooks(on_prepare, on_check, on_tick, on_idle)`

Install cognitive scheduler hooks.

```cpp
platform.InstallCognitiveHooks(
    []() { /* prepare */ },
    []() { /* check */ },
    []() { /* tick */ },
    []() { /* idle */ }
);
```

##### `Run()`

Run the event loop (blocking).

```cpp
int result = platform.Run();
```

##### `Stop()`

Stop the event loop.

```cpp
platform.Stop();
```

---

### CognitiveSynergyEngine

Main cognitive synergy engine.

#### Constructor

```cpp
node::cognitive::CognitiveSynergyConfig config;
config.cognitive_tick_ms = 5;
config.worker_threads = 4;
config.attention_based_scheduling = true;

node::cognitive::CognitiveSynergyEngine engine(config);
```

#### Methods

##### `Initialize()`

Initialize the engine.

```cpp
bool success = engine.Initialize();
```

##### `CreateIsolate(id)`

Create a new isolate.

```cpp
node::cognitive::IsolateContext* isolate = engine.CreateIsolate("reasoning");
```

##### `DestroyIsolate(id)`

Destroy an isolate.

```cpp
engine.DestroyIsolate("reasoning");
```

##### `GetIsolate(id)`

Get an isolate by ID.

```cpp
auto* isolate = engine.GetIsolate("reasoning");
```

##### `Start()`

Start the cognitive loop.

```cpp
engine.Start();
```

##### `Stop()`

Stop the cognitive loop.

```cpp
engine.Stop();
```

##### `Run()`

Run the event loop (blocking).

```cpp
int result = engine.Run();
```

##### `loop()`

Get the libuv loop.

```cpp
uv_loop_t* loop = engine.loop();
```

##### `platform()`

Get the platform.

```cpp
auto* platform = engine.platform();
```

##### `scheduler()`

Get the scheduler.

```cpp
auto* scheduler = engine.scheduler();
```

---

### IsolateContext

Represents an isolated V8 execution context.

#### Constructor

```cpp
node::cognitive::IsolateContext context(isolate, env, "reasoning");
```

#### Methods

##### `SetSTI(sti)`

Set Short-Term Importance.

```cpp
context.SetSTI(100.0);
```

##### `GetSTI()`

Get Short-Term Importance.

```cpp
double sti = context.GetSTI();
```

##### `SetLTI(lti)`

Set Long-Term Importance.

```cpp
context.SetLTI(90.0);
```

##### `GetLTI()`

Get Long-Term Importance.

```cpp
double lti = context.GetLTI();
```

##### `ExecuteTasks(max_microtasks)`

Execute pending tasks.

```cpp
context.ExecuteTasks(100);
```

##### `PerformMicrotaskCheckpoint()`

Perform microtask checkpoint.

```cpp
context.PerformMicrotaskCheckpoint();
```

##### `GetMemoryUsage()`

Get memory usage in bytes.

```cpp
size_t bytes = context.GetMemoryUsage();
```

##### `GetCPUTime()`

Get CPU time in seconds.

```cpp
double seconds = context.GetCPUTime();
```

---

### CognitiveScheduler

Decides which isolate runs when.

#### Methods

##### `SelectNextIsolate()`

Select next isolate based on STI/LTI.

```cpp
auto* next = scheduler.SelectNextIsolate();
```

##### `UpdateAttention()`

Update attention based on resource usage.

```cpp
scheduler.UpdateAttention();
```

##### `DecayAttention()`

Decay attention over time.

```cpp
scheduler.DecayAttention();
```

##### `RegisterIsolate(context)`

Register an isolate.

```cpp
scheduler.RegisterIsolate(context);
```

##### `UnregisterIsolate(id)`

Unregister an isolate.

```cpp
scheduler.UnregisterIsolate("reasoning");
```

##### `GetIsolateCount()`

Get number of registered isolates.

```cpp
size_t count = scheduler.GetIsolateCount();
```

---

## ESM Loader API

### loader.mjs

ESM loader hooks for NodeSpace integration.

#### Usage

```bash
node --experimental-loader ./lib/loader.mjs app.js
```

#### Hooks

##### `initialize(data)`

Initialize the loader.

```javascript
export function initialize(data) {
  // Setup NodeSpace connection
}
```

##### `resolve(specifier, context, nextResolve)`

Resolve module specifier.

```javascript
export async function resolve(specifier, context, nextResolve) {
  const result = await nextResolve(specifier, context);
  // Record import in NodeSpace
  return result;
}
```

##### `load(url, context, nextLoad)`

Load module source.

```javascript
export async function load(url, context, nextLoad) {
  const result = await nextLoad(url, context);
  // Record module in NodeSpace
  return result;
}
```

##### `getModuleGraph()`

Get module graph statistics.

```javascript
import { getModuleGraph } from './lib/loader.mjs';

const graph = getModuleGraph();
console.log(graph);
// {
//   modules: [...],
//   dependencies: [...],
//   statistics: { moduleCount, dependencyCount, totalSTI }
// }
```

---

## Configuration

### PlatformConfig

```cpp
struct PlatformConfig {
  int worker_threads = 4;
  uint64_t cognitive_tick_ms = 5;
  bool verbose = false;
};
```

### CognitiveSynergyConfig

```cpp
struct CognitiveSynergyConfig {
  uint64_t cognitive_tick_ms = 5;
  int worker_threads = 4;
  int max_microtasks_per_slice = 100;
  bool attention_based_scheduling = true;
  bool enable_monitoring = true;
};
```

---

## Error Handling

### JavaScript

```javascript
try {
  const engine = createEngine(config);
  engine.initialize();
} catch (error) {
  console.error('Engine initialization failed:', error);
}
```

### C++

```cpp
if (!engine.Initialize()) {
  std::cerr << "Failed to initialize engine\n";
  return 1;
}
```

---

## Examples

See the `examples/` directory for complete working examples:
- `basic.js` - Basic usage
- `multi-isolate.js` - Multiple isolates
- `attention.js` - Attention-based scheduling
- `nodespace.js` - NodeSpace integration

---

**See Also:**
- [Architecture Overview](ARCHITECTURE.md)
- [Integration Guide](INTEGRATION.md)
