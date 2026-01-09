# Cunfyooz Usage Examples

This document provides comprehensive examples of how to use the cunfyooz binary obfuscation framework, including both the core transformation engine and the AI-powered agent framework.

## Table of Contents
1. [Basic Binary Transformation](#basic-binary-transformation)
2. [Advanced Agent-Based Analysis](#advanced-agent-based-analysis)
3. [Pipeline Workflows](#pipeline-workflows)
4. [Configuration Options](#configuration-options)
5. [Troubleshooting](#troubleshooting)

## Basic Binary Transformation

### Simple Transformation
Transform a binary using default settings:

```bash
python cunfyooz_cli.py transform my_binary.exe
```

This will create a transformed binary named `cunfyoozed_my_binary.exe` in the same directory.

### Custom Output Path
Specify a custom output path for the transformed binary:

```bash
python cunfyooz_cli.py transform my_binary.exe -o obfuscated_binary.exe
```

### Using Custom Configuration
Apply transformations using a custom configuration file:

```bash
python cunfyooz_cli.py transform my_binary.exe -c my_config.json -o obfuscated_binary.exe
```

### Compare Original and Transformed Binaries
Compare the original and transformed binaries after transformation:

```bash
python cunfyooz_cli.py transform my_binary.exe --compare
```

## Advanced Agent-Based Analysis

### Basic Binary Analysis
Analyze a binary using AI agents:

```bash
python cunfyooz_cli.py analyze my_binary.exe
```

### Specify Custom Task
Provide a specific task for the agents to perform:

```bash
python cunfyooz_cli.py analyze my_binary.exe -t "identify potential anti-debugging techniques in this binary"
```

### Use Different Claude Model
Specify a different Claude model for analysis:

```bash
python cunfyooz_cli.py analyze my_binary.exe -m claude-opus-4-5-20250929
```

### Run Agents in Pipeline Mode
Execute agents in sequential pipeline mode instead of parallel swarm:

```bash
python cunfyooz_cli.py analyze my_binary.exe --pipeline
```

## Pipeline Workflows

### Complete Analysis Pipeline
Run a complete analysis pipeline that includes transformation and analysis:

```bash
python cunfyooz_cli.py pipeline my_binary.exe
```

This command executes a comprehensive workflow that:
1. Analyzes the original binary for potential vulnerabilities and characteristics
2. Applies obfuscation transformations based on default or specified configuration
3. Validates the transformed binary maintains original functionality
4. Performs post-transformation analysis to assess effectiveness

### Pipeline with Custom Configuration
Run a complete pipeline with a custom configuration file:

```bash
python cunfyooz_cli.py pipeline my_binary.exe -c my_config.json
```

### Pipeline with Verbose Output
Get detailed information about each step in the pipeline:

```bash
python cunfyooz_cli.py pipeline my_binary.exe -v
```

### Pipeline with Custom Output Directory
Specify a custom directory for pipeline outputs:

```bash
python cunfyooz_cli.py pipeline my_binary.exe -o ./pipeline_outputs/
```

### Pipeline with Specific Agent Task
Run pipeline with a specific analysis task for the agents:

```bash
python cunfyooz_cli.py pipeline my_binary.exe -t "focus on control flow obfuscation techniques"
```

### Pipeline with Multiple Configuration Files
Chain multiple configuration files for layered transformations:

```bash
python cunfyooz_cli.py pipeline my_binary.exe -c config_layer1.json -c config_layer2.json
```

### Pipeline Skipping Validation
Skip functionality validation for faster processing (use with caution):

```bash
python cunfyooz_cli.py pipeline my_binary.exe --skip-validation
```

### Pipeline with Model Specification
Use a specific Claude model for the analysis agents:

```bash
python cunfyooz_cli.py pipeline my_binary.exe -m claude-opus-4-5-20250929
```

### Pipeline with Agent Limiting
Limit the number of concurrent agents in the pipeline:

```bash
python cunfyooz_cli.py pipeline my_binary.exe --max-agents 3
```

### Pipeline with Different Obfuscation Levels

#### Light Obfuscation Pipeline
For minimal obfuscation that preserves performance:

```bash
python cunfyooz_cli.py pipeline my_binary.exe -c light_obfuscation.json
```

Example `light_obfuscation.json`:
```json
{
  "transformations": {
    "nop_insertion": {
      "enabled": true,
      "probability": 3
    },
    "instruction_substitution": {
      "enabled": true,
      "probability": 5
    },
    "register_shuffling": {
      "enabled": false,
      "probability": 0
    },
    "enhanced_nop_insertion": {
      "enabled": true,
      "probability": 2
    },
    "control_flow_obfuscation": {
      "enabled": false,
      "probability": 0
    },
    "stack_frame_obfuscation": {
      "enabled": false,
      "probability": 0
    },
    "instruction_reordering": {
      "enabled": true,
      "probability": 3
    },
    "anti_analysis_techniques": {
      "enabled": false,
      "probability": 0
    },
    "virtualization_engine": {
      "enabled": false,
      "probability": 0
    }
  }
}
```

#### Aggressive Obfuscation Pipeline
For maximum protection at the cost of performance:

```bash
python cunfyooz_cli.py pipeline my_binary.exe -c aggressive_obfuscation.json
```

Example `aggressive_obfuscation.json`:
```json
{
  "transformations": {
    "nop_insertion": {
      "enabled": true,
      "probability": 20
    },
    "instruction_substitution": {
      "enabled": true,
      "probability": 25
    },
    "register_shuffling": {
      "enabled": true,
      "probability": 15
    },
    "enhanced_nop_insertion": {
      "enabled": true,
      "probability": 10
    },
    "control_flow_obfuscation": {
      "enabled": true,
      "probability": 20
    },
    "stack_frame_obfuscation": {
      "enabled": true,
      "probability": 10
    },
    "instruction_reordering": {
      "enabled": true,
      "probability": 15
    },
    "anti_analysis_techniques": {
      "enabled": true,
      "probability": 30
    },
    "virtualization_engine": {
      "enabled": true,
      "probability": 20
    }
  }
}
```

#### Evasive Obfuscation Pipeline
Optimized for evading detection by security tools:

```bash
python cunfyooz_cli.py pipeline my_binary.exe -c evasive_obfuscation.json
```

Example `evasive_obfuscation.json`:
```json
{
  "transformations": {
    "nop_insertion": {
      "enabled": true,
      "probability": 8
    },
    "instruction_substitution": {
      "enabled": true,
      "probability": 12
    },
    "register_shuffling": {
      "enabled": true,
      "probability": 10
    },
    "enhanced_nop_insertion": {
      "enabled": true,
      "probability": 5
    },
    "control_flow_obfuscation": {
      "enabled": true,
      "probability": 15
    },
    "stack_frame_obfuscation": {
      "enabled": false,
      "probability": 0
    },
    "instruction_reordering": {
      "enabled": true,
      "probability": 10
    },
    "anti_analysis_techniques": {
      "enabled": true,
      "probability": 25
    },
    "virtualization_engine": {
      "enabled": false,
      "probability": 0
    }
  }
}
```

## Configuration Options

### Default Configuration Structure
The default configuration includes various transformation techniques:

```json
{
  "transformations": {
    "nop_insertion": {
      "enabled": true,
      "probability": 5
    },
    "instruction_substitution": {
      "enabled": true,
      "probability": 10
    },
    "register_shuffling": {
      "enabled": true,
      "probability": 8
    },
    "enhanced_nop_insertion": {
      "enabled": true,
      "probability": 3
    },
    "control_flow_obfuscation": {
      "enabled": true,
      "probability": 5
    },
    "stack_frame_obfuscation": {
      "enabled": true,
      "probability": 2
    },
    "instruction_reordering": {
      "enabled": true,
      "probability": 5
    },
    "anti_analysis_techniques": {
      "enabled": true,
      "probability": 15
    },
    "virtualization_engine": {
      "enabled": false,
      "probability": 10
    }
  },
  "output": {
    "verbose": true,
    "log_transformations": true
  },
  "security": {
    "validate_functionality": true,
    "preserve_original_behavior": true
  }
}
```

## Advanced Pipeline Usage Scenarios

### Conditional Pipeline Execution
Execute different pipelines based on binary characteristics:

```bash
# Check if binary contains specific characteristics before applying pipeline
if python cunfyooz_cli.py analyze my_binary.exe -t "check if binary contains anti-debugging techniques" | grep -q "yes"; then
  python cunfyooz_cli.py pipeline my_binary.exe -c advanced_protection.json
else
  python cunfyooz_cli.py pipeline my_binary.exe -c standard_protection.json
fi
```

### Pipeline with Iterative Refinement
Apply multiple pipeline iterations for enhanced obfuscation:

```bash
# First iteration with standard config
python cunfyooz_cli.py pipeline my_binary.exe -c standard_config.json -o temp_output.exe

# Second iteration with aggressive config on already obfuscated binary
python cunfyooz_cli.py pipeline temp_output.exe -c aggressive_config.json -o final_output.exe

# Clean up intermediate files
rm temp_output.exe
```

### Pipeline with External Validation
Integrate external validation tools into the pipeline:

```bash
#!/bin/bash
BINARY=$1

# Run the pipeline
python cunfyooz_cli.py pipeline "$BINARY" -o obfuscated_"$BINARY"

# Validate with external tools
if hash clamscan 2>/dev/null; then
  echo "Running antivirus scan on obfuscated binary..."
  clamscan "obfuscated_$BINARY"
fi

# Check file integrity
sha256sum "$BINARY" > original_checksum.txt
sha256sum "obfuscated_$BINARY" > obfuscated_checksum.txt

echo "Original checksum: $(cat original_checksum.txt)"
echo "Obfuscated checksum: $(cat obfuscated_checksum.txt)"
```

### Pipeline with Performance Profiling
Profile the performance impact of different obfuscation levels:

```bash
#!/bin/bash
BINARY=$1

echo "Profiling original binary performance..."
time "./$BINARY" > original_output.txt 2> original_time.txt

# Apply light obfuscation
python cunfyooz_cli.py pipeline "$BINARY" -c light_obfuscation.json -o light_obfuscated.exe
echo "Profiling light obfuscated binary performance..."
time "./light_obfuscated.exe" > light_output.txt 2> light_time.txt

# Apply heavy obfuscation
python cunfyooz_cli.py pipeline "$BINARY" -c aggressive_obfuscation.json -o heavy_obfuscated.exe
echo "Profiling heavy obfuscated binary performance..."
time "./heavy_obfuscated.exe" > heavy_output.txt 2> heavy_time.txt

# Compare performance
echo "Performance comparison:"
echo "Original: $(cat original_time.txt | grep real | awk '{print $2}')"
echo "Light obfuscation: $(cat light_time.txt | grep real | awk '{print $2}')"
echo "Heavy obfuscation: $(cat heavy_time.txt | grep real | awk '{print $2}')"
```

### Pipeline with Continuous Integration
Integrate pipeline into CI/CD workflows:

```yaml
# .github/workflows/obfuscation.yml
name: Binary Obfuscation Pipeline

on:
  push:
    branches: [ main ]
  pull_request:
    branches: [ main ]

jobs:
  obfuscate:
    runs-on: ubuntu-latest

    steps:
    - uses: actions/checkout@v2

    - name: Set up Python
      uses: actions/setup-python@v2
      with:
        python-version: 3.9

    - name: Install dependencies
      run: |
        pip install -r requirements.txt
        pip install anthropic

    - name: Set API key
      env:
        ANTHROPIC_API_KEY: ${{ secrets.ANTHROPIC_API_KEY }}
      run: |
        echo "ANTHROPIC_API_KEY=$ANTHROPIC_API_KEY" >> $GITHUB_ENV

    - name: Run obfuscation pipeline
      run: |
        python cunfyooz_cli.py pipeline my_binary.exe -c ci_config.json -o obfuscated_binary.exe

    - name: Upload obfuscated binary
      uses: actions/upload-artifact@v2
      with:
        name: obfuscated-binary
        path: obfuscated_binary.exe
```

### Chaining Multiple Pipeline Operations
Chain multiple pipeline operations for complex workflows:

```bash
# Sequential pipeline execution with different configurations
python cunfyooz_cli.py pipeline my_binary.exe -c layer1_config.json -o stage1_output.exe && \
python cunfyooz_cli.py pipeline stage1_output.exe -c layer2_config.json -o stage2_output.exe && \
python cunfyooz_cli.py pipeline stage2_output.exe -c layer3_config.json -o final_output.exe

# Clean up intermediate files
rm stage1_output.exe stage2_output.exe
```

### Parallel Pipeline Execution
Run multiple pipelines in parallel for different analysis aspects:

```bash
# Run security-focused pipeline in background
python cunfyooz_cli.py pipeline my_binary.exe -c security_config.json -o security_output.exe &

# Run performance-focused pipeline in background
python cunfyooz_cli.py pipeline my_binary.exe -c performance_config.json -o performance_output.exe &

# Run compatibility-focused pipeline in background
python cunfyooz_cli.py pipeline my_binary.exe -c compatibility_config.json -o compatibility_output.exe &

# Wait for all pipelines to complete
wait

echo "All pipeline operations completed"
```

### Pipeline with Rollback Capability
Implement pipeline with rollback in case of failure:

```bash
#!/bin/bash
BINARY=$1
BACKUP="${BINARY}.backup"

# Create backup of original binary
cp "$BINARY" "$BACKUP"

# Attempt pipeline execution
if python cunfyooz_cli.py pipeline "$BINARY" -c production_config.json -o obfuscated_"$BINARY"; then
    echo "Pipeline completed successfully"
    # Remove backup after successful completion
    rm "$BACKUP"
else
    echo "Pipeline failed, restoring from backup"
    cp "$BACKUP" "$BINARY"
    rm "$BACKUP"
    exit 1
fi
```

### Pipeline with Dynamic Configuration Selection
Select pipeline configuration dynamically based on runtime conditions:

```bash
#!/bin/bash
BINARY=$1
SIZE=$(stat -c%s "$BINARY")

if [ $SIZE -lt 1000000 ]; then  # Less than 1MB
    CONFIG="light_config.json"
elif [ $SIZE -lt 5000000 ]; then  # Between 1-5MB
    CONFIG="medium_config.json"
else  # Larger than 5MB
    CONFIG="heavy_config.json"
fi

echo "Selected configuration: $CONFIG for binary size: $SIZE bytes"
python cunfyooz_cli.py pipeline "$BINARY" -c "$CONFIG" -o "obfuscated_$BINARY"
```

### Pipeline with Progress Tracking
Track pipeline progress and log each stage:

```bash
#!/bin/bash
BINARY=$1
LOG_FILE="pipeline_$(date +%Y%m%d_%H%M%S).log"

echo "$(date): Starting pipeline for $BINARY" >> "$LOG_FILE"

echo "$(date): Step 1 - Initial analysis" >> "$LOG_FILE"
python cunfyooz_cli.py analyze "$BINARY" >> "$LOG_FILE" 2>&1

echo "$(date): Step 2 - Applying transformations" >> "$LOG_FILE"
python cunfyooz_cli.py pipeline "$BINARY" -c default_config.json -o temp_output.exe >> "$LOG_FILE" 2>&1

echo "$(date): Step 3 - Post-transformation analysis" >> "$LOG_FILE"
python cunfyooz_cli.py analyze "temp_output.exe" -t "assess transformation effectiveness" >> "$LOG_FILE" 2>&1

echo "$(date): Step 4 - Validation" >> "$LOG_FILE"
python cunfyooz_cli.py transform "temp_output.exe" --compare >> "$LOG_FILE" 2>&1

mv temp_output.exe "final_$(basename $BINARY .exe)_obfuscated.exe"

echo "$(date): Pipeline completed for $BINARY" >> "$LOG_FILE"
echo "Pipeline log saved to $LOG_FILE"
```

### Custom Configuration File
Create a custom configuration file to fine-tune transformation parameters:

```json
{
  "transformations": {
    "nop_insertion": {
      "enabled": true,
      "probability": 15
    },
    "instruction_substitution": {
      "enabled": true,
      "probability": 20
    },
    "register_shuffling": {
      "enabled": false,
      "probability": 0
    },
    "enhanced_nop_insertion": {
      "enabled": true,
      "probability": 5
    },
    "control_flow_obfuscation": {
      "enabled": true,
      "probability": 10
    },
    "stack_frame_obfuscation": {
      "enabled": true,
      "probability": 5
    },
    "instruction_reordering": {
      "enabled": true,
      "probability": 8
    },
    "anti_analysis_techniques": {
      "enabled": true,
      "probability": 25
    },
    "virtualization_engine": {
      "enabled": true,
      "probability": 15
    }
  },
  "output": {
    "verbose": true,
    "log_transformations": true
  },
  "security": {
    "validate_functionality": true,
    "preserve_original_behavior": true
  }
}
```

## Troubleshooting

### Common Issues and Solutions

#### Issue: ANTHROPIC_API_KEY not set
**Error**: "ANTHROPIC_API_KEY environment variable not set"
**Solution**: Set your API key:
```bash
export ANTHROPIC_API_KEY=your_api_key_here
```

#### Issue: Binary transformation fails
**Error**: "Failed to parse PE file or not a PE file"
**Solution**: Ensure the input file is a valid PE (Windows executable) file. The current version supports PE files only.

#### Issue: Validation fails
**Error**: "Validation failed: Transformed executable produces different output than original"
**Solution**: The transformation may have altered the program's behavior. Note that functionality validation is disabled by default for security reasons (it executes binaries for comparison). To enable validation, set `"validate_functionality": true` in your configuration file, but be aware this executes both original and transformed binaries.

#### Issue: Large binary processing takes too long
**Solution**: Reduce the number of enabled transformations or lower their probabilities in the configuration file.

### Performance Tips

1. **For faster processing**: Disable virtualization_engine and reduce probabilities of complex transformations
2. **For stronger obfuscation**: Enable all transformations with higher probabilities
3. **For preserving functionality**: Validation is disabled by default for security (to prevent automatic execution of binaries). To enable validation, set `"validate_functionality": true` in your configuration, but be aware this executes both original and transformed binaries for comparison.

### Environment Setup

Before using the agent framework, ensure you have the required dependencies:

```bash
pip install anthropic
```

Set up your Anthropic API key:

```bash
export ANTHROPIC_API_KEY=your_api_key_here
```

## Advanced Usage Examples

### Multi-Stage Analysis
Combine multiple analysis techniques:

```bash
# First, transform the binary
python cunfyooz_cli.py transform my_binary.exe -o obfuscated.exe

# Then analyze with agents
python cunfyooz_cli.py analyze obfuscated.exe -t "analyze the obfuscation techniques applied to this binary"
```

### Batch Processing
Process multiple binaries in sequence:

```bash
for binary in *.exe; do
  python cunfyooz_cli.py transform "$binary" -o "cunfyoozed_$binary"
done
```

### Automated Pipeline
Create an automated pipeline that transforms and validates binaries:

```bash
#!/bin/bash
BINARY_PATH=$1
CONFIG_PATH=${2:-"default_config.json"}

echo "Processing binary: $BINARY_PATH"

# Transform the binary
python cunfyooz_cli.py transform "$BINARY_PATH" -c "$CONFIG_PATH" --compare

# Analyze the results
python cunfyooz_cli.py analyze "${BINARY_PATH%.exe}_cunfyoozed.exe" -t "provide a security assessment of this transformed binary"
```

## Practical Use Cases

### Case 1: Malware Research
For security researchers studying malware samples:

```bash
# Analyze a suspicious binary
python cunfyooz_cli.py analyze suspicious_sample.exe -t "identify potential anti-debugging and anti-analysis techniques"

# Transform the sample for safer analysis
python cunfyooz_cli.py transform suspicious_sample.exe -c aggressive_config.json -o safe_sample.exe

# Compare before and after
python cunfyooz_cli.py pipeline suspicious_sample.exe
```

### Case 2: Software Protection
For protecting legitimate software from reverse engineering:

```bash
# Apply strong obfuscation to protect intellectual property
python cunfyooz_cli.py transform commercial_app.exe -c protection_config.json -o protected_app.exe --compare

# Verify the protected binary functions correctly
python cunfyooz_cli.py analyze protected_app.exe -t "verify that core functionality remains intact after obfuscation"
```

### Case 3: Red Team Operations
For penetration testers needing to evade detection:

```bash
# Transform tools to avoid signature-based detection
python cunfyooz_cli.py transform pentest_tool.exe -c evasive_config.json -o evasive_tool.exe

# Analyze transformation effectiveness
python cunfyooz_cli.py analyze evasive_tool.exe -t "assess how well the transformations obscure the original functionality"
```

## Agent Framework Capabilities

The cunfyooz agent framework includes several specialized agents:

1. **CunfyoozAgent**: Performs metamorphic transformations on PE binaries
2. **CunfyoozAnalysisAgent**: Analyzes binary properties and evaluates transformation effectiveness
3. **ResearchAgent**: Gathers and analyzes information (general purpose)
4. **AnalysisAgent**: Processes and interprets data (general purpose)

These agents can work together in pipeline or swarm configurations to provide comprehensive binary analysis and obfuscation services.

## Development and Debugging

### Debugging Transformations
When troubleshooting transformation issues:

```bash
# Use verbose output to see detailed transformation logs
python cunfyooz_cli.py transform debug_binary.exe --config verbose_config.json

# Analyze why certain transformations didn't apply
python cunfyooz_cli.py analyze cunfyoozed_debug_binary.exe -t "explain why certain obfuscation techniques were not applied"
```

### Custom Agent Development
To create custom agents for specific analysis tasks:

```python
from framework import BaseAgent, ToolDefinitions

class CustomSecurityAgent(BaseAgent):
    def __init__(self, config):
        super().__init__(
            agent_id="custom_security_agent",
            name="Custom Security Agent",
            description="Specialized security analysis for specific threat models",
            capabilities=[
                "Threat modeling",
                "Vulnerability assessment",
                "Security hardening recommendations"
            ],
            config=config
        )

    def run(self, task, context=None):
        # Implement custom analysis logic
        pass
```

Then integrate it with the CLI by adding it to the agent registry.