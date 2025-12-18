# NodeCog Cognitive Features

This document details the advanced cognitive AI capabilities implemented in NodeCog.

## Overview

NodeCog implements several key cognitive features inspired by OpenCog and AGI research:

1. **Hypergraph Knowledge Representation** - Flexible, typed knowledge graphs
2. **Economic Attention Allocation** - ECAN-inspired resource management
3. **Autonomous Agent System** - Self-managing cognitive agents
4. **Pattern Mining** - Automated discovery of implicit patterns
5. **Inference** - Logical reasoning and knowledge generation
6. **Distributed Cognition** - Multi-node knowledge synchronization
7. **Meta-Optimization** - MOSES evolutionary search
8. **Planning** - Goal-directed behavior
9. **NLP Integration** - Natural language understanding

## 1. Hypergraph Knowledge Representation

### Concept

Knowledge is represented as a **typed hypergraph** where:
- **Nodes** represent atomic concepts (entities, predicates, values)
- **Links** represent relationships (can connect any number of nodes)
- **Types** provide semantic categories (CONCEPT, PREDICATE, EVALUATION, etc.)

### Example

```javascript
// Represent: "JavaScript is a programming language"

// Create atoms
const js = atomspace.addNode('CONCEPT', 'JavaScript');
const progLang = atomspace.addNode('CONCEPT', 'ProgrammingLanguage');
const isA = atomspace.addNode('PREDICATE', 'is_a');

// Create relationship
const statement = atomspace.addLink('INHERITANCE', [js, progLang]);

// Or with evaluation
const evaluation = atomspace.addLink('EVALUATION', [
  isA,
  atomspace.addLink('LIST', [js, progLang])
]);
```

### Truth Values

Every atom has a **probabilistic truth value**:

```javascript
atomspace.setTruthValue(statement, {
  strength: 0.95,      // How true (0.0 to 1.0)
  confidence: 0.90     // How certain (0.0 to 1.0)
});
```

This enables:
- **Uncertain reasoning**: Handle incomplete knowledge
- **Probabilistic inference**: Propagate uncertainty
- **Confidence tracking**: Know what we don't know

### Pattern Matching

Query the hypergraph by structure:

```javascript
// Find all inheritance relationships
const inheritances = atomspace.getAtomsByType('INHERITANCE');

// Find all atoms with name containing "JavaScript"
const jsAtoms = atomspace.query({
  namePattern: /JavaScript/i
});

// Complex pattern matching
const patterns = atomspace.patternMatch({
  type: 'EVALUATION',
  outgoing: [
    { type: 'PREDICATE', name: 'is_a' },
    { type: 'LIST', outgoing: [{ any: true }, { type: 'CONCEPT' }] }
  ]
});
```

## 2. Economic Attention Allocation (ECAN)

### Concept

Not all knowledge is equally important. NodeCog implements **Economic Attention Networks** (ECAN) to manage scarce attentional resources:

- **STI** (Short-Term Importance): Working memory, focus of current processing
- **LTI** (Long-Term Importance): Learned long-term significance
- **VLTI** (Very Long-Term Importance): Core knowledge, protected from forgetting

### Attention Economy

Total attention is **conserved** (fixed budget):
- Atoms compete for attention
- High-attention atoms stay in working memory
- Low-attention atoms fade (forgotten)
- Attention spreads through relationships

### Attention Spreading

```javascript
// Boost attention on specific atom
attentionBank.stimulate(conceptAtom, 100); // Add 100 STI

// Spread attention through hypergraph
attentionBank.spreadAttention(conceptAtom);
```

**Spreading Algorithm**:
1. Start with source atom (has STI = 100)
2. Find all linked atoms (via INHERITANCE, EVALUATION, etc.)
3. Transfer fraction of attention: `transferAmount = STI * spreadingFactor`
4. Update target atoms: `targetSTI += transferAmount * linkWeight`
5. Update source: `sourceSTI -= totalTransferred`
6. Recursively spread to neighbors (with decay)

### Decay and Normalization

```javascript
// Natural forgetting (entropy)
attentionBank.decay(0.1); // Reduce all STI by 10%

// Maintain attention economy (constant total)
attentionBank.normalize();
```

### Use Cases

