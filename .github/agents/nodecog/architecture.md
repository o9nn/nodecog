# NodeCog System Architecture

This document provides detailed architectural information for the NodeCog system.

## System Overview

NodeCog implements a layered cognitive architecture that integrates OpenCog principles with the Node.js runtime:

```
┌──────────────────────────────────────────────────────────┐
│  Application Layer                                       │
│  - User Applications                                     │
│  - Custom Cognitive Agents                               │
│  - Knowledge Models                                      │
└────────────────┬─────────────────────────────────────────┘
                 │
┌────────────────▼─────────────────────────────────────────┐
│  JavaScript API Layer                                    │
│  - AtomSpace (Hypergraph Knowledge)                      │
│  - NodeSpace (Module Tracking)                           │
│  - AttentionBank (ECAN Allocation)                       │
│  - AgentOrchestrator (Multi-Agent Coordination)          │
│  - CognitiveLoop (Autonomous Cycles)                     │
└────────────────┬─────────────────────────────────────────┘
                 │ N-API Bridge (Future)
┌────────────────▼─────────────────────────────────────────┐
│  Echo.Kern (C++ Kernel Layer) - PLANNED                 │
│  - Hypergraph Filesystem (hgfs_*)                        │
│  - DTESN Cognitive Scheduler (dtesn_sched_*)             │
│  - PLN Inference Engine (pln_*)                          │
│  - ESN Reservoir Computing (esn_*)                       │
│  - Distributed Sync (dist_*)                             │
└────────────────┬─────────────────────────────────────────┘
                 │ GGML Tensor API
┌────────────────▼─────────────────────────────────────────┐
│  GGML Tensor Backend - PLANNED                           │
│  - Tensor Operations                                     │
│  - Memory Pool Management                                │
│  - Hypergraph as Tensor Structure                        │
└────────────────┬─────────────────────────────────────────┘
                 │ Hardware Abstraction
┌────────────────▼─────────────────────────────────────────┐
│  Hardware Acceleration - PLANNED                         │
│  - CPU: AVX2, AVX512, NEON                               │
│  - GPU: CUDA (NVIDIA), Metal (Apple)                     │
│  - BLAS: OpenBLAS, MKL, Accelerate                       │
└──────────────────────────────────────────────────────────┘
```

## Core Components

### 1. AtomSpace - Hypergraph Knowledge Representation

**Location**: `lib/internal/opencog/atomspace.js` (265 lines)

**Purpose**: Central knowledge store using typed hypergraph representation

**Key Features**:
- **Nodes**: Atomic concepts with names and types
- **Links**: Relationships between atoms (arbitrary arity)
- **Truth Values**: Probabilistic strength and confidence
- **Attention Values**: STI (short-term), LTI (long-term), VLTI (very long-term)
- **Pattern Matching**: Query knowledge by structure
- **Automatic Forgetting**: Memory management via attention decay
- **Event System**: Subscribe to atom additions/removals

**API**:
```javascript
// Create AtomSpace
const atomspace = new AtomSpace({ maxSize: 10000, forgettingEnabled: true });

// Add nodes
const concept = atomspace.addNode('CONCEPT', 'JavaScript');
const predicate = atomspace.addNode('PREDICATE', 'is_fast');

// Add links
const evaluation = atomspace.addLink('EVALUATION', [predicate, concept]);

// Set truth values
atomspace.setTruthValue(concept, { strength: 0.9, confidence: 0.85 });

// Query
const allConcepts = atomspace.getAtomsByType('CONCEPT');
const jsAtoms = atomspace.getAtomsByName('JavaScript');
```

**Data Structure**:
- Internal Map: `atomId` → `Atom` object
- Index: Multi-level indexing by type, name, outgoing atoms
- Event Emitter: Node.js EventEmitter for change notifications

**Memory Management**:
- Max size enforcement (FIFO eviction when full)
- Attention-based forgetting (remove low-attention atoms)
- Configurable forgetting thresholds

### 2. NodeSpace - Cognitive Module Tracking

**Location**: `lib/internal/opencog/nodespace.js` (464 lines)

**Purpose**: Automatic tracking of Node.js modules in AtomSpace

