/**
 * Cognitive Synergy Engine - JavaScript API
 * 
 * This module provides a high-level JavaScript API for the Cognitive Synergy Engine.
 * It allows you to create and manage V8 isolates with attention-based scheduling.
 */

'use strict';

// Try to load the native binding if available
let cognitiveBinding = null;
try {
  const { internalBinding } = require('internal/bootstrap/realm');
  cognitiveBinding = internalBinding('cognitive_synergy');
} catch (e) {
  // Binding not available - this is expected when running standalone
  console.warn('Native cognitive synergy binding not available');
  console.warn('This module requires Node.js built with Cognitive Synergy Engine');
}

/**
 * Represents an isolated V8 execution context with cognitive control
 */
class IsolateContext {
  constructor(engine, id, options = {}) {
    this._engine = engine;
    this._id = id;
    this._sti = options.sti || 50.0;
    this._lti = options.lti || 50.0;
  }
  
  get id() {
    return this._id;
  }
  
  /**
   * Get Short-Term Importance (execution priority)
   */
  getSTI() {
    if (cognitiveBinding) {
      return cognitiveBinding.getSTI(this._id);
    }
    return this._sti;
  }
  
  /**
   * Set Short-Term Importance
   */
  setSTI(value) {
    if (cognitiveBinding) {
      cognitiveBinding.setSTI(this._id, value);
    }
    this._sti = value;
  }
  
  /**
   * Get Long-Term Importance (memory budget)
   */
  getLTI() {
    if (cognitiveBinding) {
      return cognitiveBinding.getLTI(this._id);
    }
    return this._lti;
  }
  
  /**
   * Set Long-Term Importance
   */
  setLTI(value) {
    if (cognitiveBinding) {
      cognitiveBinding.setLTI(this._id, value);
    }
    this._lti = value;
  }
  
  /**
   * Boost attention (increase STI)
   */
  boost(amount = 10) {
    this.setSTI(this.getSTI() + amount);
  }
  
  /**
   * Decay attention (decrease STI)
   */
  decay(amount = 10) {
    this.setSTI(Math.max(0, this.getSTI() - amount));
  }
  
  /**
   * Get memory usage in bytes
   */
  getMemoryUsage() {
    if (cognitiveBinding) {
      return cognitiveBinding.getMemoryUsage(this._id);
    }
    return 0;
  }
  
  /**
   * Get CPU time in milliseconds
   */
  getCPUTime() {
    if (cognitiveBinding) {
      return cognitiveBinding.getCPUTime(this._id);
    }
    return 0;
  }
  
  /**
   * Run code in this isolate (if supported)
   */
  run(code) {
    if (cognitiveBinding && cognitiveBinding.runInIsolate) {
      return cognitiveBinding.runInIsolate(this._id, code);
    }
    throw new Error('runInIsolate not available');
  }
}

/**
 * Main Cognitive Synergy Engine class
 */
class CognitiveSynergyEngine {
  constructor(config = {}) {
    this.config = {
      cognitiveTick: config.cognitiveTick || 5,
      workerThreads: config.workerThreads || 4,
      maxMicrotasks: config.maxMicrotasks || 100,
      attentionBased: config.attentionBased !== false,
      monitoring: config.monitoring !== false,
    };
    
    this.initialized = false;
    this.isolates = new Map();
  }
  
  /**
   * Initialize the engine
   */
  initialize() {
    if (this.initialized) {
      throw new Error('Engine already initialized');
    }
    
    if (cognitiveBinding) {
      cognitiveBinding.createEngine(this.config);
    }
    
    this.initialized = true;
  }
  
  /**
   * Destroy the engine
   */
  destroy() {
    if (!this.initialized) {
      return;
    }
    
    // Destroy all isolates
    for (const id of this.isolates.keys()) {
      this.destroyIsolate(id);
    }
    
    if (cognitiveBinding) {
      cognitiveBinding.destroyEngine();
    }
    
    this.initialized = false;
  }
  
  /**
   * Create a new isolate
   */
  createIsolate(id, options = {}) {
    if (!this.initialized) {
      throw new Error('Engine not initialized');
    }
    
    if (this.isolates.has(id)) {
      throw new Error(`Isolate ${id} already exists`);
    }
    
    if (cognitiveBinding) {
      const success = cognitiveBinding.createIsolate(id);
      if (!success) {
        throw new Error(`Failed to create isolate ${id}`);
      }
    }
    
    const context = new IsolateContext(this, id, options);
    this.isolates.set(id, context);
    
    // Set initial STI/LTI if provided
    if (options.sti !== undefined) {
      context.setSTI(options.sti);
    }
    if (options.lti !== undefined) {
      context.setLTI(options.lti);
    }
    
    return context;
  }
  
  /**
   * Destroy an isolate
   */
  destroyIsolate(id) {
    if (!this.isolates.has(id)) {
      return;
    }
    
    if (cognitiveBinding) {
      cognitiveBinding.destroyIsolate(id);
    }
    
    this.isolates.delete(id);
  }
  
  /**
   * Get an isolate by ID
   */
  getIsolate(id) {
    return this.isolates.get(id);
  }
  
  /**
   * Get all isolates
   */
  getAllIsolates() {
    return Array.from(this.isolates.values());
  }
  
  /**
   * Get engine statistics
   */
  getStatistics() {
    const isolates = this.getAllIsolates();
    
    return {
      isolateCount: isolates.length,
      totalMemory: isolates.reduce((sum, iso) => sum + iso.getMemoryUsage(), 0),
      averageSTI: isolates.reduce((sum, iso) => sum + iso.getSTI(), 0) / isolates.length || 0,
      averageLTI: isolates.reduce((sum, iso) => sum + iso.getLTI(), 0) / isolates.length || 0,
    };
  }
  
  /**
   * Create a shared buffer for inter-isolate communication
   */
  createSharedBuffer(size) {
    if (cognitiveBinding && cognitiveBinding.createSharedBuffer) {
      return cognitiveBinding.createSharedBuffer(size);
    }
    // Fallback to regular SharedArrayBuffer
    return new SharedArrayBuffer(size);
  }
}

/**
 * Factory function to create a new engine
 */
function createEngine(config) {
  const engine = new CognitiveSynergyEngine(config);
  engine.initialize();
  return engine;
}

module.exports = {
  CognitiveSynergyEngine,
  IsolateContext,
  createEngine,
};
