# NodeCog API Reference

Complete API documentation for NodeCog's cognitive computing platform.

## Table of Contents

1. [AtomSpace API](#atomspace-api)
2. [NodeSpace API](#nodespace-api)
3. [Attention Bank API](#attention-bank-api)
4. [Agent API](#agent-api)
5. [Agent Orchestrator API](#agent-orchestrator-api)
6. [Cognitive Loop API](#cognitive-loop-api)
7. [Distributed AtomSpace API](#distributed-atomspace-api)
8. [Runtime API (process.opencog)](#runtime-api)
9. [Configuration Options](#configuration-options)
10. [Event System](#event-system)

---

## AtomSpace API

### Constructor

```javascript
const atomspace = new AtomSpace(options);
```

**Options**:
```javascript
{
  maxSize: 10000,           // Maximum atoms (0 = unlimited)
  forgettingEnabled: true,  // Enable automatic forgetting
  forgettingThreshold: 0.1, // STI threshold for forgetting
  enableEvents: true        // Enable event emission
}
```

### Methods

#### addNode(type, name, truthValue?)
Create a new node atom.

```javascript
const concept = atomspace.addNode('CONCEPT', 'JavaScript', {
  strength: 0.9,
  confidence: 0.85
});
```

**Returns**: `Atom` object with `{ id, type, name, truthValue, attentionValue }`

#### addLink(type, outgoing, truthValue?)
Create a new link atom.

```javascript
const link = atomspace.addLink('INHERITANCE', [
  concept1.id,
  concept2.id
], {
  strength: 0.8,
  confidence: 0.9
});
```

**Returns**: `Atom` object

#### getAtom(atomId)
Get atom by ID.

```javascript
const atom = atomspace.getAtom(atomId);
```

**Returns**: `Atom` or `null`

#### getAtomsByType(type)
Get all atoms of specific type.

```javascript
const concepts = atomspace.getAtomsByType('CONCEPT');
const links = atomspace.getAtomsByType('INHERITANCE');
```

**Returns**: `Atom[]`

#### getAtomsByName(name)
Get all atoms with specific name.

```javascript
const jsAtoms = atomspace.getAtomsByName('JavaScript');
```

**Returns**: `Atom[]`

#### getIncomingLinks(atomId, linkType?)
Get atoms that link to this atom.

```javascript
// All incoming links
const incoming = atomspace.getIncomingLinks(atomId);

// Only specific type
const inheritances = atomspace.getIncomingLinks(atomId, 'INHERITANCE');
```

**Returns**: `Atom[]`

#### removeAtom(atomId)
Remove atom from AtomSpace.

```javascript
atomspace.removeAtom(atomId);
```

**Returns**: `boolean` (success)

#### setTruthValue(atomId, truthValue)
Set truth value.

```javascript
atomspace.setTruthValue(atomId, {
  strength: 0.95,      // 0.0 to 1.0
  confidence: 0.90     // 0.0 to 1.0
});
```

#### getTruthValue(atomId)
Get truth value.

```javascript
const tv = atomspace.getTruthValue(atomId);
// => { strength: 0.95, confidence: 0.90 }
```

#### setAttentionValue(atomId, attentionValue)
Set attention value.

```javascript
atomspace.setAttentionValue(atomId, {
  sti: 100,           // Short-term importance
  lti: 50,            // Long-term importance
  vlti: false         // Very long-term importance
});
```

#### getAttentionValue(atomId)
Get attention value.

```javascript
const av = atomspace.getAttentionValue(atomId);
// => { sti: 100, lti: 50, vlti: false }
```

#### query(criteria)
Advanced query.

```javascript
const results = atomspace.query({
  type: 'CONCEPT',
  namePattern: /JavaScript/i,
  minConfidence: 0.7,
  minSTI: 10
});
```

#### patternMatch(pattern)
Structural pattern matching.

```javascript
const matches = atomspace.patternMatch({
  type: 'INHERITANCE',
  outgoing: [
    { type: 'CONCEPT' },
    { type: 'CONCEPT', name: 'ProgrammingLanguage' }
  ]
});
```

#### clear()
Remove all atoms.

```javascript
atomspace.clear();
```

#### getSize()
Get atom count.

```javascript
const count = atomspace.getSize();
```

### Events

```javascript
atomspace.on('atom:added', (atom) => {
  console.log('Added:', atom.name);
});

atomspace.on('atom:removed', (atomId) => {
  console.log('Removed:', atomId);
});

atomspace.on('atom:modified', (atom) => {
  console.log('Modified:', atom.name);
});
```

---

## NodeSpace API

### Constructor

Automatically created when `NODE_OPENCOG_ENABLE=1`.

Access via `process.opencog.nodespace`.

### Methods

#### trackModule(path, type, parentPath?)
Manually track a module.

```javascript
const moduleAtom = nodespace.trackModule(
  '/path/to/module.js',
  'LOCAL_MODULE',
  '/path/to/parent.js'
);
```

#### trackDependency(fromPath, toPath)
Manually track dependency.

```javascript
nodespace.trackDependency(
  '/app.js',
  './utils.js'
);
```

#### getModuleAtom(path)
Get atom for module.

```javascript
const atom = nodespace.getModuleAtom('./mymodule.js');
```

**Returns**: `Atom` or `null`

#### getModuleDependencies(path)
Get direct dependencies.

```javascript
const deps = nodespace.getModuleDependencies('./app.js');
// => [Atom, Atom, ...]
```

#### getDependencyChain(path, maxDepth?)
Get full dependency tree.

```javascript
const chain = nodespace.getDependencyChain('./app.js', 10);
// => [[Atom], [Atom, Atom], [Atom, Atom, Atom], ...]
```

#### detectCircularDependencies()
Find circular dependencies.

```javascript
const cycles = nodespace.detectCircularDependencies();
// => [
//   { cycle: ['/a.js', '/b.js', '/c.js', '/a.js'], length: 3 },
//   ...
// ]
```

#### getModuleGraph()
Get full module graph.

```javascript
const graph = nodespace.getModuleGraph();
// => {
//   nodes: [{ id, name, type }, ...],
//   edges: [{ source, target, type }, ...]
// }
```

#### getModulesByType(type)
Get modules of specific type.

```javascript
const npmModules = nodespace.getModulesByType('NPM_MODULE');
const builtins = nodespace.getModulesByType('BUILTIN_MODULE');
```

---

## Attention Bank API

### Constructor

```javascript
const attentionBank = new AttentionBank(atomspace, options);
```

**Options**:
```javascript
{
  totalSTI: 10000,          // Total short-term importance budget
  totalLTI: 10000,          // Total long-term importance budget
  decayRate: 0.1,           // Decay rate per cycle (0.0-1.0)
  spreadingFactor: 0.5,     // Attention transfer factor (0.0-1.0)
  minSTI: 0,                // Minimum STI value
  maxSTI: 1000,             // Maximum STI value
  rentRate: 1               // STI cost per cycle
}
```

### Methods

#### stimulate(atomId, amount)
Increase STI.

```javascript
attentionBank.stimulate(atomId, 50);
```

#### getSTI(atomId)
Get short-term importance.

```javascript
const sti = attentionBank.getSTI(atomId);
```

#### getLTI(atomId)
Get long-term importance.

```javascript
const lti = attentionBank.getLTI(atomId);
```

#### setSTI(atomId, value)
Set STI directly.

```javascript
attentionBank.setSTI(atomId, 100);
```

#### setLTI(atomId, value)
Set LTI directly.

```javascript
attentionBank.setLTI(atomId, 50);
```

#### spreadAttention(sourceAtomId, maxDepth?)
Spread attention through hypergraph.

```javascript
attentionBank.spreadAttention(atomId, 3);
```

#### decay(rate?)
Apply attention decay.

```javascript
attentionBank.decay(0.1); // 10% decay
```

#### normalize()
Normalize attention values to maintain economy.

```javascript
attentionBank.normalize();
```

#### getTopSTIAtoms(n)
Get highest-attention atoms.

```javascript
const top10 = attentionBank.getTopSTIAtoms(10);
```

#### getTotalSTI()
Get total STI in system.

```javascript
const total = attentionBank.getTotalSTI();
```

---

## Agent API

### Base Class

```javascript
class MyAgent extends Agent {
  constructor() {
    super('MyAgent', 5); // Name, frequency (every N cycles)
  }
  
  async execute(atomspace, attentionBank) {
    // Agent logic here
  }
}
```

### Built-In Agents

#### InferenceAgent

```javascript
const inferenceAgent = new InferenceAgent({
  frequency: 1,
  minConfidence: 0.5,
  maxInferences: 100
});
```

#### AttentionAllocationAgent

```javascript
const attentionAgent = new AttentionAllocationAgent({
  frequency: 1,
  spreadingFactor: 0.5,
  decayRate: 0.1,
  rentRate: 1
});
```

#### PatternMinerAgent

```javascript
const patternMiner = new PatternMinerAgent({
  frequency: 10,
  minSupport: 3,
  maxPatternSize: 4
});
```

#### ModuleAnalyzerAgent

```javascript
const moduleAnalyzer = new ModuleAnalyzerAgent({
  frequency: 5,
  deadCodeThreshold: 0,
  hotspotThreshold: 10,
  circularDetection: true
});
```

### Agent Properties

```javascript
agent.name              // Agent name
agent.frequency         // Execution frequency
agent.executionCount    // Times executed
agent.totalExecutionTime // Total ms
agent.avgExecutionTime  // Average ms
```

### Methods

#### shouldRun(cycleNumber)
Check if agent should run.

```javascript
const should = agent.shouldRun(10);
```

#### getStatistics()
Get execution statistics.

```javascript
const stats = agent.getStatistics();
// => {
//   executionCount: 42,
//   totalExecutionTime: 1234,
//   avgExecutionTime: 29.4
// }
```

---

## Agent Orchestrator API

### Constructor

```javascript
const orchestrator = new AgentOrchestrator(atomspace, attentionBank, options);
```

**Options**:
```javascript
{
  concurrency: 5,     // Max parallel agents
  timeout: 30000      // Agent timeout (ms)
}
```

### Methods

#### registerAgent(agent, priority?)
Register agent.

```javascript
orchestrator.registerAgent(new MyAgent(), 10); // Priority 10
```

#### unregisterAgent(name)
Unregister agent.

```javascript
orchestrator.unregisterAgent('MyAgent');
```

#### runCycle(cycleNumber?)
Execute one cycle.

```javascript
const results = await orchestrator.runCycle(42);
// => [{ agent, result, duration }, ...]
```

#### getStatistics()
Get orchestrator statistics.

```javascript
const stats = orchestrator.getStatistics();
// => {
//   totalCycles: 100,
//   totalExecutions: 500,
//   totalTime: 12345,
//   avgCycleTime: 123.45,
//   agents: [...]
// }
```

#### getAgents()
Get registered agents.

```javascript
const agents = orchestrator.getAgents();
```

---

## Cognitive Loop API

### Constructor

```javascript
const cognitiveLoop = new CognitiveLoop(orchestrator, attentionBank, options);
```

**Options**:
```javascript
{
  interval: 1000,        // Cycle interval (ms)
  autoDecay: true,       // Auto decay each cycle
  autoNormalize: true,   // Auto normalize each cycle
  decayRate: 0.1         // Decay rate if autoDecay
}
```

### Methods

#### start()
Start autonomous operation.

```javascript
cognitiveLoop.start();
```

#### pause()
Pause execution.

```javascript
cognitiveLoop.pause();
```

#### resume()
Resume from pause.

```javascript
cognitiveLoop.resume();
```

#### stop()
Stop completely.

```javascript
cognitiveLoop.stop();
```

#### isRunning()
Check if running.

```javascript
const running = cognitiveLoop.isRunning();
```

#### isPaused()
Check if paused.

```javascript
const paused = cognitiveLoop.isPaused();
```

#### getCurrentCycle()
Get cycle number.

```javascript
const cycle = cognitiveLoop.getCurrentCycle();
```

### Events

```javascript
cognitiveLoop.on('cycle:start', (cycleNum) => {
  console.log('Starting cycle', cycleNum);
});

cognitiveLoop.on('cycle:end', (cycleNum, stats) => {
  console.log('Cycle', cycleNum, 'took', stats.duration, 'ms');
});

cognitiveLoop.on('error', (error) => {
  console.error('Loop error:', error);
});
```

---

## Distributed AtomSpace API

### Constructor

```javascript
const distributed = new DistributedAtomSpace(atomspace, options);
```

**Options**:
```javascript
{
  nodeId: 'node-1',           // Unique node ID
  peers: ['node-2', 'node-3'], // Peer node IDs
  syncInterval: 1000,         // Sync interval (ms)
  conflictResolution: 'lww'   // 'lww' or 'merge'
}
```

### Methods

#### start()
Start synchronization.

```javascript
distributed.start();
```

#### stop()
Stop synchronization.

```javascript
distributed.stop();
```

#### syncAtom(atomId)
Manually sync specific atom.

```javascript
distributed.syncAtom(atomId);
```

#### syncAll()
Sync all atoms.

```javascript
distributed.syncAll();
```

---

## Runtime API (process.opencog)

When `NODE_OPENCOG_ENABLE=1`, access via `process.opencog`:

### Properties

```javascript
process.opencog.nodespace      // NodeSpace instance
process.opencog.atomspace      // AtomSpace instance
process.opencog.orchestrator   // AgentOrchestrator (if AUTO_ANALYZE)
```

### Methods

```javascript
// Get module dependencies
const deps = process.opencog.getModuleDependencies('./mymodule.js');

// Get module graph statistics
const stats = process.opencog.analyzeModules();
// => {
//   totalModules: 42,
//   byType: { BUILTIN_MODULE: 10, NPM_MODULE: 20, LOCAL_MODULE: 12 },
//   totalDependencies: 128
// }

// Detect circular dependencies
const cycles = process.opencog.detectCircularDependencies();
// => [{ cycle: [...], length: 3 }, ...]
```

---

## Configuration Options

### Environment Variables

```bash
# Enable NodeSpace module tracking
NODE_OPENCOG_ENABLE=1

# Enable autonomous module analysis
NODE_OPENCOG_AUTO_ANALYZE=1

# Set cognitive loop interval (ms)
NODE_OPENCOG_CYCLE_INTERVAL=1000

# Enable debug logging
NODE_OPENCOG_DEBUG=1

# Future: Enable kernel backend
NODE_OPENCOG_KERNEL_BACKEND=1
```

### Factory Function

```javascript
const { createCognitiveSystem } = require('lib/opencog');

const system = createCognitiveSystem({
  // AtomSpace config
  atomspaceMaxSize: 10000,
  forgettingEnabled: true,
  forgettingThreshold: 0.1,
  
  // Attention config
  totalSTI: 10000,
  totalLTI: 10000,
  decayRate: 0.1,
  spreadingFactor: 0.5,
  
  // Orchestrator config
  concurrency: 5,
  agentTimeout: 30000,
  
  // Cognitive loop config
  loopInterval: 1000,
  autoDecay: true,
  autoNormalize: true
});

// Access components
system.atomspace
system.attentionBank
system.orchestrator
system.cognitiveLoop
```

---

## Event System

### AtomSpace Events

```javascript
atomspace.on('atom:added', (atom) => { });
atomspace.on('atom:removed', (atomId) => { });
atomspace.on('atom:modified', (atom) => { });
```

### Cognitive Loop Events

```javascript
cognitiveLoop.on('cycle:start', (cycleNum) => { });
cognitiveLoop.on('cycle:end', (cycleNum, stats) => { });
cognitiveLoop.on('error', (error) => { });
cognitiveLoop.on('paused', () => { });
cognitiveLoop.on('resumed', () => { });
cognitiveLoop.on('stopped', () => { });
```

### NodeSpace Events

```javascript
nodespace.on('module:tracked', (moduleAtom) => { });
nodespace.on('dependency:tracked', (fromPath, toPath) => { });
nodespace.on('circular:detected', (cycle) => { });
```

---

## Usage Examples

### Complete System Setup

```javascript
const { createCognitiveSystem } = require('lib/opencog');

// Create system
const system = createCognitiveSystem({
  atomspaceMaxSize: 10000,
  loopInterval: 1000
});

// Add custom agent
class MyAgent extends Agent {
  constructor() {
    super('MyAgent', 5);
  }
  
  async execute(atomspace, attentionBank) {
    // Your logic
  }
}

system.orchestrator.registerAgent(new MyAgent());

// Start cognitive loop
system.cognitiveLoop.start();

// Monitor cycles
system.cognitiveLoop.on('cycle:end', (num, stats) => {
  console.log(`Cycle ${num}: ${stats.duration}ms`);
});
```

### Module Tracking

```javascript
// Enable via environment variable
// NODE_OPENCOG_ENABLE=1 node app.js

// Access module graph
const stats = process.opencog.analyzeModules();
console.log('Total modules:', stats.totalModules);

// Check for circular dependencies
const cycles = process.opencog.detectCircularDependencies();
if (cycles.length > 0) {
  console.warn('Circular dependencies found:', cycles);
}

// Get specific module dependencies
const deps = process.opencog.getModuleDependencies('./mymodule.js');
console.log('Dependencies:', deps.map(d => d.name));
```

---

## Type Definitions

### Atom

```typescript
interface Atom {
  id: string;
  type: string;
  name?: string;
  outgoing?: string[];
  truthValue?: TruthValue;
  attentionValue?: AttentionValue;
}
```

### TruthValue

```typescript
interface TruthValue {
  strength: number;     // 0.0 to 1.0
  confidence: number;   // 0.0 to 1.0
}
```

### AttentionValue

```typescript
interface AttentionValue {
  sti: number;          // Short-term importance
  lti: number;          // Long-term importance
  vlti: boolean;        // Very long-term importance flag
}
```

---

## References

For more details:
- **architecture.md** - System architecture
- **cognitive-features.md** - Cognitive capabilities
- **kernel-layer.md** - Future kernel API
- **development-roadmap.md** - Implementation timeline
- **Main documentation** - doc/opencog/README.md
