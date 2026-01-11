# cunfyooz

cunfyooz is a powerful metamorphic engine implemented in C, designed to transform binary executables while preserving their functionality. It applies various code obfuscation techniques to make reverse engineering and analysis more difficult.

A metamorphic engine is a sophisticated piece of software that can modify its own code or the code of other programs while preserving the original functionality. The key characteristic of metamorphic code is that each generation appears completely different from the previous one, making detection and analysis extremely challenging for security researchers and antivirus software.

## What is Metamorphic Code?

Metamorphic code is code that mutates itself into functionally equivalent but structurally different code each time it runs or replicates. The transformed code maintains the same logical behavior and functionality as the original, but appears completely different to analysis tools. This technique is used both by malware authors to evade detection and by security researchers to understand obfuscation techniques and develop better detection methods.

### Key Properties of Metamorphic Engines:
* **Functional Equivalence**: The transformed code maintains the exact same functionality as the original
* **Structural Diversity**: Each transformation produces code that appears completely different from the original
* **Self-Modification**: The engine can transform its own code or target code
* **Anti-Analysis**: The transformations make static and dynamic analysis more difficult

## Use Cases

cunfyooz applies transformations to modify the binary's structure while preserving its functions. These modifications are used to:
* Frustrate reverse-engineering efforts
* Impede static and dynamic analysis
* Obfuscate code for protection against tampering
* Study and research metamorphic code behavior and detection methods
* Educational purposes to understand obfuscation techniques

## Supported Formats

cunfyooz currently supports:
* **PE binaries** - Windows executables (via `MZ` magic followed by PE header)

## Features

- **True Metamorphic Randomization**: Each transformation run produces different outputs with randomized patterns, intervals, and instruction sequences while preserving functionality. The engine uses a seeded random number generator based on time and clock to ensure different transformations on each run.
- **Sophisticated NOP Insertion**: Inserts NOP instructions more carefully by identifying safe locations to maintain binary functionality. The engine avoids inserting NOPs after control flow instructions like jumps, calls, and returns to prevent behavioral changes.
- **Functional Instruction Substitution**: Replaces instructions with functionally equivalent alternatives while preserving code size. For example, `LEA reg, [base]` can be substituted with `MOV reg, base`, or conditional jumps with equivalent constructs.
- **Functional Control Flow Obfuscation**: Modifies control flow structures while preserving functionality and code size. This includes adding opaque predicates and junk code blocks that don't affect program execution.
- **Size-Changing NOP Insertion**: Inserts NOP instructions and properly handles size changes for maximum transformation. The engine supports various types of NOPs beyond the standard `0x90`, including `xchg rax, rax`, `lea rax, [rax + 0x0]`, `test rax, rax`, `add rax, 0`, and `sub rax, 0`.
- **Instruction Reordering**: Reorders instructions within basic blocks to obfuscate code structure. The engine performs dependency analysis to ensure that data and control dependencies are preserved during reordering.
- **Register Shuffling**: Randomizes register allocation to obfuscate code structure while preserving functionality. The engine carefully avoids modifying critical registers like RSP and RBP that control stack operations.
- **Stack Frame Obfuscation**: Modifies stack frame operations to obfuscate function structure while preserving functionality. This includes adding push/pop pairs of registers that preserve the original register state.
- **Virtualization Engine**: Converts code to bytecode for execution in a virtual machine with optional obfuscation (with enhanced stability for multiple transformation rounds, graceful error handling, and fallback mechanisms to preserve functionality). The virtualization engine wraps the original code in a custom virtual machine that interprets obfuscated bytecode.
- **Anti-Analysis Techniques**: Implements various anti-analysis techniques including:
  - **Debugger Detection**: Uses techniques like checking the PEB (Process Environment Block) for debugging flags
  - **Reverse Engineering Tool Detection**: Identifies the presence of common reverse engineering tools
  - **Timing-Based Anti-Analysis**: Uses RDTSC (Read Time-Stamp Counter) to detect virtual machines or emulators based on timing differences
  - **Process Debugging State Check**: Monitors for signs of active debugging
  - **Hardware Breakpoint Detection**: Detects hardware breakpoints by inspecting processor debug registers
  - **Kernel-Level Debugging Detection**: Identifies kernel-level debugging mechanisms