**Key Features**:
- **Zero-Config Tracking**: Hooks into CJS/ESM loaders
- **Module Types**: BUILTIN, NPM, LOCAL, JSON
- **Dependency Graph**: Real-time hypergraph of module relationships
- **Export Tracking**: Tracks module exports as atoms
- **Circular Detection**: Identifies circular dependencies
- **Attention Integration**: Modules get STI/LTI based on usage
- **Analysis API**: Rich querying and statistics

**Atom Types**:
- `BUILTIN_MODULE`: Node.js core modules (fs, http, etc.)
- `NPM_MODULE`: External npm packages
- `LOCAL_MODULE`: Project files
- `JSON_MODULE`: JSON config files

**Link Types**:
- `DEPENDS_ON`: Module A depends on Module B
- `EXPORTS`: Module exports symbol

**Integration Points**:
- `lib/internal/modules/cjs/loader.js`: Module._load hook
- `lib/internal/modules/esm/module_job.js`: ESM linking hook
- `process.opencog.nodespace`: Global runtime access

**API**:
```javascript
// Enabled via environment variable
// NODE_OPENCOG_ENABLE=1 node app.js

// Runtime access
const nodespace = process.opencog.nodespace;

// Get dependencies
const deps = nodespace.getModuleDependencies('./mymodule.js');
console.log('Direct dependencies:', deps.length);

// Analyze dependency chains
const chain = nodespace.getDependencyChain('./app.js');
console.log('Full dependency tree depth:', chain.length);

// Detect circular dependencies
const cycles = nodespace.detectCircularDependencies();
cycles.forEach(cycle => {
  console.log('Circular:', cycle.join(' -> '));
});

// Get statistics
const stats = nodespace.getModuleGraph();
console.log('Total modules:', stats.nodes.length);
console.log('Total dependencies:', stats.edges.length);
```

### 3. Attention Bank - Economic Attention Allocation

**Location**: `lib/internal/opencog/attention.js` (183 lines)

**Purpose**: Implements ECAN (Economic Attention Networks) for resource allocation

**Key Features**:
- **STI**: Short-Term Importance (working memory)
- **LTI**: Long-Term Importance (long-term knowledge)
- **VLTI**: Very Long-Term Importance (core knowledge)
- **Attention Spreading**: Propagate attention through links
- **Decay**: Natural forgetting over time
- **Normalization**: Keep total attention constant (economy)
- **Stimulus**: Boost attention for specific atoms
- **Rent**: Attention cost for staying in working memory

**Attention Economy**:
- Total attention is conserved (fixed budget)
- High-attention atoms stay in focus
- Low-attention atoms get forgotten
- Spreading shares attention across related atoms

**API**:
```javascript
// Create attention bank
const attentionBank = new AttentionBank(atomspace, {
  totalSTI: 10000,
  totalLTI: 10000,
  decayRate: 0.1,
  spreadingFactor: 0.5
});

// Stimulate atom (increase STI)
attentionBank.stimulate(atomId, 50);

// Spread attention through hypergraph
attentionBank.spreadAttention(sourceAtomId);

// Decay attention over time
attentionBank.decay();

// Normalize to maintain economy
attentionBank.normalize();

// Get high-attention atoms
const focused = attentionBank.getTopSTIAtoms(10);
console.log('Top 10 focused atoms:', focused);
```

**Attention Values Structure**:
```javascript
{
  sti: 100,      // Short-term importance (working memory)
  lti: 500,      // Long-term importance (learned significance)
  vlti: false    // Very long-term (protected from forgetting)
}
```

### 4. Agent System - Autonomous Cognitive Agents

**Location**: `lib/internal/opencog/agent.js` (217 lines)

**Purpose**: Framework for creating autonomous cognitive agents

**Base Agent Class**:
```javascript
class Agent {
  constructor(name, frequency = 1) {
    this.name = name;
    this.frequency = frequency; // How often to execute (1 = every cycle)
    this.cyclesSinceLastRun = 0;
    this.executionCount = 0;
    this.totalExecutionTime = 0;
  }

  // Override in subclasses
  async execute(atomspace, attentionBank) {
    // Agent logic here
  }

  shouldRun(currentCycle) {
    return currentCycle % this.frequency === 0;
  }
}
```

