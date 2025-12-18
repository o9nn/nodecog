#ifndef COGNITIVE_PLATFORM_H_
#define COGNITIVE_PLATFORM_H_

#include <memory>
#include <string>
#include "v8.h"
#include "uv.h"
#include "node.h"
#include "node_platform.h"

namespace cognitive {

/**
 * Configuration for cognitive platform initialization
 */
struct PlatformConfig {
  // Number of worker threads for libuv threadpool
  int worker_threads = 4;
  
  // Cognitive loop tick interval in milliseconds
  uint64_t cognitive_tick_ms = 5;
  
  // Enable verbose logging
  bool verbose = false;
};

/**
 * CognitivePlatform - Manages V8 platform and Node.js integration
 * 
 * This class handles the initialization and management of:
 * - V8 platform
 * - libuv event loop
 * - Node.js MultiIsolatePlatform
 * - Cognitive scheduling hooks
 */
class CognitivePlatform {
 public:
  explicit CognitivePlatform(const PlatformConfig& config);
  ~CognitivePlatform();
  
  // Initialize the platform
  bool Initialize();
  
  // Shutdown the platform
  void Shutdown();
  
  // Get the V8 platform
  v8::Platform* GetV8Platform();
  
  // Get the Node.js platform
  node::MultiIsolatePlatform* GetNodePlatform();
  
  // Get the libuv loop
  uv_loop_t* GetLoop();
  
  // Create a new V8 isolate with Node.js environment
  v8::Isolate* CreateIsolate();
  
  // Create Node.js environment for an isolate
  node::Environment* CreateEnvironment(
      v8::Isolate* isolate,
      v8::Local<v8::Context> context,
      const std::vector<std::string>& args,
      const std::vector<std::string>& exec_args);
  
  // Run the event loop (blocking)
  int Run();
  
  // Stop the event loop
  void Stop();
  
  // Install cognitive scheduler hooks
  void InstallCognitiveHooks(
      std::function<void()> on_prepare,
      std::function<void()> on_check,
      std::function<void()> on_tick,
      std::function<void()> on_idle);
  
 private:
  PlatformConfig config_;
  uv_loop_t* loop_;
  std::unique_ptr<node::MultiIsolatePlatform> platform_;
  std::unique_ptr<v8::ArrayBuffer::Allocator> allocator_;
  bool initialized_;
  bool running_;
  
  // Cognitive hooks
  uv_prepare_t prepare_handle_;
  uv_check_t check_handle_;
  uv_timer_t timer_handle_;
  uv_idle_t idle_handle_;
  
  std::function<void()> on_prepare_;
  std::function<void()> on_check_;
  std::function<void()> on_tick_;
  std::function<void()> on_idle_;
  
  // Static callbacks for libuv
  static void OnPrepareCallback(uv_prepare_t* handle);
  static void OnCheckCallback(uv_check_t* handle);
  static void OnTimerCallback(uv_timer_t* handle);
  static void OnIdleCallback(uv_idle_t* handle);
};

}  // namespace cognitive

#endif  // COGNITIVE_PLATFORM_H_
