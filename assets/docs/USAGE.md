# cunfyooz Usage Guide

This guide provides detailed instructions on how to use cunfyooz, including command-line usage, configuration options, and best practices.

## Table of Contents
1. [Basic Usage](#basic-usage)
2. [Command-Line Options](#command-line-options)
3. [Configuration](#configuration)
4. [Advanced Usage Examples](#advanced-usage-examples)
5. [Troubleshooting](#troubleshooting)
6. [Best Practices](#best-practices)
7. [Security and Ethical Considerations](#security-and-ethical-considerations)

## Basic Usage

### Simple Transformation

The most basic usage of cunfyooz is to transform a single PE file:

```bash
./bin/cunfyooz input_program.exe
```

This command will:
1. Parse `input_program.exe` to extract its PE sections
2. Apply all available transformation techniques in sequence
3. Generate a transformed file named `cunfyoozed_input_program.exe`
4. Display transformation progress to the console

### Output File Naming

cunfyooz automatically generates output files by prepending `cunfyoozed_` to the input filename. For example:
- Input: `calculator.exe` → Output: `cunfyoozed_calculator.exe`
- Input: `/path/to/program.dll` → Output: `/path/to/cunfyoozed_program.dll`

## Command-Line Options

cunfyooz currently accepts a single parameter:

```
./bin/cunfyooz <input_pe_binary>
```

- `input_pe_binary`: Path to the PE file (EXE, DLL, etc.) to be transformed

### Supported File Types
- Windows executable files (.exe)
- Dynamic Link Libraries (.dll)
- Any PE format binary with valid headers

## Configuration

cunfyooz supports optional configuration through a `config.json` file located in the working directory. If this file exists, cunfyooz will load the settings before beginning transformation.

### Configuration File Example

Create a `config.json` file in the working directory:

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
    "validate_functionality": false,  /* CHANGED FROM true TO false FOR SECURITY */
    "preserve_original_behavior": true
  }
}
```

### Configuration Options

#### Transformations Settings
- `enabled`: Boolean value to enable or disable each transformation
- `probability`: Percentage chance (0-100) that the transformation will be applied at appropriate locations

#### Output Settings
- `verbose`: When true, displays detailed transformation information
- `log_transformations`: When true, records all transformations applied

#### Security Settings
- `validate_functionality`: Perform validation checks to ensure functionality is preserved (DEFAULT: false for security - enables execution of binaries for comparison)
- `preserve_original_behavior`: Attempt to maintain original program behavior as closely as possible

## Advanced Usage Examples

### Example 1: Transform with Minimal Obfuscation
To apply minimal transformations while maintaining functionality:

```bash
# Create a minimal config.json
cat > config.json << EOF
{
  "transformations": {
    "nop_insertion": {"enabled": true, "probability": 1},
    "instruction_substitution": {"enabled": true, "probability": 2},
    "register_shuffling": {"enabled": false, "probability": 0},
    "enhanced_nop_insertion": {"enabled": false, "probability": 0},
    "control_flow_obfuscation": {"enabled": true, "probability": 1},
    "stack_frame_obfuscation": {"enabled": false, "probability": 0},
    "instruction_reordering": {"enabled": false, "probability": 0},
    "anti_analysis_techniques": {"enabled": false, "probability": 0},
    "virtualization_engine": {"enabled": false, "probability": 0}
  }
}
EOF

# Run transformation
./bin/cunfyooz target_program.exe
```

### Example 2: Maximum Obfuscation
To apply all available transformations with high intensity:

```bash
# Create a maximum obfuscation config.json
cat > config.json << EOF
{
  "transformations": {
    "nop_insertion": {"enabled": true, "probability": 25},
    "instruction_substitution": {"enabled": true, "probability": 30},
    "register_shuffling": {"enabled": true, "probability": 20},
    "enhanced_nop_insertion": {"enabled": true, "probability": 15},
    "control_flow_obfuscation": {"enabled": true, "probability": 15},
    "stack_frame_obfuscation": {"enabled": true, "probability": 10},
    "instruction_reordering": {"enabled": true, "probability": 20},
    "anti_analysis_techniques": {"enabled": true, "probability": 25},
    "virtualization_engine": {"enabled": true, "probability": 15}
  }
}
EOF

# Run transformation
./bin/cunfyooz target_program.exe
```

### Example 3: Testing Transformation Consistency
To verify that transformations preserve functionality:

```bash
# Transform the same file multiple times
./bin/cunfyooz test_program.exe
./bin/cunfyooz test_program.exe
./bin/cunfyooz test_program.exe

# Compare the outputs - they should be functionally equivalent but structurally different
ls -la cunfyoozed_test_program.exe*
```

## Troubleshooting

### Common Issues and Solutions

#### Issue: Segmentation Fault during Transformation
**Symptoms:** Program terminates with "Segmentation fault" message
**Solutions:**
1. Check that the input file is a valid PE binary
2. Verify that Capstone and Keystone libraries are properly installed
3. Ensure sufficient system memory is available
4. Try with a smaller or simpler binary to test functionality

#### Issue: Invalid PE File Created
**Symptoms:** Transformed file doesn't run or reports "This app can't be run on your PC"
**Solutions:**
1. Verify the original file was a valid PE executable
2. Check that critical PE headers are preserved (AddressOfEntryPoint, ImageBase, etc.)
3. Try reducing transformation probability in config.json
4. Ensure the original file has a standard PE structure

#### Issue: Large File Size Increase
**Symptoms:** Transformed file is significantly larger than the original
**Solutions:**
- This is expected behavior due to NOP insertion and other transformations
- Use a configuration with lower transformation probabilities
- The increase is due to obfuscation techniques and does not affect functionality

#### Issue: Antivirus Detection
**Symptoms:** Antivirus software flags transformed files
**Solutions:**
- This is expected as transformations include techniques used by malware
- Use in isolated environments
- This is normal behavior for obfuscation tools

### Debugging Tips

1. **Start Simple:** Begin with minimal transformations to verify basic functionality
2. **Validate Original:** Ensure the original file runs correctly before transformation
3. **Check Configuration:** Verify config.json syntax with a JSON validator
4. **Monitor Output:** Pay attention to console output for transformation progress and errors
5. **Incremental Changes:** Gradually increase transformation probabilities to find the right balance

## Best Practices

### 1. Environment Setup
- Use an isolated virtual machine for testing
- Maintain backups of original files
- Ensure proper development environment with Capstone/Keystone

### 2. Configuration Strategy
- Start with conservative settings and gradually increase
- Test functionality preservation after each transformation run
- Document successful configurations for different types of binaries

### 3. Validation Process
- Always verify that transformed binaries execute the same basic functions
- Compare file hashes to confirm structural differences
- Test in environments similar to target deployment

### 4. Performance Considerations
- Larger binaries will take longer to transform
- Higher transformation probabilities will increase processing time
- Monitor system resources during transformation

---

- Use responsibly in security research, academic study, and reverse engineering education

---

For more information about the technical architecture and implementation details, refer to the technical README in the docs/README.md file.