**Built-In Agents**:

1. **InferenceAgent**: Forward-chaining logical reasoning
   - Applies inference rules to AtomSpace
   - Generates new knowledge from existing facts
   - Uses attention to focus on important atoms
   
2. **AttentionAllocationAgent**: Dynamic attention spreading
   - Spreads attention through hypergraph
   - Applies decay and normalization
   - Maintains attention economy
   
3. **PatternMinerAgent**: Automated pattern discovery
   - Finds frequent subgraphs
   - Mines implicit patterns
   - Creates pattern atoms
   
4. **ModuleAnalyzerAgent**: Code analysis and optimization
   - Detects dead code (unused modules)
   - Identifies circular dependencies
   - Finds dependency hotspots
   - Recommends optimizations

**Custom Agent Example**:
```javascript
class SecurityScannerAgent extends Agent {
  constructor() {
    super('SecurityScanner', 10); // Run every 10 cycles
  }

  async execute(atomspace, attentionBank) {
    // Find all NPM modules
    const npmModules = atomspace.getAtomsByType('NPM_MODULE');
    
    for (const module of npmModules) {
      // Check for known vulnerabilities
      const hasVuln = await this.checkCVE(module.name);
      
      if (hasVuln) {
        // Boost attention to alert
        attentionBank.stimulate(module.id, 1000);
        
        // Add vulnerability link
        const vulnNode = atomspace.addNode('VULNERABILITY', hasVuln.cveId);
        atomspace.addLink('HAS_VULNERABILITY', [module.id, vulnNode.id]);
      }
    }
  }
}
```

### 5. Agent Orchestrator - Multi-Agent Coordination

**Location**: `lib/internal/opencog/orchestrator.js` (145 lines)

**Purpose**: Coordinates execution of multiple cognitive agents

**Key Features**:
- **Priority-Based Scheduling**: High-priority agents run first
- **Concurrent Execution**: Batch processing of independent agents
- **Dynamic Agent Management**: Add/remove agents at runtime
- **Performance Monitoring**: Track execution time and statistics
- **Cycle Management**: Orchestrate agent cycles
- **Error Handling**: Isolate agent failures

**API**:
```javascript
// Create orchestrator
const orchestrator = new AgentOrchestrator(atomspace, attentionBank, {
  concurrency: 5  // Run up to 5 agents in parallel
});

// Register agents
orchestrator.registerAgent(new InferenceAgent());
orchestrator.registerAgent(new AttentionAllocationAgent());
orchestrator.registerAgent(new PatternMinerAgent());
orchestrator.registerAgent(new MyCustomAgent());

// Run one cycle (all agents)
const results = await orchestrator.runCycle();
console.log('Agents executed:', results.length);

// Get statistics
const stats = orchestrator.getStatistics();
console.log('Total executions:', stats.totalExecutions);
console.log('Average cycle time:', stats.avgCycleTime, 'ms');

// Dynamic management
orchestrator.unregisterAgent('PatternMiner');
orchestrator.registerAgent(new AnotherAgent(), 10); // Priority 10
```

**Execution Order**:
1. Group agents by priority
2. Within priority group, execute concurrently (up to concurrency limit)
3. Check each agent's frequency
4. Execute eligible agents
5. Track performance metrics

### 6. Cognitive Loop - Autonomous Operation

**Location**: `lib/internal/opencog/cognitive_loop.js` (152 lines)

**Purpose**: Continuous autonomous cognitive cycles

**Key Features**:
- **Autonomous Cycles**: Runs indefinitely once started
- **Configurable Interval**: Adjust cycle frequency (default 1000ms)
- **Auto Decay**: Automatic attention decay each cycle
- **Auto Normalize**: Maintain attention economy
- **Pause/Resume**: Control execution
- **Event System**: Subscribe to cycle events
- **Error Recovery**: Continue on agent failures

