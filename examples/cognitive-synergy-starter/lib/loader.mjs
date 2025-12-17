/**
 * NodeSpace ESM Loader Hooks
 * 
 * This loader integrates the ES Module loader with OpenCog's NodeSpace,
 * creating a live software hypergraph in the AtomSpace.
 * 
 * Usage: node --experimental-loader ./lib/loader.mjs script.js
 */

import { pathToFileURL, fileURLToPath } from 'url';

// Global NodeSpace instance (if available)
let nodespace = null;
let atomspace = null;

/**
 * Initialize the loader
 */
export function initialize(data) {
  console.log('[NodeSpace Loader] Initializing...');
  
  // Try to get NodeSpace from global context
  if (data && data.nodespace) {
    nodespace = data.nodespace;
  } else if (globalThis.process?.opencog?.nodespace) {
    nodespace = globalThis.process.opencog.nodespace;
  }
  
  if (data && data.atomspace) {
    atomspace = data.atomspace;
  } else if (globalThis.process?.opencog?.atomspace) {
    atomspace = globalThis.process.opencog.atomspace;
  }
  
  if (nodespace) {
    console.log('[NodeSpace Loader] ✓ Connected to NodeSpace');
  } else {
    console.log('[NodeSpace Loader] ⚠ NodeSpace not available (operating in tracking mode)');
  }
}

// Simple in-memory module graph for tracking when NodeSpace is not available
const moduleGraph = {
  modules: new Map(),
  dependencies: new Map(),
};

/**
 * Record module in graph
 */
function recordModule(url, metadata = {}) {
  if (!moduleGraph.modules.has(url)) {
    moduleGraph.modules.set(url, {
      url,
      sti: 0,
      lti: 50,
      loadCount: 0,
      firstLoaded: Date.now(),
      lastLoaded: Date.now(),
      ...metadata,
    });
  }
  
  const module = moduleGraph.modules.get(url);
  module.loadCount++;
  module.lastLoaded = Date.now();
  
  return module;
}

/**
 * Record dependency
 */
function recordDependency(parent, child, metadata = {}) {
  const key = `${parent}->${child}`;
  
  if (!moduleGraph.dependencies.has(key)) {
    moduleGraph.dependencies.set(key, {
      parent,
      child,
      sti: 0,
      useCount: 0,
      firstUsed: Date.now(),
      lastUsed: Date.now(),
      ...metadata,
    });
  }
  
  const dep = moduleGraph.dependencies.get(key);
  dep.useCount++;
  dep.lastUsed = Date.now();
  
  return dep;
}

/**
 * Resolve hook - intercepts module resolution
 */
export async function resolve(specifier, context, nextResolve) {
  const result = await nextResolve(specifier, context);
  
  // Record import relationship
  if (context.parentURL) {
    try {
      if (nodespace) {
        // Use NodeSpace if available
        nodespace.recordImport(context.parentURL, result.url, {
          format: result.format,
          timestamp: Date.now(),
        });
        
        // Boost STI for imported modules
        const parentModule = nodespace.getModule(context.parentURL);
        if (parentModule && parentModule.atom) {
          parentModule.atom.sti = Math.min(100, (parentModule.atom.sti || 0) + 1);
        }
      } else {
        // Use local tracking
        const dep = recordDependency(context.parentURL, result.url, {
          format: result.format,
        });
        
        // Boost STI
        const parentModule = moduleGraph.modules.get(context.parentURL);
        if (parentModule) {
          parentModule.sti = Math.min(100, parentModule.sti + 1);
        }
      }
    } catch (error) {
      // Don't break module loading
      console.error('[NodeSpace Loader] Recording error:', error.message);
    }
  }
  
  return result;
}

/**
 * Load hook - intercepts module loading
 */
export async function load(url, context, nextLoad) {
  const result = await nextLoad(url, context);
  
  // Record module
  try {
    if (nodespace) {
      // Use NodeSpace if available
      nodespace.recordModule(url, {
        format: result.format,
        timestamp: Date.now(),
      });
      
      // Track as active
      const module = nodespace.getModule(url);
      if (module && module.atom) {
        module.atom.sti = Math.min(100, (module.atom.sti || 0) + 5);
        module.lastLoaded = Date.now();
      }
    } else {
      // Use local tracking
      const module = recordModule(url, {
        format: result.format,
      });
      
      // Boost STI on load
      module.sti = Math.min(100, module.sti + 5);
    }
    
    // Extract exports if possible
    if (result.format === 'module' && result.source) {
      const exports = extractExports(result.source.toString());
      
      if (nodespace) {
        for (const exportName of exports) {
          nodespace.recordExport(url, exportName);
        }
      }
    }
  } catch (error) {
    // Don't break module loading
    console.error('[NodeSpace Loader] Recording error:', error.message);
  }
  
  return result;
}

/**
 * Extract exported identifiers from module source
 */
function extractExports(source) {
  const exports = new Set();
  
  // Match: export function name(...
  // Match: export class name...
  // Match: export const/let/var name...
  const directExports = source.matchAll(/export\s+(?:async\s+)?(?:function|class|const|let|var)\s+(\w+)/g);
  for (const match of directExports) {
    exports.add(match[1]);
  }
  
  // Match: export { name, name as alias }
  const namedExports = source.matchAll(/export\s*{([^}]+)}/g);
  for (const match of namedExports) {
    const names = match[1].split(',').map(s => s.trim().split(/\s+as\s+/)[0]);
    for (const name of names) {
      if (name) exports.add(name);
    }
  }
  
  // Match: export default
  if (/export\s+default\s+/.test(source)) {
    exports.add('default');
  }
  
  return Array.from(exports);
}

/**
 * Global preload hook
 */
export function globalPreload(context) {
  return `
    // NodeSpace Loader Global Setup
    console.log('[NodeSpace Loader] Global preload executing');
    
    // Make module graph accessible globally for debugging
    globalThis.__nodespace_module_graph = {
      getModules() {
        // This will be populated by the loader
        return [];
      },
      getDependencies() {
        return [];
      },
      getStatistics() {
        return {
          modules: 0,
          dependencies: 0,
          totalSTI: 0,
        };
      }
    };
  `;
}

// Export module graph for debugging
export function getModuleGraph() {
  return {
    modules: Array.from(moduleGraph.modules.values()),
    dependencies: Array.from(moduleGraph.dependencies.values()),
    statistics: {
      moduleCount: moduleGraph.modules.size,
      dependencyCount: moduleGraph.dependencies.size,
      totalSTI: Array.from(moduleGraph.modules.values())
        .reduce((sum, mod) => sum + mod.sti, 0),
      averageSTI: Array.from(moduleGraph.modules.values())
        .reduce((sum, mod) => sum + mod.sti, 0) / moduleGraph.modules.size || 0,
    },
  };
}
