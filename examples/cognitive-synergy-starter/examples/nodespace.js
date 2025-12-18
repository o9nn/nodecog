/**
 * NodeSpace Integration Example
 * 
 * This example demonstrates how the ESM loader creates a live software
 * hypergraph by tracking module dependencies and updating attention values.
 * 
 * Run with: node --experimental-loader ../lib/loader.mjs nodespace.js
 */

console.log('========================================');
console.log('NodeSpace Integration Example');
console.log('========================================\n');

console.log('This example demonstrates ESM loader hooks that:');
console.log('  1. Track module imports/exports');
console.log('  2. Create dependency graph');
console.log('  3. Update attention on module use');
console.log('  4. Build live software hypergraph\n');

// Import some modules to trigger the loader
import { readFileSync } from 'fs';
import { join } from 'path';
import { fileURLToPath } from 'url';

console.log('✓ Modules imported\n');

// The loader has now tracked these imports
console.log('Module Dependencies Tracked:');
console.log('  fs → readFileSync');
console.log('  path → join');
console.log('  url → fileURLToPath\n');

// In a real OpenCog integration, these would be AtomSpace links:
console.log('Equivalent AtomSpace Structure:');
console.log('  (ImportLink');
console.log('    (ModuleNode "nodespace.js")');
console.log('    (ModuleNode "fs"))');
console.log();
console.log('  (ExportLink');
console.log('    (ModuleNode "fs")');
console.log('    (SymbolNode "readFileSync"))');
console.log();

// Demonstrate attention updates
console.log('Attention Updates:');
console.log('  • fs module STI: +5 (loaded)');
console.log('  • fs module STI: +1 (imported by nodespace.js)');
console.log('  • Current fs STI: ~6');
console.log();

// Show how repeated imports boost attention
console.log('Repeated Import Pattern:');
console.log('  Import 1: STI += 5');
console.log('  Import 2: STI += 5');
console.log('  Import 3: STI += 5');
console.log('  → Frequently used modules get higher attention');
console.log('  → Unused modules decay over time\n');

// NodeSpace query examples (conceptual)
console.log('NodeSpace Query Examples:');
console.log('═'.repeat(50));
console.log();
console.log('1. Get module dependencies:');
console.log('   nodespace.getDependencies("app.js")');
console.log('   → ["fs", "path", "express", "lodash"]');
console.log();
console.log('2. Find who imports a module:');
console.log('   nodespace.getImporters("lodash")');
console.log('   → ["utils.js", "helpers.js", "data.js"]');
console.log();
console.log('3. Get module attention:');
console.log('   nodespace.getModuleSTI("express")');
console.log('   → 87.5 (heavily used)');
console.log();
console.log('4. Find hot modules:');
console.log('   nodespace.getHotModules(threshold: 80)');
console.log('   → ["express", "react", "lodash"]');
console.log();
console.log('5. Analyze dependency depth:');
console.log('   nodespace.getDependencyDepth("app.js")');
console.log('   → { maxDepth: 5, avgDepth: 3.2 }');
console.log();

// Attention-based module loading
console.log('\nAttention-Based Module Loading:');
console.log('═'.repeat(50));
console.log();
console.log('High-attention modules (STI > 80):');
console.log('  • Preload into memory');
console.log('  • Cache compiled code');
console.log('  • Pin in V8 heap');
console.log();
console.log('Medium-attention modules (50 < STI < 80):');
console.log('  • Load on demand');
console.log('  • Cache temporarily');
console.log('  • Monitor usage');
console.log();
console.log('Low-attention modules (STI < 50):');
console.log('  • Load lazily');
console.log('  • Evict from cache');
console.log('  • Consider for removal');
console.log();

// Cognitive implications
console.log('\nCognitive Implications:');
console.log('═'.repeat(50));
console.log();
console.log('Software as Knowledge Graph:');
console.log('  • Modules = Concepts (nodes)');
console.log('  • Imports = Relationships (links)');
console.log('  • Attention = Importance/Usage');
console.log('  • Graph = Living system state');
console.log();
console.log('Attention-Driven Optimization:');
console.log('  • Hot code paths get priority');
console.log('  • Unused code fades away');
console.log('  • System learns usage patterns');
console.log('  • Self-optimizing architecture');
console.log();

// Integration with Cognitive Synergy Engine
console.log('Integration with Cognitive Synergy Engine:');
console.log('═'.repeat(50));
console.log();
console.log('1. Module STI → Isolate STI');
console.log('   High-attention modules run in high-STI isolates');
console.log();
console.log('2. Dependency Clusters → Isolate Groups');
console.log('   Related modules share isolates');
console.log();
console.log('3. Usage Patterns → Scheduling Hints');
console.log('   Frequently co-imported modules scheduled together');
console.log();
console.log('4. Load Events → Attention Updates');
console.log('   Module loads trigger cognitive events');
console.log();

console.log('========================================');
console.log('NodeSpace Integration Example Complete');
console.log('========================================');
console.log();
console.log('To see the loader in action, run:');
console.log('  node --experimental-loader ../lib/loader.mjs nodespace.js');
console.log();
console.log('The loader will output tracking information showing:');
console.log('  • Module resolution events');
console.log('  • Import relationship recording');
console.log('  • Attention value updates');
console.log('  • Dependency graph construction');
