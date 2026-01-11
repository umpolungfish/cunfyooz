# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

**cunfyooz** is a metamorphic binary obfuscation engine that transforms PE binaries into functionally equivalent but structurally unique variants. The project consists of:

1. **Core C Engine** - Native binary transformation engine using Capstone (disassembly) and Keystone (assembly)
2. **Python AI Agent Framework** - Claude-powered agents for intelligent binary analysis and automated obfuscation workflows
3. **Unified CLI** - Command-line interface providing access to both the core engine and AI agents

## Build and Development Commands

### Core C Engine

```bash
# Build the core cunfyooz binary
make

# Clean build artifacts
make clean

# Install CLI wrapper (makes cunfyooz_cli.py executable)
make install
```

The compiled binary will be in `bin/cunfyooz`.

### Python Environment Setup

```bash
# Set up Python virtual environment
python3 -m venv venv
source venv/bin/activate  # On Windows: venv\Scripts\activate

# Install dependencies
pip install -r requirements.txt

# Set Claude API key for AI agents
export ANTHROPIC_API_KEY="your-api-key"
```

### Running Transformations

```bash
# Direct C binary usage
./bin/cunfyooz target.exe

# Python CLI - core transformation
python cunfyooz_cli.py transform target.exe

# Python CLI - AI-powered analysis
python cunfyooz_cli.py analyze target.exe

# Python CLI - complete pipeline
python cunfyooz_cli.py pipeline target.exe
```

### Testing

The `tests/` directory contains test programs:

```bash
# Compile and run test binaries
gcc tests/test.c -o tests/test
gcc tests/verify_pe.c -o tests/verify_pe
gcc tests/check_pe.c -o tests/check_pe -lcapstone

# Run verification
./tests/verify_pe target.exe
```

## Architecture

### Dual-Layer Design

The project has two distinct but integrated layers:

**Layer 1: Core Transformation Engine (C)**
- PE binary parsing (`pe_parser.c`)
- Disassembly via Capstone (`disassembler.c`)
- Metamorphic transformations (`transformer.c`)
- Reassembly via Keystone (`assembler.c`)
- Optional virtualization (`virtualization_engine.c`)
- JSON configuration parsing (`json_parser.c`)

**Layer 2: AI Agent Framework (Python)**
- Agent orchestration (`framework/orchestrator.py`)
- Binary analysis tools (`framework/binary_tools.py`)
- Inter-agent communication (`framework/communication.py`)
- Persistent memory (`framework/memory.py`)
- Specialized agents (`agents/cunfyooz_agent.py`)

### Transformation Pipeline

The C engine follows a 10-stage pipeline:

1. **Configuration Loading** - Parse `config.json` or use defaults
2. **PE Parsing** - Extract DOS/NT headers and section table
3. **Disassembly** - Use Capstone to disassemble `.text` section
4. **Control Flow Analysis** - Build CFG, identify basic blocks
5. **Data Flow Analysis** - Track register dependencies and def-use chains
6. **NOP Insertion** - Insert basic and enhanced NOPs at safe locations
7. **Instruction Substitution** - Replace instructions with functional equivalents
8. **Register Shuffling** - Rename registers while preserving dependencies
9. **Control Flow Obfuscation** - Insert opaque predicates and dead code
10. **Reconstruction** - Reassemble using Keystone and rebuild PE binary

### Key Architectural Constraints

**Critical Register Preservation**: The transformer NEVER modifies `RSP` or `RBP` to maintain stack frame integrity.

**Dependency Tracking**: All transformations honor data flow dependencies. Register renaming and instruction reordering use def-use chain analysis to prevent corruption.

**Safe Transformation Points**: Transformations avoid insertion after branches, calls, or instructions with specific jump targets unless properly updating relocation tables.

**Configuration-Driven**: All transformation probabilities and enabled/disabled states are controlled via `config.json`.

## Configuration System

The engine reads `config.json` from the working directory. Key configuration sections:

**Transformations**: Each transformation has `enabled` (bool) and `probability` (1-100) fields:
- `nop_insertion` - Basic 0x90 NOP insertion
- `enhanced_nop_insertion` - Multi-byte NOPs (XCHG, LEA, TEST)
- `instruction_substitution` - LEA↔MOV, TEST↔CMP replacements
- `register_shuffling` - Register renaming with dependency preservation
- `control_flow_obfuscation` - Opaque predicates and dead code
- `stack_frame_obfuscation` - Stack manipulation sequences
- `instruction_reordering` - Dependency-aware instruction scheduling
- `anti_analysis_techniques` - Anti-debugging and analysis resistance
- `virtualization_engine` - Code-to-bytecode transformation (typically disabled)

**Security Settings**:
- `validate_functionality` - Set to `false` by default to prevent execution of potentially malicious binaries
- `preserve_original_behavior` - Ensures semantic equivalence

## AI Agent Framework

### Agent Types

**CunfyoozAgent** (`agents/cunfyooz_agent.py`): Performs binary obfuscation using the core engine. Executes the `bin/cunfyooz` binary and manages transformation workflows.

