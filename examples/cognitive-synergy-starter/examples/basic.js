/**
 * Basic Cognitive Synergy Engine Example
 * 
 * This example demonstrates the basic usage of the Cognitive Synergy Engine.
 */

const { createEngine } = require('../lib/cognitive_api');

console.log('========================================');
console.log('Basic Cognitive Synergy Engine Example');
console.log('========================================\n');

// Create and initialize the engine
console.log('1. Creating engine...');
const engine = createEngine({
  cognitiveTick: 5,      // 5ms cognitive loop tick
  workerThreads: 4,       // 4 libuv worker threads
  attentionBased: true,   // Enable attention-based scheduling
});
console.log('   ✓ Engine created\n');

// Create a single isolate
console.log('2. Creating isolate...');
const agent = engine.createIsolate('agent', {
  sti: 100,  // High priority
  lti: 80,   // Medium-high memory budget
});
console.log(`   ✓ Isolate created: ${agent.id}`);
console.log(`   - STI: ${agent.getSTI()}`);
console.log(`   - LTI: ${agent.getLTI()}`);
console.log(`   - Memory: ${agent.getMemoryUsage()} bytes\n`);

// Demonstrate attention manipulation
console.log('3. Manipulating attention...');
console.log(`   Initial STI: ${agent.getSTI()}`);

agent.boost(20);
console.log(`   After boost(20): ${agent.getSTI()}`);

agent.decay(10);
console.log(`   After decay(10): ${agent.getSTI()}`);

agent.setSTI(75);
console.log(`   After setSTI(75): ${agent.getSTI()}\n`);

// Get engine statistics
console.log('4. Engine statistics:');
const stats = engine.getStatistics();
console.log(`   - Isolates: ${stats.isolateCount}`);
console.log(`   - Total memory: ${stats.totalMemory} bytes`);
console.log(`   - Average STI: ${stats.averageSTI.toFixed(2)}`);
console.log(`   - Average LTI: ${stats.averageLTI.toFixed(2)}\n`);

// Cleanup
console.log('5. Cleaning up...');
engine.destroyIsolate('agent');
engine.destroy();
console.log('   ✓ Engine destroyed\n');

console.log('========================================');
console.log('Example Complete');
console.log('========================================');