**API**:
```javascript
// Create cognitive loop
const cognitiveLoop = new CognitiveLoop(orchestrator, attentionBank, {
  interval: 1000,        // 1 second cycles
  autoDecay: true,       // Decay attention each cycle
  autoNormalize: true,   // Normalize attention each cycle
  decayRate: 0.1         // 10% decay per cycle
});

// Listen to events
cognitiveLoop.on('cycle:start', (cycleNum) => {
  console.log('Starting cycle', cycleNum);
});

cognitiveLoop.on('cycle:end', (cycleNum, stats) => {
  console.log('Cycle', cycleNum, 'complete in', stats.duration, 'ms');
});

// Start autonomous operation
cognitiveLoop.start();

// Later: pause
cognitiveLoop.pause();

// Resume
cognitiveLoop.resume();

// Stop completely
cognitiveLoop.stop();
```

**Cycle Steps**:
1. Emit 'cycle:start' event
2. Run agent orchestrator cycle
3. Apply attention decay (if enabled)
4. Normalize attention (if enabled)
5. Emit 'cycle:end' event with statistics
6. Wait for interval
7. Repeat

## Integration Points

### Module Loader Hooks

**CommonJS Integration** (`lib/internal/modules/cjs/loader.js`):
```javascript
// Original Module._load
const originalLoad = Module._load;

// Wrapped version with NodeSpace tracking
Module._load = function(request, parent, isMain) {
  const result = originalLoad.apply(this, arguments);
  
  // Track in NodeSpace if enabled
  if (process.opencog?.nodespace) {
    const modulePath = Module._resolveFilename(request, parent);
    const moduleType = detectModuleType(modulePath);
    
    // Add to AtomSpace
    const moduleAtom = process.opencog.nodespace.trackModule(
      modulePath, 
      moduleType,
      parent?.filename
    );
    
    // Track dependency
    if (parent) {
      process.opencog.nodespace.trackDependency(
        parent.filename,
        modulePath
      );
    }
  }
  
  return result;
};
```

**ESM Integration** (`lib/internal/modules/esm/module_job.js`):
```javascript
// In module linking phase
async link() {
  const result = await originalLink.call(this);
  
  // Track ESM module
  if (process.opencog?.nodespace) {
    const moduleAtom = process.opencog.nodespace.trackModule(
      this.url,
      'NPM_MODULE' // or detect type
    );
    
    // Track imports
    for (const importUrl of this.moduleImports) {
      process.opencog.nodespace.trackDependency(this.url, importUrl);
    }
  }
  
  return result;
}
```

### Runtime API (`process.opencog`)

Initialized during Node.js startup when `NODE_OPENCOG_ENABLE=1`:

```javascript
// Global process.opencog object
process.opencog = {
  nodespace: nodeSpaceInstance,
  atomspace: atomSpaceInstance,
  orchestrator: orchestratorInstance, // If AUTO_ANALYZE enabled
  
  // Convenience methods
  getModuleDependencies: (path) => nodespace.getModuleDependencies(path),
  analyzeModules: () => nodespace.getModuleGraph(),
  detectCircularDependencies: () => nodespace.detectCircularDependencies()
};
```

## Data Flow

### Module Loading Flow

```
1. require('./mymodule') or import './mymodule'
   ↓
2. CJS/ESM Loader Hook Triggered
   ↓
3. NodeSpace.trackModule(path, type)
   ↓
4. AtomSpace.addNode('MODULE_TYPE', path)
   ↓
5. If parent: NodeSpace.trackDependency(parent, child)
   ↓
6. AtomSpace.addLink('DEPENDS_ON', [parent, child])
   ↓
7. AttentionBank.stimulate(moduleAtom) // Boost STI
   ↓
8. Module loaded and returned to application
```

### Cognitive Cycle Flow

```
1. CognitiveLoop.start()
   ↓
2. setInterval(() => {
   ↓
3.   orchestrator.runCycle()
   ↓
4.     For each agent (priority order):
   ↓
5.       agent.execute(atomspace, attentionBank)
   ↓
6.         Agent analyzes AtomSpace
   ↓
7.         Agent modifies knowledge/attention
   ↓
8.   attentionBank.decay()
   ↓
9.   attentionBank.normalize()
   ↓
10. }, interval)
```

