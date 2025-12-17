/**
 * Cognitive Synergy Engine - Minimal Embedder
 * 
 * This demonstrates the two paths to embedding V8+libuv under cognitive control:
 * 
 * Path A (Recommended): Node-on-V8 - Full Node.js API with cognitive control
 * Path B (Advanced): Raw V8+libuv - Maximum control, minimal runtime
 * 
 * This example implements Path A.
 */

#include "cognitive_platform.h"
#include "../../src/cognitive_synergy_engine.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>

using namespace cognitive;
using namespace node::cognitive;

// Global state for cognitive scheduler
static std::atomic<int> tick_count{0};
static CognitiveSynergyEngine* g_engine = nullptr;

/**
 * uv_prepare callback - Runs before I/O polling
 * This is where we select the next isolate based on attention (STI)
 */
void OnCognitiveSchedule() {
  if (!g_engine || !g_engine->scheduler()) {
    return;
  }
  
  // Select next isolate based on STI/LTI
  IsolateContext* selected = g_engine->scheduler()->SelectNextIsolate();
  
  if (selected) {
    // Schedule tasks for this isolate
    // In a real implementation, you'd queue work on the isolate's
    // foreground task runner here
    selected->ExecuteTasks(100);  // Max 100 microtasks per slice
  }
}

/**
 * uv_check callback - Runs after I/O polling
 * This is where we perform microtask checkpoints for deterministic Promises
 */
void OnMicrotaskCheckpoint() {
  if (!g_engine || !g_engine->scheduler()) {
    return;
  }
  
  // Perform microtask checkpoint for running isolates
  // This ensures Promises settle deterministically under our control
  IsolateContext* current = g_engine->scheduler()->SelectNextIsolate();
  if (current) {
    current->PerformMicrotaskCheckpoint();
  }
}

/**
 * uv_timer callback - Cognitive loop tick
 * This is where attention decay and resource updates happen
 */
void OnCognitiveLoopTick() {
  tick_count++;
  
  if (!g_engine || !g_engine->scheduler()) {
    return;
  }
  
  // Decay attention over time (1% per tick)
  g_engine->scheduler()->DecayAttention();
  
  // Update attention based on resource usage
  g_engine->scheduler()->UpdateAttention();
  
  // Every 100 ticks, print statistics
  if (tick_count % 100 == 0 && g_engine->scheduler()->GetIsolateCount() > 0) {
    std::cout << "\n[Cognitive Tick " << tick_count << "]\n";
    std::cout << "  Active isolates: " << g_engine->scheduler()->GetIsolateCount() << "\n";
  }
}

/**
 * uv_idle callback - Background maintenance
 * This is where we flush NodeSpace deltas and run low-priority tasks
 */
void OnBackgroundMaintenance() {
  // In a real implementation, you would:
  // - Flush NodeSpace graph deltas to AtomSpace
  // - Run low-priority data miners
  // - Perform garbage collection hints
  // - Update persistent state
}

/**
 * Main entry point
 */