- **Enhanced Configuration System**: Fully configurable transformation parameters with per-transformer intensity controls and enable/disable options via JSON configuration files, now with robust JSON parsing capabilities
- **Advanced Validation**: Comprehensive binary validation with execution time comparison, return code verification, and environment-based behavioral analysis to ensure transformed binaries maintain original functionality
- **Improved Output Formatting**: Clean, well-formatted output with proper newlines and structured information display
- **Functional Anti-Analysis Implementations**: All anti-analysis techniques now have actual functional implementations instead of placeholder messages, providing real protection against reverse engineering tools including debugger detection, reverse engineering tool detection, timing-based anti-analysis, process debugging state checks, hardware breakpoint detection, and kernel-level debugging detection
- **Advanced Disassembly**: Uses the Capstone Engine for accurate x86-64 disassembly and instruction analysis with detailed operand information
- **Assembly Generation**: Uses the Keystone Engine for generating transformed assembly code from instruction representations
- **PE Format Support**: Parses and transforms Portable Executable (PE) files with proper section handling, maintaining critical headers like AddressOfEntryPoint, ImageBase, and Subsystem
- **Stable Memory Management**: Fixed memory allocation/deallocation issues to prevent segmentation faults and memory leaks through proper resource management
- **Robust PE File Writing**: Handles PE files with expanded code sections while maintaining proper headers and structure to ensure compatibility with the operating system
- **Error Handling**: Comprehensive error checking throughout the transformation pipeline to prevent crashes and provide meaningful error messages
- **Control Flow Analysis**: Performs detailed control flow analysis to identify basic blocks, entry points, and branch targets for safe transformations
- **Data Flow Analysis**: Analyzes data dependencies between instructions to ensure transformations preserve program semantics
- **Dependency Graphing**: Creates dependency graphs to safely reorder instructions while respecting data and control dependencies

### Standard Features

- **Instruction substitution transformation** for x86/x86_64: Replaces instructions with functionally equivalent alternatives, such as converting `LEA reg, [base]` to `MOV reg, base` when appropriate
- **Register shuffling transformation** for x86/x86_64: Randomly reassigns registers while preserving program semantics and avoiding critical registers like RSP and RBP
- **Enhanced NOP insertion** with multiple types of NOPs (including `xchg rax, rax`, `lea rax, [rax + 0x0]`, `test rax, rax`, `add rax, 0`, `sub rax, 0`) to increase obfuscation
- **Code transposition transformation**: Rearranges code blocks to create structural differences while preserving execution flow
- **Control flow obfuscation** with junk code blocks: Inserts opaque predicates and unreachable code to confuse control flow analysis
- **Instruction reordering transformation** with basic dependency analysis: Safely reorders independent instructions based on data dependency analysis
- **Stack frame manipulation**: Adds obfuscating push/pop operations that preserve register states but complicate analysis
- **Anti-analysis techniques** including timing-based checks using RDTSC to detect virtualized or emulated environments
- **Virtualization engine** with bytecode translation: Converts code to custom bytecode with a virtual machine wrapper for execution
- **PE binary parsing and section extraction**: Properly parses PE headers, sections, and metadata to ensure transformed executables remain valid
- **Address handling for x86-64 architecture** with Intel syntax: Maintains correct addressing modes and instruction encodings during transformations
- **Basic Block Analysis**: Identifies and analyzes basic blocks to perform safe transformations within control flow boundaries
- **Dependency Analysis**: Tracks register and memory dependencies to ensure transformations preserve program semantics
- **JSON Configuration Parsing**: Robust JSON parser for loading transformation configuration parameters from external files, enabling flexible customization of metamorphic behavior

## Architecture

cunfyooz follows a modular architecture with the following components:

- **PE Parser**: Handles parsing of Portable Executable files and extraction of code sections, including proper handling of PE headers, section tables, and metadata. The parser identifies the `.text` section containing executable code and extracts it for transformation.
- **Disassembler**: Uses Capstone to disassemble executable code into instruction lists with detailed operand information. The disassembler maintains instruction addresses and metadata for proper reconstruction.
- **Transformer**: Implements transformation algorithms to modify the instructions through multiple passes, including NOP insertion, instruction substitution, register shuffling, control flow obfuscation, and other techniques. Each transformer operates on instruction lists and maintains functional equivalence.
- **Virtualization Engine**: Translates native code to custom bytecode for execution in a virtual machine. The engine creates a wrapper that interprets the obfuscated bytecode, adding an extra layer of obfuscation.
- **Assembler**: Uses Keystone to reassemble transformed instructions back into binary format. The assembler handles instruction encoding and produces executable machine code from the transformed instruction list.
- **Main Controller**: Orchestrates the entire transformation pipeline by coordinating between all components. The controller manages the sequential application of transformations and ensures proper resource management.
- **Control Flow Analysis Engine**: Performs detailed analysis to identify basic blocks, entry points, and branch targets for safe transformations. This component ensures that control flow modifications don't break program semantics.
- **Data Flow Analysis Engine**: Analyzes data dependencies between instructions to ensure transformations preserve program semantics. The engine creates dependency graphs to guide safe instruction reordering.
- **Anti-Analysis Module**: Implements various techniques to detect and respond to analysis tools, including timing-based checks, debugger detection, and virtualization detection.
- **JSON Configuration Parser**: Parses JSON configuration files to customize transformation parameters, enabling flexible control over metamorphic behavior through external configuration files.

