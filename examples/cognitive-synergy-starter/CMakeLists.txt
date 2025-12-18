cmake_minimum_required(VERSION 3.10)
project(CognitiveSynergyStarter CXX)

# Set C++ standard (V8 requires C++20)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Platform detection
if(APPLE)
  set(PLATFORM_MACOS TRUE)
elseif(WIN32)
  set(PLATFORM_WINDOWS TRUE)
else()
  set(PLATFORM_LINUX TRUE)
endif()

# ============================================================================
# Node.js Detection
# ============================================================================

# Option 1: Use NODE_ROOT if provided
if(DEFINED NODE_ROOT)
  set(NODE_SRC_DIR "${NODE_ROOT}")
  message(STATUS "Using NODE_ROOT: ${NODE_ROOT}")
else()
  # Option 2: Assume we're in examples/cognitive-synergy-starter
  set(NODE_SRC_DIR "${CMAKE_SOURCE_DIR}/../..")
  message(STATUS "Using relative Node.js path: ${NODE_SRC_DIR}")
endif()

# Set Node.js paths
set(NODE_INCLUDE_DIR "${NODE_SRC_DIR}/src")
set(NODE_DEPS_DIR "${NODE_SRC_DIR}/deps")
set(V8_INCLUDE_DIR "${NODE_DEPS_DIR}/v8/include")
set(LIBUV_INCLUDE_DIR "${NODE_DEPS_DIR}/uv/include")

# Verify paths exist
if(NOT EXISTS "${NODE_INCLUDE_DIR}")
  message(FATAL_ERROR "Node.js source directory not found at: ${NODE_SRC_DIR}\n"
                      "Please set NODE_ROOT to your Node.js build directory:\n"
                      "  cmake -DNODE_ROOT=/path/to/node ..")
endif()

message(STATUS "Node.js configuration:")
message(STATUS "  Source: ${NODE_SRC_DIR}")
message(STATUS "  Include: ${NODE_INCLUDE_DIR}")
message(STATUS "  V8: ${V8_INCLUDE_DIR}")
message(STATUS "  libuv: ${LIBUV_INCLUDE_DIR}")

# ============================================================================
# Include Directories
# ============================================================================

include_directories(
  ${NODE_INCLUDE_DIR}
  ${V8_INCLUDE_DIR}
  ${LIBUV_INCLUDE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# ============================================================================
# Source Files
# ============================================================================

set(COGNITIVE_PLATFORM_SOURCES
  src/cognitive_platform.cc
)

set(EMBEDDER_SOURCES
  src/embedder.cc
  ${COGNITIVE_PLATFORM_SOURCES}
)

# Note: The full cognitive_synergy_engine.cc requires Node.js to be built
# For demonstration purposes, the embedder shows the architecture without
# actually linking against the full engine
# 
# To build with full functionality, first build Node.js:
#   cd ../../ && ./configure && make
# Then rebuild this project and uncomment the line below:
# set(COGNITIVE_ENGINE_SOURCES ${NODE_SRC_DIR}/src/cognitive_synergy_engine.cc)

# ============================================================================
# Executables
# ============================================================================

# Demo executable (builds without Node.js)
add_executable(cognitive-demo
  src/demo.cc
)

# Full embedder (requires Node.js to be built)
# Uncomment when Node.js is available
# add_executable(cognitive-embedder
#   ${EMBEDDER_SOURCES}
#   ${NODE_SRC_DIR}/src/cognitive_synergy_engine.cc
# )

# ============================================================================
# Link Libraries
# ============================================================================

# Find threads library
find_package(Threads REQUIRED)

# Link demo executable (minimal dependencies)
target_link_libraries(cognitive-demo
  Threads::Threads
)

# Link full embedder (requires Node.js libraries)
# When building with Node.js, uncomment the lines below:
# target_link_libraries(cognitive-embedder
#   Threads::Threads
#   ${CMAKE_DL_LIBS}
#   ${NODE_LIBRARY}
# )

# Platform-specific libraries for embedder
# if(PLATFORM_LINUX)
#   target_link_libraries(cognitive-embedder rt)
# endif()

# Link against Node.js library if found
# if(NODE_LIBRARY)
#   message(STATUS "Found Node.js library: ${NODE_LIBRARY}")
#   target_link_libraries(cognitive-embedder ${NODE_LIBRARY})
# else()
#   message(WARNING "Node.js library not found. You may need to build Node.js first.")
#   message(WARNING "Build Node.js with: ./configure && make")
# endif()

# ============================================================================
# Compile Options
# ============================================================================

# Enable warnings for demo
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(cognitive-demo PRIVATE
    -Wall
    -Wextra
    -Wno-unused-parameter
    -Wno-missing-field-initializers
  )
endif()

# Debug/Release flags for demo
if(CMAKE_BUILD_TYPE MATCHES Debug)
  target_compile_definitions(cognitive-demo PRIVATE DEBUG=1)
  message(STATUS "Building in Debug mode")
else()
  target_compile_definitions(cognitive-demo PRIVATE NDEBUG=1)
  message(STATUS "Building in Release mode")
endif()

# ============================================================================
# Installation
# ============================================================================

install(TARGETS cognitive-demo DESTINATION bin)

# Install library files
install(DIRECTORY lib/ DESTINATION lib)

# Install examples
install(DIRECTORY examples/ DESTINATION examples)

# Install documentation
install(DIRECTORY docs/ DESTINATION docs)
install(FILES README.md DESTINATION .)

# ============================================================================
# Testing (Optional)
# ============================================================================

enable_testing()

# Add a simple smoke test
add_test(NAME demo_runs
  COMMAND cognitive-demo
)

# ============================================================================
# Print Configuration Summary
# ============================================================================

message(STATUS "")
message(STATUS "=======================================================")
message(STATUS "Cognitive Synergy Engine - Build Configuration")
message(STATUS "=======================================================")
message(STATUS "  CMake version:      ${CMAKE_VERSION}")
message(STATUS "  C++ compiler:       ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "  Build type:         ${CMAKE_BUILD_TYPE}")
message(STATUS "  Platform:           ${CMAKE_SYSTEM_NAME}")
message(STATUS "  Node.js source:     ${NODE_SRC_DIR}")
message(STATUS "  Node.js library:    ${NODE_LIBRARY}")
message(STATUS "  Install prefix:     ${CMAKE_INSTALL_PREFIX}")
message(STATUS "=======================================================")
message(STATUS "")
