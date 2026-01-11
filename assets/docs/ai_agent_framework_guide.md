# Cunfyooz AI Agent Framework: Comprehensive Guide

## Table of Contents
- [Overview](#overview)
- [Architecture](#architecture)
- [Installation](#installation)
- [Quick Start](#quick-start)
- [Agent Components](#agent-components)
- [Usage Examples](#usage-examples)
- [Advanced Usage](#advanced-usage)
- [Configuration](#configuration)
- [Troubleshooting](#troubleshooting)
- [API Reference](#api-reference)

## Overview

The Cunfyooz AI Agent Framework combines the power of the Claude AI API with the cunfyooz metamorphic binary obfuscation engine. This integration enables intelligent, automated binary analysis and transformation through AI-powered agents that can make decisions about optimal obfuscation strategies, validate transformations, and provide detailed analysis reports.

### Key Features
- **AI-Powered Decision Making**: Agents intelligently determine optimal obfuscation strategies based on binary analysis
- **Multi-Agent Coordination**: Support for single agents, swarms, and sequential pipelines
- **Binary Analysis**: Comprehensive analysis of binary properties, entropy, and structural characteristics
- **Transformation Validation**: Automatic validation to ensure transformed binaries maintain original functionality (disabled by default for security - enables execution of binaries for comparison)
- **Extensible Architecture**: Easy to add new agent types and transformation techniques

## Architecture

### Core Components

```
┌─────────────────────────────────────────────────────────────┐
│                    Agent Orchestrator                       │
│  ┌──────────────┐  ┌──────────────┐   ┌──────────────┐      │
│  │ Cunfyooz     │  │ Analysis     │   │ Custom       │      │
│  │  Agent       │  │  Agent       │   │  Agent       │      │
│  └──────┬───────┘  └──────┬───────┘   └──────┬───────┘      │
│         │                 │                  │              │
│         └─────────────────┴──────────────────┘              │
│                           │                                 │
└───────────────────────────┼─────────────────────────────────┘
                            │
                ┌───────────┴──────────┐
                │                      │
        ┌───────▼──────┐      ┌────────▼────────┐
        │ Claude API   │      │ Binary Tools    │
        │ Integration  │      │ (analysis,      │
        │              │      │  transformation)│
        └──────────────┘      └─────────────────┘
                │                      │
        ┌───────▼──────────────────────▼───────┐
        │     Memory & Communication           │
        │  (State, Messages, Collaboration)    │
        └──────────────────────────────────────┘
```

### Component Breakdown

| Component | Purpose |
|-----------|---------|
| **Cunfyooz Agent** | Performs binary obfuscation and metamorphic transformations |
| **Analysis Agent** | Evaluates transformation effectiveness and binary properties |
| **Agent Orchestrator** | Coordinates single, swarm, and pipeline execution |
| **Binary Tools** | Specialized tools for binary analysis and transformation |
| **Memory System** | Persists agent state and transformation history |
| **Communication Layer** | Enables inter-agent collaboration |

## Installation

### Prerequisites

- Python 3.8+
- Cunfyooz binary (built from source)
- Claude API key

### Step 1: Set Up Virtual Environment

```bash
cd /path/to/cunfyooz
python3 -m venv venv
source venv/bin/activate  # On Windows: venv\Scripts\activate
```

### Step 2: Install Dependencies

```bash
pip install -r requirements.txt
```

### Step 3: Set Claude API Key

```bash
export ANTHROPIC_API_KEY="your-claude-api-key-here"
```

### Step 4: Build Cunfyooz Binary

Ensure the cunfyooz binary is built and located at `./bin/cunfyooz`:

```bash
make  # From the cunfyooz project root
```

## Quick Start

### Single Agent Execution

```python
import os
os.environ['ANTHROPIC_API_KEY'] = 'your-api-key'

from agents.cunfyooz_agent import CunfyoozAgent

# Configure the agent
config = {
    "model": "claude-sonnet-4-5-20250929",
    "max_tokens": 4000,
    "temperature": 1.0
}

# Initialize and run
agent = CunfyoozAgent(config)
result = agent.run("analyze and obfuscate binary at ./malware_sample.exe")

print(result['findings'])
```

### Multi-Agent Swarm

```python
from agents.cunfyooz_agent import CunfyoozAgent, CunfyoozAnalysisAgent
from framework import AgentOrchestrator

# Initialize orchestrator and agents
orchestrator = AgentOrchestrator({"max_concurrent_agents": 3})
cunfyooz_agent = CunfyoozAgent(config)
analysis_agent = CunfyoozAnalysisAgent(config)

# Register agents
orchestrator.register_agent("cunfyooz", cunfyooz_agent)
orchestrator.register_agent("analysis", analysis_agent)

# Run swarm
result = orchestrator.run_swarm(
    task="process binary ./sample.exe",
    agent_ids=["cunfyooz", "analysis"]
)
```

### Sequential Pipeline

```python
# Run agents in sequence with context passing
result = orchestrator.run_pipeline(
    task="comprehensive binary analysis",
    agent_ids=["cunfyooz", "analysis"],
    initial_context={"binary_path": "./sample.exe"}
)
```

## Agent Components

### CunfyoozAgent

The primary agent for binary obfuscation and transformation.

#### Capabilities
- PE binary parsing
- Metamorphic code generation
- Binary obfuscation
- NOP insertion
- Instruction substitution
- Register shuffling
- Control flow obfuscation
- Binary validation

#### Methods
- `run(task, context)`: Execute transformation task
- `get_tools()`: Get available tools for Claude API

#### Example Usage
```python
agent = CunfyoozAgent(config)
result = agent.run("obfuscate binary at ./target.exe with maximum entropy")
```

### CunfyoozAnalysisAgent

Specialized agent for analyzing binary properties and transformation effectiveness.

#### Capabilities
- Binary structure analysis
- Size comparison
- Entropy analysis
- Signature detection
- Transformation effectiveness evaluation

#### Example Usage
```python
analysis_agent = CunfyoozAnalysisAgent(config)
result = analysis_agent.run(
    "analyze transformation effectiveness", 
    context={
        "original_binary": "./original.exe",
        "transformed_binary": "./cunfyoozed_original.exe"
    }
)
```

## Usage Examples

### Example 1: Single Agent Execution

**File**: `examples/single_agent.py`

```python
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent.parent))

from agents.cunfyooz_agent import CunfyoozAgent

def main():
    config = {
        "model": "claude-sonnet-4-5-20250929",
        "max_tokens": 4000,
        "temperature": 1.0
    }

    agent = CunfyoozAgent(config)
    task = "analyze and obfuscate binary at ./test_binary.exe"
    result = agent.run(task)

    print(f"Status: {result['status']}")
    if result['status'] == 'success':
        print(f"Findings:\n{result['findings']}")
        print(f"Artifacts: {len(result['artifacts'])}")

if __name__ == "__main__":
    import os
    if not os.getenv("ANTHROPIC_API_KEY"):
        print("WARNING: ANTHROPIC_API_KEY not set")
    main()
```

### Example 2: Multi-Agent Swarm

**File**: `examples/swarm.py`

```python
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent.parent))

from agents.cunfyooz_agent import CunfyoozAgent, CunfyoozAnalysisAgent
from framework import AgentOrchestrator

def main():
    config = {
        "model": "claude-sonnet-4-5-20250929",
        "max_tokens": 4000,
        "temperature": 1.0
    }

    orchestrator = AgentOrchestrator({"max_concurrent_agents": 3})
    cunfyooz_agent = CunfyoozAgent(config)
    analysis_agent = CunfyoozAnalysisAgent(config)

    orchestrator.register_agent("cunfyooz", cunfyooz_agent)
    orchestrator.register_agent("analysis", analysis_agent)

    result = orchestrator.run_swarm(
        task="perform binary obfuscation and analysis on ./test_binary.exe",
        agent_ids=["cunfyooz", "analysis"]
    )

    print(f"Successful: {result['successful']}")
    print(f"Failed: {result['failed']}")

if __name__ == "__main__":
    main()
```

### Example 3: Sequential Pipeline

**File**: `examples/pipeline.py`

```python
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent.parent))

from agents.cunfyooz_agent import CunfyoozAgent, CunfyoozAnalysisAgent
from framework import AgentOrchestrator

def main():
    config = {
        "model": "claude-sonnet-4-5-20250929",
        "max_tokens": 4000,
        "temperature": 1.0
    }

    orchestrator = AgentOrchestrator()
    cunfyooz_agent = CunfyoozAgent(config)
    analysis_agent = CunfyoozAnalysisAgent(config)

    orchestrator.register_agent("cunfyooz", cunfyooz_agent)
    orchestrator.register_agent("analysis", analysis_agent)

    result = orchestrator.run_pipeline(
        task="obfuscate and analyze binary ./test_binary.exe",
        agent_ids=["cunfyooz", "analysis"],
        initial_context={"binary_path": "./test_binary.exe"}
    )

    if result['status'] == 'success':
        print(f"Pipeline completed: {result['stages_completed']} stages")
        for stage in result['pipeline_results']:
            print(f"Stage {stage['stage']}: {stage['agent_id']} - {stage['result']['status']}")

if __name__ == "__main__":
    main()
```

## Advanced Usage

### Custom Agent Creation

Create your own specialized agent by inheriting from `BaseAgent`:

```python
from framework import BaseAgent, BinaryToolDefinitions

class MyCustomAgent(BaseAgent):
    def __init__(self, config):
        super().__init__(
            agent_id="my_custom_agent",
            name="My Custom Agent",
            description="Performs specialized binary analysis",
            capabilities=["specialized_analysis", "custom_transformations"],
            config=config
        )

    def get_tools(self):
        return [
            BinaryToolDefinitions.binary_info(),
            BinaryToolDefinitions.binary_entropy(),
            BinaryToolDefinitions.run_cunfyooz(),
        ]

    def run(self, task, context=None):
        # Your custom logic here
        messages = [{"role": "user", "content": task}]
        response = self.call_claude(messages=messages)
        
        # Process response and return results
        return {
            "status": "success",
            "findings": response.content[0].text if response.content else "",
            "artifacts": self.artifacts,
            "metadata": {}
        }
```

### Binary Tool Definitions

The framework provides specialized tools for binary operations:

#### Available Binary Tools
- `binary_info()`: Get basic information about a binary file
- `binary_compare()`: Compare two binary files and report differences
- `binary_entropy()`: Calculate entropy of a binary file to assess randomness
- `pe_analysis()`: Analyze PE file structure and properties
- `run_cunfyooz()`: Run cunfyooz metamorphic transformation

#### Example Tool Usage
```python
from framework import BinaryToolExecutor

executor = BinaryToolExecutor()
result = executor.execute_tool("binary_entropy", {"path": "./sample.exe"})
print(result)  # Outputs entropy information
```

### Cunfyooz Wrapper

The Python wrapper provides direct access to cunfyooz functionality:

```python
from cunfyooz_wrapper import CunfyoozWrapper

wrapper = CunfyoozWrapper()
result = wrapper.transform_binary(
    input_path="./input.exe",
    config={
        "transformations": {
            "nop_insertion": {"enabled": True, "probability": 5},
            "instruction_substitution": {"enabled": True, "probability": 10}
        }
    }
)
```

## Configuration

### Agent Configuration

Each agent accepts a configuration dictionary with the following options:

```python
config = {
    "model": "claude-sonnet-4-5-20250929",  # Claude model to use
    "max_tokens": 4000,                     # Maximum tokens in response
    "temperature": 1.0,                     # Sampling temperature (0.0-1.0)
}
```

### Orchestrator Configuration

The orchestrator accepts configuration for managing agent execution:

```python
orchestrator_config = {
    "max_concurrent_agents": 5,  # Maximum agents running in parallel
    "timeout": 300               # Timeout in seconds
}
```

### Cunfyooz Transformation Configuration

Transformation parameters can be customized:

```python
cunfyooz_config = {
    "transformations": {
        "nop_insertion": {
            "enabled": True,
            "probability": 5
        },
        "instruction_substitution": {
            "enabled": True,
            "probability": 10
        },
        "register_shuffling": {
            "enabled": True,
            "probability": 8
        },
        "enhanced_nop_insertion": {
            "enabled": True,
            "probability": 3
        },
        "control_flow_obfuscation": {
            "enabled": True,
            "probability": 5
        },
        "stack_frame_obfuscation": {
            "enabled": True,
            "probability": 2
        },
        "instruction_reordering": {
            "enabled": True,
            "probability": 5
        },
        "anti_analysis_techniques": {
            "enabled": True,
            "probability": 15
        },
        "virtualization_engine": {
            "enabled": False,
            "probability": 10
        }
    },
    "output": {
        "verbose": True,
        "log_transformations": True
    },
    "security": {
        "validate_functionality": False,  /* CHANGED FROM True TO False FOR SECURITY */
        "preserve_original_behavior": True
    }
}
```

## Troubleshooting

### Common Issues

#### 1. Claude API Key Not Set
**Error**: `ANTHROPIC_API_KEY environment variable not set`
**Solution**: 
```bash
export ANTHROPIC_API_KEY="your-api-key"
```

#### 2. Cunfyooz Binary Not Found
**Error**: `cunfyooz binary not found`
**Solution**: Ensure the cunfyooz binary is built and located at `./bin/cunfyooz`

#### 3. Import Errors
**Error**: Module not found
**Solution**: Ensure you're running from the cunfyooz project root and virtual environment is activated

### Debugging Tips

1. **Enable Verbose Logging**: Set `verbose: True` in configuration
2. **Check Dependencies**: Run `pip list` to verify all dependencies are installed
3. **Validate Binary**: Ensure input binaries are valid PE files
4. **Monitor Resources**: Large transformations may require significant memory/time

### Testing the Framework

Verify the installation works:

```bash
# Activate virtual environment
source venv/bin/activate

# Test basic imports
python -c "from agents.cunfyooz_agent import CunfyoozAgent; print('Success')"

# Run example
python examples/single_agent.py
```

## API Reference

### BaseAgent Class

The base class for all agents inherits from the Claude Agent Framework.

#### Methods
- `__init__(agent_id, name, description, capabilities, config)`: Initialize agent
- `call_claude(messages, tools=None, max_tokens=4000, temperature=1.0)`: Call Claude API
- `run(task, context=None)`: Execute agent task (abstract method)
- `get_tools()`: Get available tools (overrideable)
- `save_artifact(artifact_data, artifact_type)`: Save an artifact
- `start()`: Mark agent as running
- `complete(results)`: Mark agent as completed
- `fail(error)`: Mark agent as failed

### AgentOrchestrator Class

Manages execution of multiple agents.

#### Methods
- `__init__(config=None)`: Initialize orchestrator
- `register_agent(agent_id, agent)`: Register an agent
- `run_agent(agent_id, task, context=None)`: Run single agent
- `run_swarm(task, agent_ids=None, context=None)`: Run agents in parallel
- `run_pipeline(task, agent_ids, initial_context=None)`: Run agents sequentially

### BinaryToolDefinitions Class

Provides specialized tools for binary operations.

#### Methods
- `binary_info()`: Get binary information tool definition
- `binary_compare()`: Get binary comparison tool definition
- `binary_entropy()`: Get entropy calculation tool definition
- `pe_analysis()`: Get PE analysis tool definition
- `run_cunfyooz()`: Get cunfyooz execution tool definition
- `get_all_binary_tools()`: Get all binary tools

### CunfyoozWrapper Class

Python wrapper for the cunfyooz binary.

#### Methods
- `__init__(cunfyooz_path=None)`: Initialize wrapper
- `transform_binary(input_path, output_path=None, config=None)`: Transform binary
- `analyze_binary(binary_path)`: Analyze binary properties
- `compare_binaries(original_path, transformed_path)`: Compare two binaries

### CunfyoozPipeline Class

High-level pipeline for complete transformation workflows.

#### Methods
- `__init__(cunfyooz_path=None)`: Initialize pipeline
- `run_complete_transformation(input_path, config=None)`: Complete transformation pipeline

---

## Conclusion

The Cunfyooz AI Agent Framework provides a powerful platform for intelligent binary analysis and transformation. By combining the Claude AI API with the cunfyooz metamorphic engine, users can automate complex binary obfuscation tasks while leveraging AI for decision-making and analysis.

The modular architecture allows for easy extension with custom agents and tools, making it suitable for a wide range of binary analysis and security research applications.