**CunfyoozAnalysisAgent** (`agents/cunfyooz_agent.py`): Analyzes transformed binaries for effectiveness, entropy changes, and structural differences.

**ResearchAgent / AnalysisAgent** (`agents/example_agent.py`): Example agents demonstrating the framework capabilities.

### Agent Execution Modes

**Single Agent**: Run one agent on a single task
```python
from agents.cunfyooz_agent import CunfyoozAgent
agent = CunfyoozAgent(config)
result = agent.process(task, context)
```

**Swarm Mode**: Run multiple agents in parallel
```python
orchestrator = AgentOrchestrator()
result = orchestrator.run_swarm(task, agent_ids)
```

**Pipeline Mode**: Sequential execution with context passing
```python
result = orchestrator.run_pipeline(task, agent_ids, initial_context)
```

### Tool System

Agents use tools defined in `framework/tools.py` and `framework/binary_tools.py`:
- File operations (read, write, analyze)
- Binary transformations (via wrapper to C engine)
- Binary comparison and entropy calculation
- Shell command execution
- Cross-agent communication

## Code Layout

```
cunfyooz/
├── src/               # Core C engine sources
│   ├── main.c         # Entry point, config loading, validation
│   ├── pe_parser.c    # PE file parsing
│   ├── disassembler.c # Capstone wrapper
│   ├── transformer.c  # All transformation logic
│   ├── assembler.c    # Keystone wrapper
│   └── virtualization_engine.c
├── include/           # C headers
├── framework/         # Python AI agent framework
│   ├── base_agent.py
│   ├── orchestrator.py
│   ├── tools.py
│   ├── binary_tools.py
│   ├── communication.py
│   └── memory.py
├── agents/            # Specialized agent implementations
│   ├── cunfyooz_agent.py
│   └── example_agent.py
├── examples/          # Usage examples for agents
├── tests/             # Test binaries and verification tools
├── bin/               # Compiled binaries (created by make)
├── build/             # Build artifacts (created by make)
├── artifacts/         # Generated transformed binaries
├── cunfyooz_cli.py    # Unified CLI
├── cunfyooz_wrapper.py # Python wrapper for C engine
├── config.json        # Transformation configuration
└── Makefile
```

## Important Implementation Notes

### Security Considerations

**Binary Execution is Disabled**: The `validate_transformation()` function in `main.c` is intentionally disabled to prevent automatic execution of potentially malicious binaries. This is a security feature.

**Metamorphic Output May Trigger AV**: Transformed binaries may be flagged by antivirus software due to:
- Unusual instruction patterns
- Anti-debugging techniques
- Code structure changes
Always test in isolated environments.

### Transformation Safety

**Never Break Control Flow**: When inserting or modifying instructions, the engine updates all jump targets and maintains control flow graph integrity.

**Preserve Semantics**: All transformations maintain functional equivalence. The engine verifies that:
- All execution paths are preserved
- Register states remain consistent
- Memory operations are unaffected
- Function call conventions are maintained

### Random Number Generation

Each run uses time-based seeding: `srand((unsigned int)time(NULL) + clock())` in `main.c:32`. This ensures truly unique transformations on each execution.

### Working with Large Binaries

The engine processes the entire `.text` section in memory. For very large binaries:
- Monitor memory usage
- Consider disabling expensive transformations (virtualization, extensive control flow obfuscation)
- Adjust probabilities to reduce transformation density

## Common Workflows

### Adding a New Transformation

1. Define the transformation logic in `src/transformer.c`
2. Add configuration fields to the `config_t` struct in `include/json_parser.h`
3. Update `parse_json_config()` in `src/json_parser.c` to read the new config
4. Add the transformation to the pipeline in `src/main.c`
5. Update `config.json` with the new transformation settings

### Creating a New Agent

1. Create agent file in `agents/`
2. Inherit from `BaseAgent` in `framework/base_agent.py`
3. Implement required methods: `get_tools()`, `process()`
4. Register with `AgentOrchestrator` in the CLI or custom scripts
5. Add example usage to `examples/`

### Analyzing Transformation Effectiveness

Use the Python wrapper's comparison functionality:

```python
from cunfyooz_wrapper import CunfyoozWrapper

wrapper = CunfyoozWrapper()
comparison = wrapper.compare_binaries(original, transformed)
print(f"Entropy change: {comparison['original']['entropy']} -> {comparison['transformed']['entropy']}")
print(f"Size change: {comparison['size_change_percent']}%")
```

## External Dependencies

**C Engine**:
- **Capstone** (disassembly) - Install from https://github.com/aquynh/capstone
- **Keystone** (assembly) - Install from https://github.com/keystone-engine/keystone
- GCC compiler
- GNU Make

**Python Framework**:
- **anthropic** >= 0.21.0 - Claude API client
- Python 3.10+

## Output Artifacts

Transformed binaries are saved with predictable naming:
- Direct C engine: `cunfyoozed_<original_name>.exe`
- Python CLI: Configurable via `-o` flag or default naming
- AI agents: Timestamped artifacts in `artifacts/cunfyooz_agent/`

All output paths are relative to the current working directory unless absolute paths are specified.