**Module Importance**:
```javascript
// Frequently accessed module gets high STI
const fsModule = nodespace.getModuleAtom('fs');
console.log('fs STI:', attentionBank.getSTI(fsModule)); // High value

// Rarely used module has low STI
const unusedModule = nodespace.getModuleAtom('./old/unused.js');
console.log('unused STI:', attentionBank.getSTI(unusedModule)); // Low value
```

**Focusing on Critical Issues**:
```javascript
// Critical vulnerability detected
const vulnAtom = atomspace.addNode('VULNERABILITY', 'CVE-2024-12345');
attentionBank.stimulate(vulnAtom, 1000); // Maximum attention!

// Spreads to affected modules
attentionBank.spreadAttention(vulnAtom);
```

## 3. Autonomous Agent System

### Concept

**Agents** are autonomous cognitive processes that:
- Operate independently
- Execute at configurable frequencies
- Analyze and modify the AtomSpace
- Coordinate through the orchestrator
- Learn from experience

### Agent Types

#### 3.1 InferenceAgent

**Purpose**: Forward-chaining logical inference

**Algorithm**:
1. Find all high-attention atoms (working memory)
2. Apply inference rules (e.g., deduction, induction)
3. Generate new atoms (inferred knowledge)
4. Set truth values based on rule confidence

**Example**:
```javascript
// Given:
// JavaScript INHERITANCE ProgrammingLanguage (TV: 0.95, 0.9)
// ProgrammingLanguage INHERITANCE ComputerSkill (TV: 0.9, 0.85)

// InferenceAgent infers:
// JavaScript INHERITANCE ComputerSkill (TV: 0.855, 0.765)
// Using PLN deduction rule
```

**Configuration**:
```javascript
const inferenceAgent = new InferenceAgent({
  frequency: 1,           // Run every cycle
  minConfidence: 0.5,     // Only use confident knowledge
  maxInferences: 100      // Limit per cycle
});
```

#### 3.2 AttentionAllocationAgent

**Purpose**: Dynamic attention management

**Algorithm**:
1. Identify high-STI atoms (focus points)
2. Spread attention through their neighborhoods
3. Apply rent (cost of staying in working memory)
4. Apply decay (forgetting over time)
5. Normalize to maintain economy

**Example**:
```javascript
const attentionAgent = new AttentionAllocationAgent({
  frequency: 1,
  spreadingFactor: 0.5,  // 50% attention transfer
  decayRate: 0.1,        // 10% decay per cycle
  rentRate: 1            // -1 STI per cycle for being in memory
});
```

#### 3.3 PatternMinerAgent

**Purpose**: Discover frequent patterns (subgraphs)

**Algorithm**:
1. Enumerate all subgraphs up to size N
2. Count occurrences of each pattern
3. Identify patterns above frequency threshold
4. Create pattern atoms to represent discoveries
5. Link patterns to their instances

**Example**:
```javascript
// Discovers pattern: EVALUATION(PREDICATE, LIST(X, Y))
// Appears in many statements
// Creates PATTERN atom to represent this structure

const patternMiner = new PatternMinerAgent({
  frequency: 10,         // Run every 10 cycles (expensive)
  minSupport: 3,         // Pattern must appear 3+ times
  maxPatternSize: 4      // Limit pattern complexity
});
```

#### 3.4 ModuleAnalyzerAgent

**Purpose**: Analyze code structure and dependencies

**Detections**:

1. **Dead Code**: Modules with zero dependencies (unused)
```javascript
// Finds:
const deadModules = atomspace.query({
  type: 'NPM_MODULE',
  filter: (atom) => {
    const incomingLinks = atomspace.getIncomingLinks(atom.id, 'DEPENDS_ON');
    return incomingLinks.length === 0;
  }
});
```

2. **Circular Dependencies**: Cycles in dependency graph
```javascript
// Finds cycles: A → B → C → A
const cycles = nodespace.detectCircularDependencies();
```

3. **Dependency Hotspots**: Over-depended modules
```javascript
// Finds modules with many dependents
const hotspots = atomspace.query({
  type: 'MODULE',
  filter: (atom) => {
    const dependents = atomspace.getIncomingLinks(atom.id, 'DEPENDS_ON');
    return dependents.length > 10;
  }
});
```