## Transformation Pipeline

The transformation pipeline applies multiple obfuscation techniques in sequence with detailed control flow and data flow analysis to ensure functional equivalence:

1. **Configuration Loading**: Loads transformation parameters from `config.json` if present, allowing customization of transformation probabilities and enable/disable settings for each technique
2. **NOP Insertion**: Inserts various types of NOP instructions in safe locations, avoiding insertion after control flow instructions to preserve program behavior
3. **Instruction Substitution**: Replaces instructions with functionally equivalent alternatives, such as converting `LEA reg, [base]` to `MOV reg, base` where semantically appropriate
4. **Register Shuffling**: Reorders register usage through safe transformations that avoid critical registers (RSP, RBP) and maintain semantic equivalence
5. **Enhanced NOP Insertion**: Additional sophisticated NOP patterns using various equivalent instructions like `xchg rax, rax`, `lea rax, [rax + 0x0]`, etc.
6. **Control Flow Obfuscation**: Inserts junk code blocks and opaque predicates that don't affect program execution but complicate control flow analysis
7. **Stack Frame Obfuscation**: Manipulates stack operations with push/pop pairs that preserve register states but obfuscate the code structure
8. **Instruction Reordering**: Rearranges independent instructions based on dependency analysis to preserve program semantics while changing code structure  
9. **Anti-Analysis Techniques**: Implements protection against analysis tools using timing checks (RDTSC), CPUID instructions, and other anti-debugging techniques
10. **Virtualization**: Transforms code for execution in a custom virtual machine with bytecode interpretation for maximum obfuscation

Each transformation step includes validation to ensure the transformed code maintains functional equivalence to the original while becoming structurally different.

## JSON Configuration Parser

cunfyooz now includes a robust JSON configuration parser that allows users to customize transformation behavior through external configuration files. The parser implements a lightweight, dependency-free JSON parser that can handle the specific configuration format used by cunfyooz.

### Features

- **Lightweight Implementation**: Built from scratch without external dependencies, keeping the engine self-contained
- **Flexible Configuration**: Allows fine-grained control over each transformation technique's probability and enable/disable status
- **Error Handling**: Provides detailed error messages for malformed configuration files
- **Default Values**: Automatically falls back to default settings when configuration values are missing or invalid
- **Extensible Design**: Easily extendable to support additional configuration options

### Configuration File Format

The JSON configuration file follows this structure:

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

### Implementation Details

The JSON parser is implemented in `src/json_parser.c` and `include/json_parser.h` with the following key features:

- **Token-based Parsing**: Uses a simple tokenizer to break JSON input into manageable tokens
- **Recursive Descent Parser**: Implements a recursive descent parser for handling nested objects and arrays
- **Memory Management**: Efficiently allocates and deallocates memory for parsed configuration data
- **Type Safety**: Ensures proper type handling for boolean, integer, and string values
- **Error Recovery**: Provides meaningful error messages when encountering malformed JSON

### Usage

To use the JSON configuration parser, simply create a `config.json` file in the working directory where cunfyooz is executed. The engine will automatically detect and load the configuration file if it exists.

## Building

To build cunfyooz, you need to have GCC and the Capstone/Keystone libraries installed:

```bash
# Install Capstone Engine
git clone https://github.com/aquynh/capstone.git
cd capstone
make
sudo make install

# Install Keystone Engine
git clone https://github.com/keystone-engine/keystone.git
cd keystone
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON ..
make
sudo make install

# Build cunfyooz
make
```

## Usage

To use cunfyooz, simply run:

```bash
./bin/cunfyooz <input_pe_binary>
```

The engine will automatically parse the PE file, extract code sections, apply transformations in sequence, and output a new transformed PE file with the same functionality but with obfuscated code. The transformed executable will be named with a `cunfyoozed_` prefix (e.g., if the input is `program.exe`, the output will be `cunfyoozed_program.exe`).

### Command Line Options

Currently, cunfyooz accepts a single parameter:
- `<input_pe_binary>`: Path to the PE file to be transformed

### Configuration

cunfyooz also supports configuration through a JSON file. If `config.json` exists in the working directory, it will be loaded to customize transformation parameters. See USAGE.md for detailed configuration options.

## Testing

To test the engine with the provided test binary:

```bash
# Build the engine
make

# Test with the provided PE binary
./bin/cunfyooz test_pe.exe
```

