# NodeCog Agent Documentation

This directory contains comprehensive documentation for the NodeCog custom agent.

## File Structure

```
.github/agents/
├── nodecog.md                    # Main agent definition (14.7k)
└── nodecog/
    ├── README.md                 # This file
    ├── api-reference.md          # Complete API documentation (16.4k)
    ├── architecture.md           # System architecture details (20.9k)
    ├── cognitive-features.md     # Cognitive AI capabilities (19.3k)
    ├── development-roadmap.md    # Implementation timeline (13.7k)
    └── kernel-layer.md           # Echo.Kern specifications (22.1k)
```

**Total Documentation**: 107k characters across 6 files  
**All files under 30k limit**: ✅ Agent constraint compliant

## Documentation Overview

### Main Agent (nodecog.md)

The main agent definition provides:
- Core expertise areas and system understanding
- Quick reference for all NodeCog components
- Best practices and troubleshooting guidance
- File locations and codebase organization
- Integration with subcomponent documentation

**Use this for**: Quick lookup, understanding agent capabilities, getting oriented

### Subcomponent Files

#### architecture.md
**Detailed system architecture and component specifications**

Contains:
- Core component details (AtomSpace, NodeSpace, AttentionBank, etc.)
- Integration points and data flow
- Module loader hooks
- Performance characteristics
- Configuration options
- Extension points for custom development

**Use this for**: Understanding system internals, integration work, custom components

#### cognitive-features.md
**Cognitive AI capabilities and algorithms**

Contains:
- Hypergraph knowledge representation
- Economic attention allocation (ECAN)
- Autonomous agent system
- Pattern mining algorithms
- Inference and reasoning
- Distributed cognition
- MOSES meta-optimization
- Planning and NLP integration

**Use this for**: Understanding AI features, implementing cognitive agents, reasoning tasks

#### kernel-layer.md
**Echo.Kern high-performance tensor-based kernel specifications**

Contains:
- 35 kernel function specifications
- GGML integration details
- Bootstrap stages (0-3)
- Performance targets (10-1000x improvements)
- N-API bridge architecture
- Build system integration
- Implementation timeline

**Use this for**: Understanding planned kernel, performance optimization, C++ integration

#### development-roadmap.md
**13-phase implementation plan summary**

Contains:
- Phase breakdown (0-12)
- Implementation status and priorities
- Glossary term coverage (78/78)
- Resource requirements
- Success metrics
- Risk management
- Next immediate actions

**Use this for**: Planning, understanding project status, contributing to development

#### api-reference.md
**Complete API documentation with examples**

Contains:
- AtomSpace API
- NodeSpace API
- Attention Bank API
- Agent API
- Orchestrator API
- Cognitive Loop API
- Distributed AtomSpace API
- Runtime API (process.opencog)
- Configuration options
- Event system
- Usage examples

**Use this for**: Development, API usage, code examples, configuration

## Quick Start

### For Users

1. Start with **nodecog.md** to understand what the agent knows
2. Reference **api-reference.md** for API usage
3. Check **architecture.md** for system details
4. Use **cognitive-features.md** to understand AI capabilities

### For Contributors

1. Review **development-roadmap.md** for current status
2. Read **architecture.md** for system design
3. Check **kernel-layer.md** for planned improvements
4. Reference **api-reference.md** for existing APIs

### For Researchers

1. Study **cognitive-features.md** for AI algorithms
2. Review **architecture.md** for implementation details
3. Check **kernel-layer.md** for performance targets
4. See **development-roadmap.md** for future plans

## Agent Capabilities

The NodeCog agent can help with:

### Knowledge & Understanding
- Explain cognitive architecture concepts
- Detail OpenCog integration
- Describe attention allocation mechanisms
- Clarify hypergraph representation

### Development
- Provide API usage examples
- Guide custom agent development
- Explain module tracking
- Show configuration options

### Troubleshooting
- Diagnose module tracking issues
- Debug attention spreading
- Identify circular dependencies
- Resolve performance problems

### Planning
- Explain implementation roadmap
- Describe future kernel features
- Detail performance targets
- Outline integration strategies

## Character Limits

All files comply with the 30k character agent constraint:

| File | Size | Limit | Status |
|------|------|-------|--------|
| nodecog.md | 14.7k | 30k | ✅ 49% |
| api-reference.md | 16.4k | 30k | ✅ 55% |
| architecture.md | 20.9k | 30k | ✅ 70% |
| cognitive-features.md | 19.3k | 30k | ✅ 64% |
| development-roadmap.md | 13.7k | 30k | ✅ 46% |
| kernel-layer.md | 22.1k | 30k | ✅ 74% |

## Updates

This documentation is comprehensive and stable. Updates should focus on:

1. **New features**: Add to relevant subcomponent file
2. **API changes**: Update api-reference.md
3. **Architecture changes**: Update architecture.md
4. **Roadmap progress**: Update development-roadmap.md

## Contributing

To update agent documentation:

1. Keep files under 30k characters
2. Maintain consistent formatting
3. Update all cross-references
4. Test with example queries
5. Validate markdown syntax

## References

### Repository Documentation
- `/DEVELOPMENT_ROADMAP.md` - Full 13-phase plan
- `/OPENCOG_SUMMARY.md` - OpenCog implementation
- `/KERNEL_FUNCTION_MANIFEST.md` - All 35 kernel functions
- `/doc/ARCHITECTURE_INDEX.md` - Architecture hub
- `/doc/TECHNICAL_ARCHITECTURE.md` - System diagrams

### External Resources
- OpenCog: https://opencog.org/
- GGML: https://github.com/ggerganov/ggml
- Node.js: https://nodejs.org/
