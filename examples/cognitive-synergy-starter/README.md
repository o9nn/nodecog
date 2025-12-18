# Cognitive Synergy Engine - Starter Repository

This is a **ready-to-use starter template** for building applications with the Cognitive Synergy Engine, which places V8 and libuv under OpenCog's cognitive scheduler control.

## 🚀 Quick Start

```bash
# 1. Clone or copy this starter template
cd cognitive-synergy-starter

# 2. Build the project
mkdir build && cd build
cmake ..
make

# 3. Run the embedder example
./cognitive-embedder

# 4. Run the JavaScript example
node --experimental-loader ../lib/loader.mjs ../examples/basic.js
```

## 📋 What's Included

This starter provides everything you need:

```
cognitive-synergy-starter/
├── README.md                    # This file
├── CMakeLists.txt              # Build configuration
├── package.json                # Node.js dependencies
├── src/
│   ├── embedder.cc             # C++ embedder application
│   ├── cognitive_platform.h    # Platform initialization
│   └── cognitive_platform.cc   # Platform implementation
├── lib/
│   ├── cognitive_api.js        # JavaScript API wrapper
│   └── loader.mjs              # ESM loader hooks
├── examples/
│   ├── basic.js                # Basic usage example
│   ├── multi-isolate.js        # Multiple isolates example
│   ├── attention.js            # Attention-based scheduling
│   └── nodespace.js            # NodeSpace integration
└── docs/
    ├── ARCHITECTURE.md         # Architecture overview
    ├── API.md                  # API reference
    └── INTEGRATION.md          # Integration guide
```

## 🏗️ Architecture

The Cognitive Synergy Engine provides two integration paths:

### Path A: Node-on-V8 (Recommended)

Embed Node.js with full API support - this is what the starter template uses.

### Path B: Raw V8+libuv (Maximum Control)

For minimal runtime with full control (advanced users).

## 🧠 Cognitive Control via libuv

The engine hooks into the libuv event loop phases to give your cognitive scheduler control.

See [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) for details.

## 📚 Core Concepts

- **Isolates** = Security/Agent Domains
- **Contexts** = Goals/Plans/Episodes  
- **STI** (Short-Term Importance) = Execution priority
- **LTI** (Long-Term Importance) = Memory budget
- **NodeSpace** = Live software hypergraph

## 🛠️ Build Requirements

- **Node.js**: v18+ with embeddable API
- **CMake**: 3.10+
- **C++ Compiler**: C++17 support
- **Platform**: Linux, macOS, or Windows

## 📖 Documentation

- [Architecture Overview](docs/ARCHITECTURE.md)
- [API Reference](docs/API.md)
- [Integration Guide](docs/INTEGRATION.md)
- [Parent Documentation](../../doc/opencog/COGNITIVE_SYNERGY_ENGINE.md)

## 📝 License

MIT License - see Node.js LICENSE for details

---

**Ready to build?** Run `cmake` and start coding! 🚀