4. **Critical Path**: Longest dependency chain
```javascript
// Find deepest dependency chains
const criticalPath = nodespace.getDependencyChain(entryPoint);
console.log('Max depth:', criticalPath.length);
```

**Configuration**:
```javascript
const moduleAnalyzer = new ModuleAnalyzerAgent({
  frequency: 5,            // Run every 5 cycles
  deadCodeThreshold: 0,    // Zero dependencies = dead
  hotspotThreshold: 10,    // 10+ dependents = hotspot
  circularDetection: true  // Enable cycle detection
});
```

### Custom Agents

Create domain-specific agents:

```javascript
class SecurityAnalyzerAgent extends Agent {
  constructor() {
    super('SecurityAnalyzer', 10); // Every 10 cycles
  }

  async execute(atomspace, attentionBank) {
    // Get all NPM modules
    const modules = atomspace.getAtomsByType('NPM_MODULE');
    
    for (const module of modules) {
      // Check CVE database
      const vulnerabilities = await this.checkCVE(module.name);
      
      if (vulnerabilities.length > 0) {
        // Create vulnerability atoms
        for (const vuln of vulnerabilities) {
          const vulnAtom = atomspace.addNode('CVE', vuln.id);
          atomspace.setTruthValue(vulnAtom, {
            strength: vuln.severity / 10,  // 0.0 to 1.0
            confidence: 0.95
          });
          
          // Link to module
          atomspace.addLink('HAS_VULNERABILITY', [module.id, vulnAtom.id]);
          
          // Boost attention (alert!)
          attentionBank.stimulate(vulnAtom, 1000);
        }
      }
    }
  }
  
  async checkCVE(packageName) {
    // Query CVE database
    // Return array of vulnerabilities
  }
}

// Register
orchestrator.registerAgent(new SecurityAnalyzerAgent());
```

## 4. Pattern Mining

### Frequent Subgraph Mining

Discovers implicit patterns in knowledge:

```javascript
// Given many instances of:
// EVALUATION(likes, LIST(User1, JavaScript))
// EVALUATION(likes, LIST(User2, JavaScript))
// EVALUATION(likes, LIST(User3, JavaScript))

// PatternMiner discovers:
// PATTERN: EVALUATION(likes, LIST(User, JavaScript))
// Frequency: 3 (appears in 3 instances)
```

### Pattern Atoms

Discovered patterns become first-class atoms:

```javascript
const pattern = atomspace.addNode('PATTERN', 'likes_javascript_pattern');

// Link pattern to instances
for (const instance of instances) {
  atomspace.addLink('INSTANTIATES', [instance.id, pattern.id]);
}

// Pattern has attention based on frequency
attentionBank.stimulate(pattern, frequency * 10);
```

### Use Cases

1. **Code Patterns**: Detect common coding patterns
2. **Dependency Patterns**: Find typical module usage
3. **Anti-Patterns**: Identify problematic structures
4. **Optimization Opportunities**: Discover redundant code

## 5. Inference and Reasoning

### Forward Chaining

Start with facts, apply rules, generate new knowledge:

```javascript
// Fact 1: TypeScript IS_A JavaScript (strength: 0.8)
const ts = atomspace.addNode('CONCEPT', 'TypeScript');
const js = atomspace.addNode('CONCEPT', 'JavaScript');
const isA1 = atomspace.addLink('INHERITANCE', [ts, js]);
atomspace.setTruthValue(isA1, { strength: 0.8, confidence: 0.9 });

// Fact 2: JavaScript IS_A ProgrammingLanguage (strength: 0.95)
const progLang = atomspace.addNode('CONCEPT', 'ProgrammingLanguage');
const isA2 = atomspace.addLink('INHERITANCE', [js, progLang]);
atomspace.setTruthValue(isA2, { strength: 0.95, confidence: 0.9 });

// InferenceAgent applies transitivity rule:
// TypeScript IS_A ProgrammingLanguage (inferred)
// strength = 0.8 * 0.95 = 0.76
// confidence = 0.9 * 0.9 = 0.81
```

### Backward Chaining (Planned)

Start with goal, find facts/rules to prove it:

```javascript
// Goal: Prove "X is secure"
// Backward chainer searches for:
// - Rules that conclude "X is secure"
// - Facts that support those rules
// - Recursively prove sub-goals
```

