#include "cognitive_platform.h"
#include <iostream>

namespace cognitive {

CognitivePlatform::CognitivePlatform(const PlatformConfig& config)
    : config_(config),
      loop_(nullptr),
      initialized_(false),
      running_(false) {
}

CognitivePlatform::~CognitivePlatform() {
  Shutdown();
}

bool CognitivePlatform::Initialize() {
  if (initialized_) {
    if (config_.verbose) {
      std::cout << "Platform already initialized\n";
    }
    return true;
  }
  
  if (config_.verbose) {
    std::cout << "Initializing Cognitive Platform...\n";
    std::cout << "  Worker threads: " << config_.worker_threads << "\n";
    std::cout << "  Cognitive tick: " << config_.cognitive_tick_ms << "ms\n";
  }
  
  // Create libuv loop
  loop_ = uv_default_loop();
  if (!loop_) {
    std::cerr << "Failed to create libuv loop\n";
    return false;
  }
  
  // Create array buffer allocator
  allocator_ = std::unique_ptr<v8::ArrayBuffer::Allocator>(
      v8::ArrayBuffer::Allocator::NewDefaultAllocator());
  
  // Create Node.js MultiIsolatePlatform
  platform_ = node::MultiIsolatePlatform::Create(config_.worker_threads);
  if (!platform_) {
    std::cerr << "Failed to create Node.js platform\n";
    return false;
  }
  
  // Initialize V8
  v8::V8::InitializePlatform(platform_.get());
  v8::V8::Initialize();
  
  if (config_.verbose) {
    std::cout << "✓ Platform initialized successfully\n";
  }
  
  initialized_ = true;
  return true;
}

void CognitivePlatform::Shutdown() {
  if (!initialized_) {
    return;
  }
  
  if (config_.verbose) {
    std::cout << "Shutting down Cognitive Platform...\n";
  }
  
  // Stop event loop if running
  Stop();
  
  // Dispose V8
  v8::V8::Dispose();
  v8::V8::DisposePlatform();
  
  // Clean up platform
  platform_.reset();
  allocator_.reset();
  
  initialized_ = false;
  
  if (config_.verbose) {
    std::cout << "✓ Platform shut down\n";
  }
}

v8::Platform* CognitivePlatform::GetV8Platform() {
  return platform_.get();
}

node::MultiIsolatePlatform* CognitivePlatform::GetNodePlatform() {
  return platform_.get();
}

uv_loop_t* CognitivePlatform::GetLoop() {
  return loop_;
}

v8::Isolate* CognitivePlatform::CreateIsolate() {
  v8::Isolate::CreateParams params;
  params.array_buffer_allocator = allocator_.get();
  return v8::Isolate::New(params);
}

node::Environment* CognitivePlatform::CreateEnvironment(
    v8::Isolate* isolate,
    v8::Local<v8::Context> context,
    const std::vector<std::string>& args,
    const std::vector<std::string>& exec_args) {
  
  node::IsolateData* isolate_data = 
      node::CreateIsolateData(isolate, loop_, platform_.get());
  
  return node::CreateEnvironment(
      isolate_data,
      context,
      args,
      exec_args);
}

void CognitivePlatform::InstallCognitiveHooks(
    std::function<void()> on_prepare,
    std::function<void()> on_check,
    std::function<void()> on_tick,
    std::function<void()> on_idle) {
  
  on_prepare_ = on_prepare;
  on_check_ = on_check;
  on_tick_ = on_tick;
  on_idle_ = on_idle;
  
  // Initialize uv_prepare (runs before I/O polling)
  if (on_prepare_) {
    uv_prepare_init(loop_, &prepare_handle_);
    prepare_handle_.data = this;
    uv_prepare_start(&prepare_handle_, OnPrepareCallback);
  }
  
  // Initialize uv_check (runs after I/O polling)
  if (on_check_) {
    uv_check_init(loop_, &check_handle_);
    check_handle_.data = this;
    uv_check_start(&check_handle_, OnCheckCallback);
  }
  
  // Initialize uv_timer (cognitive loop tick)
  if (on_tick_) {
    uv_timer_init(loop_, &timer_handle_);
    timer_handle_.data = this;
    uv_timer_start(&timer_handle_, OnTimerCallback, 0, config_.cognitive_tick_ms);
  }
  
  // Initialize uv_idle (background tasks)
  if (on_idle_) {
    uv_idle_init(loop_, &idle_handle_);
    idle_handle_.data = this;
    uv_idle_start(&idle_handle_, OnIdleCallback);
  }
}

int CognitivePlatform::Run() {
  if (!initialized_) {
    std::cerr << "Platform not initialized\n";
    return -1;
  }
  
  running_ = true;
  
  if (config_.verbose) {
    std::cout << "Starting event loop...\n";
  }
  
  // Run the event loop
  int result = uv_run(loop_, UV_RUN_DEFAULT);
  
  running_ = false;
  
  if (config_.verbose) {
    std::cout << "Event loop stopped\n";
  }
  
  return result;
}

void CognitivePlatform::Stop() {
  if (!running_) {
    return;
  }
  
  // Stop all handles
  if (on_prepare_) {
    uv_prepare_stop(&prepare_handle_);
  }
  if (on_check_) {
    uv_check_stop(&check_handle_);
  }
  if (on_tick_) {
    uv_timer_stop(&timer_handle_);
  }
  if (on_idle_) {
    uv_idle_stop(&idle_handle_);
  }
  
  // Stop the loop
  uv_stop(loop_);
}

// Static callback implementations
void CognitivePlatform::OnPrepareCallback(uv_prepare_t* handle) {
  auto* platform = static_cast<CognitivePlatform*>(handle->data);
  if (platform && platform->on_prepare_) {
    platform->on_prepare_();
  }
}

void CognitivePlatform::OnCheckCallback(uv_check_t* handle) {
  auto* platform = static_cast<CognitivePlatform*>(handle->data);
  if (platform && platform->on_check_) {
    platform->on_check_();
  }
}

void CognitivePlatform::OnTimerCallback(uv_timer_t* handle) {
  auto* platform = static_cast<CognitivePlatform*>(handle->data);
  if (platform && platform->on_tick_) {
    platform->on_tick_();
  }
}

void CognitivePlatform::OnIdleCallback(uv_idle_t* handle) {
  auto* platform = static_cast<CognitivePlatform*>(handle->data);
  if (platform && platform->on_idle_) {
    platform->on_idle_();
  }
}

}  // namespace cognitive
