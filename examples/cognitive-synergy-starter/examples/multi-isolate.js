/**
 * Multi-Isolate Example
 * 
 * This example demonstrates creating and managing multiple isolates
 * with different attention values and observing how they compete for resources.
 */

const { createEngine } = require('../lib/cognitive_api');

console.log('========================================');
console.log('Multi-Isolate Cognitive System');
console.log('========================================\n');

// Create engine
const engine = createEngine({
  cognitiveTick: 5,
  workerThreads: 4,
  attentionBased: true,
  monitoring: true,
});

console.log('Creating multiple cognitive isolates...\n');

// Create isolates for different cognitive functions
const isolates = {
  reasoning: engine.createIsolate('reasoning', {
    sti: 100,  // Highest priority
    lti: 90,
  }),
  
  perception: engine.createIsolate('perception', {
    sti: 80,
    lti: 70,
  }),
  
  planning: engine.createIsolate('planning', {
    sti: 60,
    lti: 80,
  }),
  
  memory: engine.createIsolate('memory', {
    sti: 40,
    lti: 95,  // High memory budget
  }),
  
  background: engine.createIsolate('background', {
    sti: 20,  // Low priority
    lti: 50,
  }),
};

// Display initial state
console.log('Initial State:');
console.log('┌─────────────┬──────┬──────┬──────────────┐');
console.log('│ Isolate     │ STI  │ LTI  │ Memory (KB)  │');
console.log('├─────────────┼──────┼──────┼──────────────┤');
for (const [name, isolate] of Object.entries(isolates)) {
  const sti = isolate.getSTI().toFixed(1).padStart(5);
  const lti = isolate.getLTI().toFixed(1).padStart(5);
  const mem = (isolate.getMemoryUsage() / 1024).toFixed(1).padStart(11);
  console.log(`│ ${name.padEnd(11)} │ ${sti} │ ${lti} │ ${mem}  │`);
}
console.log('└─────────────┴──────┴──────┴──────────────┘\n');

// Simulate cognitive activities
console.log('Simulating cognitive activities...\n');

// Reasoning needs more attention
console.log('→ Reasoning task requires focus');
isolates.reasoning.boost(30);
console.log(`  Reasoning STI: ${isolates.reasoning.getSTI()}\n`);

// Background task completes, attention decays
console.log('→ Background task completes');
isolates.background.decay(15);
console.log(`  Background STI: ${isolates.background.getSTI()}\n`);

// Planning becomes urgent
console.log('→ Planning becomes urgent');
isolates.planning.setSTI(110);
console.log(`  Planning STI: ${isolates.planning.getSTI()}\n`);

// Memory system accessed frequently
console.log('→ Memory system accessed frequently');
isolates.memory.boost(25);
console.log(`  Memory STI: ${isolates.memory.getSTI()}\n`);

// Display updated state
console.log('Updated State:');
console.log('┌─────────────┬──────┬──────┬──────────────┐');
console.log('│ Isolate     │ STI  │ LTI  │ Memory (KB)  │');
console.log('├─────────────┼──────┼──────┼──────────────┤');
for (const [name, isolate] of Object.entries(isolates)) {
  const sti = isolate.getSTI().toFixed(1).padStart(5);
  const lti = isolate.getLTI().toFixed(1).padStart(5);
  const mem = (isolate.getMemoryUsage() / 1024).toFixed(1).padStart(11);
  console.log(`│ ${name.padEnd(11)} │ ${sti} │ ${lti} │ ${mem}  │`);
}
console.log('└─────────────┴──────┴──────┴──────────────┘\n');

// Show execution priority order (highest STI first)
console.log('Execution Priority Order:');
const sorted = Object.entries(isolates)
  .sort((a, b) => b[1].getSTI() - a[1].getSTI());

sorted.forEach(([name, isolate], index) => {
  console.log(`  ${index + 1}. ${name.padEnd(11)} (STI: ${isolate.getSTI().toFixed(1)})`);
});
console.log();

// Get overall statistics
const stats = engine.getStatistics();
console.log('System Statistics:');
console.log(`  Total isolates: ${stats.isolateCount}`);
console.log(`  Total memory: ${(stats.totalMemory / 1024).toFixed(1)} KB`);
console.log(`  Average STI: ${stats.averageSTI.toFixed(2)}`);
console.log(`  Average LTI: ${stats.averageLTI.toFixed(2)}\n`);

// Cleanup
console.log('Cleaning up...');
for (const name of Object.keys(isolates)) {
  engine.destroyIsolate(name);
}
engine.destroy();
console.log('✓ Done\n');

console.log('========================================');
console.log('Multi-Isolate Example Complete');
console.log('========================================');