### Probabilistic Logic Networks (PLN)

Future kernel implementation will include:
- **Deduction**: A→B, B→C ⊢ A→C
- **Induction**: A→B observed frequently ⊢ A→B general rule
- **Abduction**: B observed, B→A known ⊢ A likely
- **Revision**: Combine conflicting evidence
- **Analogy**: A→B similar to C→D

## 6. Distributed Cognition

### Multi-Node AtomSpace

Synchronize knowledge across multiple Node.js processes:

```javascript
// Node 1: Web server
const dist1 = new DistributedAtomSpace(atomspace, {
  nodeId: 'web-server-1',
  peers: ['worker-1', 'worker-2']
});

// Node 2: Background worker
const dist2 = new DistributedAtomSpace(atomspace, {
  nodeId: 'worker-1',
  peers: ['web-server-1', 'worker-2']
});

// Changes on Node 1 propagate to Node 2
const concept = atomspace1.addNode('CONCEPT', 'SharedKnowledge');
// Automatically synced to atomspace2
```

### Synchronization Protocol

1. **Local Changes**: Track modifications to AtomSpace
2. **Change Events**: Emit events for adds/updates/deletes
3. **Serialization**: Convert atoms to wire format
4. **Transmission**: Send via IPC/TCP to peers
5. **Conflict Resolution**: Merge concurrent changes
6. **Apply Remote**: Update local AtomSpace

### Conflict Resolution

When same atom modified on multiple nodes:

**Strategy 1: Last-Write-Wins** (LWW)
```javascript
// Use timestamp to decide
if (remoteTimestamp > localTimestamp) {
  applyRemoteChange();
}
```

**Strategy 2: Truth Value Merging**
```javascript
// Combine truth values probabilistically
const merged = {
  strength: (local.strength + remote.strength) / 2,
  confidence: Math.sqrt(local.confidence * remote.confidence)
};
```

**Strategy 3: Attention-Based**
```javascript
// Higher attention wins
if (remoteSTI > localSTI) {
  applyRemoteChange();
}
```

### Attention Propagation

Attention spreads across nodes:

```javascript
// High attention on Node 1
attentionBank1.stimulate(concept, 1000);

// Propagates to Node 2
dist1.syncAttention(concept);

// Node 2 now has high STI for concept
console.log(attentionBank2.getSTI(concept)); // High value
```

## 7. Meta-Optimization (MOSES)

### Concept

**MOSES** (Meta-Optimizing Semantic Evolutionary Search) uses genetic algorithms to evolve better agent programs:

```javascript
// Initial population: Random agent configurations
const population = [
  { spreadingFactor: 0.3, decayRate: 0.1, fitness: 0 },
  { spreadingFactor: 0.5, decayRate: 0.2, fitness: 0 },
  { spreadingFactor: 0.7, decayRate: 0.15, fitness: 0 },
  // ... more individuals
];

// Evaluate fitness
for (const individual of population) {
  individual.fitness = evaluateFitness(individual);
}

// Selection: Keep best performers
const selected = population.sort((a, b) => b.fitness - a.fitness).slice(0, 10);

// Crossover: Combine successful traits
const offspring = [];
for (let i = 0; i < selected.length; i += 2) {
  offspring.push(crossover(selected[i], selected[i+1]));
}

// Mutation: Random changes
offspring.forEach(ind => mutate(ind));

// Next generation
population = [...selected, ...offspring];
```

### Fitness Functions

Define what makes a good agent:

```javascript
function evaluateFitness(agentConfig) {
  // Create agent with config
  const agent = new AttentionAllocationAgent(agentConfig);
  
  // Run on test scenario
  const testSpace = createTestAtomSpace();
  agent.execute(testSpace, testBank);
  
  // Measure performance
  const metrics = {
    attentionUtilization: testBank.getUtilization(),
    knowledgeRetention: testSpace.getRetention(),
    processingSpeed: agent.getAvgExecutionTime()
  };
  
  // Weighted fitness score
  return (
    metrics.attentionUtilization * 0.4 +
    metrics.knowledgeRetention * 0.4 +
    (1 / metrics.processingSpeed) * 0.2
  );
}
```

### Applications