int main(int argc, char** argv) {
  std::cout << "========================================\n";
  std::cout << "Cognitive Synergy Engine - Embedder\n";
  std::cout << "Path A: Node-on-V8 Integration\n";
  std::cout << "========================================\n\n";
  
  // =========================================================================
  // Step 1: Initialize Cognitive Platform
  // =========================================================================
  
  PlatformConfig platform_config;
  platform_config.worker_threads = 4;
  platform_config.cognitive_tick_ms = 5;
  platform_config.verbose = true;
  
  CognitivePlatform platform(platform_config);
  
  if (!platform.Initialize()) {
    std::cerr << "Failed to initialize platform\n";
    return 1;
  }
  
  std::cout << "\n";
  
  // =========================================================================
  // Step 2: Create Cognitive Synergy Engine
  // =========================================================================
  
  std::cout << "Creating Cognitive Synergy Engine...\n";
  
  CognitiveSynergyConfig engine_config;
  engine_config.cognitive_tick_ms = 5;
  engine_config.worker_threads = 4;
  engine_config.max_microtasks_per_slice = 100;
  engine_config.attention_based_scheduling = true;
  engine_config.enable_monitoring = true;
  
  CognitiveSynergyEngine engine(engine_config);
  g_engine = &engine;
  
  if (!engine.Initialize()) {
    std::cerr << "Failed to initialize engine\n";
    return 1;
  }
  
  std::cout << "✓ Engine initialized\n\n";
  
  // =========================================================================
  // Step 3: Install Cognitive Hooks into libuv Loop
  // =========================================================================
  
  std::cout << "Installing cognitive hooks...\n";
  std::cout << "  ✓ uv_prepare: Attention-based isolate selection\n";
  std::cout << "  ✓ uv_check: Microtask checkpoints\n";
  std::cout << "  ✓ uv_timer: Cognitive loop tick (every 5ms)\n";
  std::cout << "  ✓ uv_idle: Background maintenance\n\n";
  
  platform.InstallCognitiveHooks(
    OnCognitiveSchedule,       // uv_prepare
    OnMicrotaskCheckpoint,     // uv_check
    OnCognitiveLoopTick,       // uv_timer
    OnBackgroundMaintenance    // uv_idle
  );
  
  // =========================================================================
  // Step 4: Create Isolates for Different Cognitive Functions
  // =========================================================================
  
  std::cout << "Creating cognitive isolates...\n";
  
  // Reasoning isolate - High priority, high memory
  IsolateContext* reasoning = engine.CreateIsolate("reasoning");
  if (!reasoning) {
    std::cerr << "Failed to create reasoning isolate\n";
    return 1;
  }
  reasoning->SetSTI(100.0);  // Highest priority
  reasoning->SetLTI(90.0);   // High memory budget
  std::cout << "  ✓ Reasoning (STI: 100, LTI: 90)\n";
  
  // Perception isolate - Medium-high priority
  IsolateContext* perception = engine.CreateIsolate("perception");
  if (!perception) {
    std::cerr << "Failed to create perception isolate\n";
    return 1;
  }
  perception->SetSTI(80.0);
  perception->SetLTI(70.0);
  std::cout << "  ✓ Perception (STI: 80, LTI: 70)\n";
  
  // Planning isolate - Medium priority
  IsolateContext* planning = engine.CreateIsolate("planning");
  if (!planning) {
    std::cerr << "Failed to create planning isolate\n";
    return 1;
  }
  planning->SetSTI(60.0);
  planning->SetLTI(80.0);
  std::cout << "  ✓ Planning (STI: 60, LTI: 80)\n";
  
  // Background isolate - Low priority
  IsolateContext* background = engine.CreateIsolate("background");
  if (!background) {
    std::cerr << "Failed to create background isolate\n";
    return 1;
  }
  background->SetSTI(20.0);
  background->SetLTI(50.0);
  std::cout << "  ✓ Background (STI: 20, LTI: 50)\n";
  
  std::cout << "\n";
  
  // =========================================================================
  // Step 5: Display Initial Statistics
  // =========================================================================
  
  std::cout << "Initial statistics:\n";
  std::cout << "  Total isolates: " << engine.scheduler()->GetIsolateCount() << "\n";
  std::cout << "  Memory usage:\n";
  std::cout << "    - Reasoning:   " << reasoning->GetMemoryUsage() / 1024 << " KB\n";
  std::cout << "    - Perception:  " << perception->GetMemoryUsage() / 1024 << " KB\n";
  std::cout << "    - Planning:    " << planning->GetMemoryUsage() / 1024 << " KB\n";
  std::cout << "    - Background:  " << background->GetMemoryUsage() / 1024 << " KB\n";
  std::cout << "\n";
  
  // =========================================================================
  // Step 6: Run Cognitive Loop
  // =========================================================================
  
  std::cout << "========================================\n";
  std::cout << "Starting Cognitive Loop\n";
  std::cout << "========================================\n\n";
  
  std::cout << "The cognitive scheduler is now controlling execution:\n";
  std::cout << "  • Isolates scheduled by STI (attention)\n";
  std::cout << "  • Automatic attention decay\n";
  std::cout << "  • Resource-aware scheduling\n";
  std::cout << "  • Deterministic Promise settlement\n\n";
  
  std::cout << "Running for 500ms...\n\n";
  
  // Run in background thread
  std::thread event_loop_thread([&engine]() {
    engine.Run();
  });
  
  // Simulate work and attention changes
  std::this_thread::sleep_for(std::chrono::milliseconds(250));
  
  std::cout << "\n[Simulating attention boost]\n";
  reasoning->SetSTI(reasoning->GetSTI() + 30);
  std::cout << "  ✓ Boosted reasoning STI to: " << reasoning->GetSTI() << "\n";
  
  std::this_thread::sleep_for(std::chrono::milliseconds(250));
  
  // =========================================================================
  // Step 7: Stop and Display Final Statistics
  // =========================================================================
  
  std::cout << "\n\nStopping cognitive loop...\n";
  engine.Stop();
  
  if (event_loop_thread.joinable()) {
    event_loop_thread.join();
  }
  
  std::cout << "\n========================================\n";
  std::cout << "Final Statistics\n";
  std::cout << "========================================\n\n";
  
  std::cout << "Cognitive ticks: " << tick_count << "\n";
  std::cout << "Isolate attention (STI):\n";
  std::cout << "  - Reasoning:   " << reasoning->GetSTI() << "\n";
  std::cout << "  - Perception:  " << perception->GetSTI() << "\n";
  std::cout << "  - Planning:    " << planning->GetSTI() << "\n";
  std::cout << "  - Background:  " << background->GetSTI() << "\n";
  std::cout << "\nMemory usage:\n";
  std::cout << "  - Reasoning:   " << reasoning->GetMemoryUsage() / 1024 << " KB\n";
  std::cout << "  - Perception:  " << perception->GetMemoryUsage() / 1024 << " KB\n";
  std::cout << "  - Planning:    " << planning->GetMemoryUsage() / 1024 << " KB\n";
  std::cout << "  - Background:  " << background->GetMemoryUsage() / 1024 << " KB\n";
  
  // =========================================================================
  // Step 8: Cleanup
  // =========================================================================
  
  std::cout << "\nCleaning up...\n";
  engine.DestroyIsolate("reasoning");
  engine.DestroyIsolate("perception");
  engine.DestroyIsolate("planning");
  engine.DestroyIsolate("background");
  
  platform.Shutdown();
  
  std::cout << "\n========================================\n";
  std::cout << "Cognitive Synergy Engine - Complete\n";
  std::cout << "========================================\n";
  
  return 0;
}