This will create a transformed executable named `test_pe.exe_transformed` in the same directory. The transformed executable will have all the metamorphic transformations applied while maintaining the same basic PE structure and functionality.

## How It Works

cunfyooz performs binary transformation through the following process:

1. **PE Parsing**: The engine first parses the Portable Executable (PE) file to locate the `.text` section containing executable code, extracting both the raw bytes and relevant metadata like virtual addresses and section characteristics.

2. **Disassembly**: Using the Capstone Engine in detailed mode, the raw code bytes are disassembled into a structured list of instructions with complete operand information, addressing modes, and metadata.

3. **Analysis**: Each transformation includes analysis phases:
   - Control Flow Analysis: Identifies basic blocks, branch targets, and execution paths
   - Data Flow Analysis: Tracks register usage, dependencies, and memory accesses
   - Dependency Graphing: Maps data and control dependencies to enable safe transformations

4. **Transformation**: The instruction list undergoes multiple sequential transformation passes, with each pass modifying the instruction list while preserving functional equivalence.

5. **Validation**: Throughout the transformation pipeline, cunfyooz validates that transformations maintain program semantics and don't introduce behavioral changes.

6. **Assembly**: The transformed instruction list is reassembled back into executable machine code using the Keystone Engine.

7. **PE Reconstruction**: The new code is written back to a PE file structure, updating headers and sections appropriately while maintaining all necessary metadata for proper execution.

## Dependencies

cunfyooz requires:
- GCC (or compatible C compiler)
- Capstone disassembly framework
- Keystone assembly framework
- GNU Make

## Transformation Examples

cunfyooz applies various transformation techniques to obfuscate code while preserving functionality. Here are examples of the transformations applied:

### NOP Insertion Examples

Original code:
```asm
mov rax, rbx
add rax, 5
```

After NOP insertion:
```asm
mov rax, rbx
nop
add rax, 5
```

Enhanced NOP insertion might include:
```asm
mov rax, rbx
xchg rax, rax
add rax, 5
```

### Instruction Substitution Examples

Original code:
```asm
lea rax, [rbx]
```

After substitution:
```asm
mov rax, rbx
```

Another example:
```asm
; Original
test rax, rax
je label
```
```asm
; Alternative form
cmp rax, 0
je label
```

### Register Shuffling Examples

Original code:
```asm
mov rax, 1
mov rbx, 2
add rax, rbx
```

After register shuffling:
```asm
mov rcx, 1
mov rdx, 2
add rcx, rdx
; Result still in target register after proper mapping
```

### Control Flow Obfuscation Examples

Original code:
```asm
cmp eax, ebx
jne skip
call function
skip:
```

After obfuscation:
```asm
cmp eax, ebx
je else_part
jmp skip
else_part:
call function
skip:
```

### Stack Frame Obfuscation Examples

Original code:
```asm
mov rax, 5
add rbx, rax
```

After stack frame obfuscation:
```asm
push rax
mov rax, 5
add rbx, rax
pop rax
```

## Security Considerations

cunfyooz is designed as a research and educational tool to study metamorphic code techniques. Users should be aware of the following security considerations:

- **Legal Compliance**: Ensure use of this tool complies with applicable laws and regulations in your jurisdiction
- **Ethical Use**: This tool should only be used on binaries you own or have explicit permission to analyze
- **Antivirus Software**: The transformations applied by cunfyooz may trigger antivirus software as they include techniques similar to those used by malware
- **System Security**: Transformed binaries may behave unpredictably; run in isolated environments when possible
- **Educational Purpose**: This tool is primarily intended for educational and research purposes in reverse engineering and security analysis

## Troubleshooting

### Common Issues and Solutions:
- **Segmentation Faults**: These have been resolved by fixing memory allocation/deallocation issues between Capstone and custom transformations
- **PE File Creation Failure**: The PE writer now properly handles cases where transformed code is larger than the original section by expanding the file appropriately
- **Invalid PE Files**: The PE writer maintains proper headers and structure to ensure transformed executables are valid
- **"This app can't be run on your PC" Error**: Fixed by properly preserving critical PE headers including AddressOfEntryPoint, ImageBase, Subsystem, SizeOfImage, SizeOfCode, section characteristics, and all execution environment fields to maintain file structure integrity
- **Metamorphic Consistency**: The engine now produces different outputs on each run with randomized transformation patterns, intervals, and instruction sequences (true metamorphism)
- **Large Code Expansion**: The metamorphic engine can significantly increase code size due to NOP insertion and other transformations - this is expected behavior

## Contributing

Contributions are welcome! Please feel free to submit pull requests or open issues to improve the engine's capabilities.

## License

cunfyooz is licensed under the MIT License. See the LICENSE file for details.