1. **Agent Optimization**: Evolve better agent parameters
2. **Rule Synthesis**: Generate new inference rules
3. **Pattern Discovery**: Find optimal pattern mining strategies
4. **Scheduler Tuning**: Optimize cognitive scheduling

## 8. Planning System

### Goal-Directed Behavior

```javascript
// Define goal
const goal = atomspace.addNode('GOAL', 'OptimizePerformance');
atomspace.setTruthValue(goal, { strength: 1.0, confidence: 1.0 });

// Define actions
const action1 = atomspace.addNode('ACTION', 'EnableCodeCache');
const action2 = atomspace.addNode('ACTION', 'ReduceMemoryUsage');
const action3 = atomspace.addNode('ACTION', 'OptimizeDependencies');

// Link actions to goal (achieves)
atomspace.addLink('ACHIEVES', [action1, goal]);
atomspace.addLink('ACHIEVES', [action2, goal]);
atomspace.addLink('ACHIEVES', [action3, goal]);

// Planning agent generates plan
const plan = planningAgent.generatePlan(goal);
console.log('Plan steps:', plan.actions);

// Execute plan
planningAgent.executePlan(plan);
```

### Hierarchical Planning

Break complex goals into sub-goals:

```javascript
// High-level goal
const mainGoal = atomspace.addNode('GOAL', 'ImproveApplication');

// Sub-goals
const subGoal1 = atomspace.addNode('GOAL', 'ReduceLatency');
const subGoal2 = atomspace.addNode('GOAL', 'IncreaseReliability');
const subGoal3 = atomspace.addNode('GOAL', 'OptimizeCosts');

// Hierarchical structure
atomspace.addLink('SUBGOAL_OF', [subGoal1, mainGoal]);
atomspace.addLink('SUBGOAL_OF', [subGoal2, mainGoal]);
atomspace.addLink('SUBGOAL_OF', [subGoal3, mainGoal]);

// Plan at each level
const hierarchicalPlan = planningAgent.generateHierarchicalPlan(mainGoal);
```

## 9. NLP Integration

### Natural Language Understanding

```javascript
// Parse natural language
const text = "Node.js is a JavaScript runtime built on V8";

// NLP processing
const entities = nlpAgent.extractEntities(text);
// => [{ text: "Node.js", type: "TECHNOLOGY" },
//     { text: "JavaScript", type: "LANGUAGE" },
//     { text: "V8", type: "ENGINE" }]

// Ground in AtomSpace
const nodejs = atomspace.addNode('CONCEPT', 'Node.js');
const javascript = atomspace.addNode('CONCEPT', 'JavaScript');
const v8 = atomspace.addNode('CONCEPT', 'V8');

// Extract relationships
const isA = atomspace.addLink('INHERITANCE', [nodejs, javascript]);
const builtOn = atomspace.addLink('EVALUATION', [
  atomspace.addNode('PREDICATE', 'built_on'),
  atomspace.addLink('LIST', [nodejs, v8])
]);
```

### Intent Classification

```javascript
// User query
const query = "How do I optimize my module dependencies?";

// Classify intent
const intent = nlpAgent.classifyIntent(query);
// => { intent: "OPTIMIZATION_REQUEST", domain: "DEPENDENCIES" }

// Map to cognitive action
if (intent.intent === "OPTIMIZATION_REQUEST") {
  // Activate ModuleAnalyzerAgent
  attentionBank.stimulate(moduleAnalyzerAgent.id, 1000);
  
  // Focus on dependencies
  const depAtoms = atomspace.getAtomsByType('DEPENDS_ON');
  depAtoms.forEach(atom => attentionBank.stimulate(atom, 100));
}
```

## Summary

NodeCog's cognitive features enable:

1. **Intelligent Knowledge Management**: Hypergraph + attention
2. **Autonomous Operation**: Self-managing agents
3. **Learning**: Pattern mining and meta-optimization
4. **Reasoning**: Inference and planning
5. **Distribution**: Multi-node coordination
6. **Language Understanding**: NLP integration

These capabilities transform Node.js from a runtime into a **cognitive platform** for AGI research and intelligent applications.

## References

- `architecture.md` - System architecture details
- `kernel-layer.md` - Future high-performance kernel
- `api-reference.md` - Complete API documentation
- `doc/opencog/PHASE4_FEATURES.md` - Advanced cognitive features
