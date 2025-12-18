/**
 * Attention-Based Scheduling Example
 * 
 * This example demonstrates how attention values (STI/LTI) control
 * execution scheduling and resource allocation.
 */

const { createEngine } = require('../lib/cognitive_api');

console.log('========================================');
console.log('Attention-Based Scheduling Example');
console.log('========================================\n');

// Create engine with attention-based scheduling enabled
const engine = createEngine({
  cognitiveTick: 5,
  attentionBased: true,
  monitoring: true,
});

console.log('Creating cognitive agents...\n');

// Create three agents with different priorities
const critical = engine.createIsolate('critical', {
  sti: 150,  // Very high priority
  lti: 80,
});

const normal = engine.createIsolate('normal', {
  sti: 50,   // Normal priority
  lti: 50,
});

const background = engine.createIsolate('background', {
  sti: 10,   // Very low priority
  lti: 30,
});

// Helper to display state
function displayState(title) {
  console.log(`\n${title}`);
  console.log('─'.repeat(50));
  console.log(`Critical:   STI=${critical.getSTI().toFixed(1).padStart(6)}  LTI=${critical.getLTI().toFixed(1).padStart(6)}`);
  console.log(`Normal:     STI=${normal.getSTI().toFixed(1).padStart(6)}  LTI=${normal.getLTI().toFixed(1).padStart(6)}`);
  console.log(`Background: STI=${background.getSTI().toFixed(1).padStart(6)}  LTI=${background.getLTI().toFixed(1).padStart(6)}`);
  console.log('─'.repeat(50));
}

displayState('Initial State');

// Scenario 1: Critical task runs and completes
console.log('\n📌 Scenario 1: Critical task completes');
console.log('   → Critical task used resources, STI decays');
critical.decay(30);
displayState('After critical task');

// Scenario 2: Normal task becomes urgent
console.log('\n📌 Scenario 2: Normal task becomes urgent');
console.log('   → External event boosts normal task priority');
normal.boost(80);
displayState('After urgency boost');

// Scenario 3: Background task gets a chance
console.log('\n📌 Scenario 3: Background task finally runs');
console.log('   → All high-priority tasks idle, background runs');
critical.decay(40);
normal.decay(60);
background.boost(30);
displayState('After background execution');

// Scenario 4: Crisis situation
console.log('\n📌 Scenario 4: Crisis - critical priority spike');
console.log('   → Emergency situation requires immediate attention');
critical.setSTI(200);  // Maximum attention
background.setSTI(5);  // Minimize background
displayState('During crisis');

// Scenario 5: Return to equilibrium
console.log('\n📌 Scenario 5: Return to equilibrium');
console.log('   → Crisis resolved, attention rebalances');
critical.setSTI(80);
normal.setSTI(50);
background.setSTI(20);
displayState('Equilibrium restored');

// Show scheduling implications
console.log('\n\nScheduling Analysis:');
console.log('═'.repeat(50));

const agents = [
  { name: 'Critical', isolate: critical },
  { name: 'Normal', isolate: normal },
  { name: 'Background', isolate: background },
];

// Sort by STI (scheduling priority)
agents.sort((a, b) => b.isolate.getSTI() - a.isolate.getSTI());

console.log('\nExecution Order (highest STI first):');
agents.forEach((agent, index) => {
  const sti = agent.isolate.getSTI();
  const percentage = (sti / agents.reduce((sum, a) => sum + a.isolate.getSTI(), 0) * 100).toFixed(1);
  console.log(`  ${index + 1}. ${agent.name.padEnd(12)} STI: ${sti.toFixed(1).padStart(6)}  (~${percentage}% CPU time)`);
});

console.log('\nMemory Budget (based on LTI):');
agents.forEach((agent) => {
  const lti = agent.isolate.getLTI();
  const budget = Math.floor(lti * 10);  // Simplified: LTI * 10MB
  console.log(`  ${agent.name.padEnd(12)} LTI: ${lti.toFixed(1).padStart(6)}  (~${budget}MB budget)`);
});

// Attention economics principles
console.log('\n\nAttention Economics Principles:');
console.log('═'.repeat(50));
console.log('1. STI (Short-Term Importance):');
console.log('   • Determines execution priority');
console.log('   • Higher STI = more CPU time');
console.log('   • Decays over time (forgetting)');
console.log('   • Boosts on use (reinforcement)');
console.log();
console.log('2. LTI (Long-Term Importance):');
console.log('   • Determines memory budget');
console.log('   • Higher LTI = more heap space');
console.log('   • Resistant to decay');
console.log('   • Grows with repeated use');
console.log();
console.log('3. Cognitive Scheduling:');
console.log('   • Select isolate with highest STI');
console.log('   • Execute bounded microtasks');
console.log('   • Update attention after execution');
console.log('   • Balance fairness vs. priority');
console.log();

// Get statistics
const stats = engine.getStatistics();
console.log('Current System State:');
console.log(`  Isolates: ${stats.isolateCount}`);
console.log(`  Average STI: ${stats.averageSTI.toFixed(2)}`);
console.log(`  Average LTI: ${stats.averageLTI.toFixed(2)}`);
console.log(`  Total memory: ${(stats.totalMemory / 1024).toFixed(1)} KB`);
console.log();

// Cleanup
console.log('Cleaning up...');
engine.destroyIsolate('critical');
engine.destroyIsolate('normal');
engine.destroyIsolate('background');
engine.destroy();
console.log('✓ Done\n');

console.log('========================================');
console.log('Attention-Based Scheduling Complete');
console.log('========================================');