### Attention Spreading Flow

```
1. attentionBank.spreadAttention(sourceAtomId)
   ↓
2. Get source atom's STI
   ↓
3. Find all outgoing links from source
   ↓
4. For each linked atom:
   ↓
5.   Calculate transfer amount (STI * spreadingFactor)
   ↓
6.   Transfer attention: source → target
   ↓
7.   Update both atoms' STI values
   ↓
8. Recursively spread to neighbors (with decay)
```

## Performance Characteristics

### Current JavaScript Implementation

| Operation | Typical Time | Notes |
|-----------|-------------|-------|
| Add Node | ~1-2 µs | Map insertion + indexing |
| Add Link | ~2-3 µs | Multiple index updates |
| Get Atom | ~0.5 µs | Map lookup |
| Pattern Match (10 nodes) | ~50 ms | Recursive search |
| Attention Spread (1000 atoms) | ~100 ms | Graph traversal |
| Cognitive Cycle | ~200-500 ms | Depends on agents |
| Module Track | < 100 µs | Minimal overhead |

### Future Kernel Implementation (Planned)

| Operation | Target Time | Improvement |
|-----------|------------|-------------|
| Add Node | < 5 µs | Tensor allocation |
| Add Link | < 10 µs | Tensor operation |
| Pattern Match (10 nodes) | < 100 µs | **500x faster** |
| Attention Spread (1000 atoms) | < 100 µs | **1000x faster** |
| Cognitive Cycle | < 10 ms | **50x faster** |
| Module Track | < 10 µs | Kernel syscall |

## Configuration

### Environment Variables

```bash
# Enable NodeSpace module tracking
NODE_OPENCOG_ENABLE=1

# Enable autonomous module analysis agents
NODE_OPENCOG_AUTO_ANALYZE=1

# Set cognitive loop interval (ms)
NODE_OPENCOG_CYCLE_INTERVAL=1000

# Enable debug logging
NODE_OPENCOG_DEBUG=1

# Future: Enable kernel backend
NODE_OPENCOG_KERNEL_BACKEND=1
```

### Programmatic Configuration

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
  
  // Cognitive loop config
  loopInterval: 1000,
  autoDecay: true,
  autoNormalize: true
});
```

## Extension Points

### Custom Agents

Extend the `Agent` base class:

```javascript
const { Agent } = require('lib/opencog');

class MyAnalysisAgent extends Agent {
  constructor() {
    super('MyAnalyzer', 5); // Run every 5 cycles
  }
  
  async execute(atomspace, attentionBank) {
    // Your cognitive logic here
    const concepts = atomspace.getAtomsByType('CONCEPT');
    
    // Analyze and create new knowledge
    for (const concept of concepts) {
      // ... analysis logic
    }
  }
}

// Register with orchestrator
system.orchestrator.registerAgent(new MyAnalysisAgent());
```

### Custom Atom Types

Define domain-specific atom types:

```javascript
// Add custom node type
const userNode = atomspace.addNode('USER', 'john@example.com');
const sessionNode = atomspace.addNode('SESSION', 'sess_abc123');

// Add custom link type
const hasSessionLink = atomspace.addLink('HAS_SESSION', [userNode.id, sessionNode.id]);
```

### Event Listeners

Subscribe to system events:

```javascript
// AtomSpace events
atomspace.on('atom:added', (atom) => {
  console.log('New atom:', atom.name);
});

atomspace.on('atom:removed', (atomId) => {
  console.log('Atom removed:', atomId);
});

// Cognitive loop events
cognitiveLoop.on('cycle:start', (num) => {
  console.log('Cycle', num, 'starting');
});

cognitiveLoop.on('cycle:end', (num, stats) => {
  console.log('Cycle', num, 'took', stats.duration, 'ms');
});
```

## References

For more details, see:
- `DEVELOPMENT_ROADMAP.md` - Complete implementation plan
- `OPENCOG_SUMMARY.md` - Implementation summary
- `doc/TECHNICAL_ARCHITECTURE.md` - Architecture diagrams
- `doc/FORMAL_SPECIFICATION.md` - Z++ formal specifications
