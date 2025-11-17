<div align="center">
  <h1>cunfyooz</h1>
  <p><b>THE METAMORPHIC BINARY OBFUSCATOR</b></p>
  
  <img src="./assets/sigil.gif" alt="cunfyooz sigil" width="400">
</div>

<div align="center">
  
  ![C](https://img.shields.io/badge/c-%2300599C.svg?style=for-the-badge&logo=c&logoColor=white)
  &nbsp;
  ![Capstone](https://img.shields.io/badge/Capstone-Disassembly-%23FF6B6B.svg?style=for-the-badge)
  &nbsp;
  ![Keystone](https://img.shields.io/badge/Keystone-Assembly-%234CAF50.svg?style=for-the-badge)
  &nbsp;
  ![PE](https://img.shields.io/badge/PE-Binary-%23FF9800.svg?style=for-the-badge)
  &nbsp;
  ![License](https://img.shields.io/badge/License-Public%20Domain-%23000000.svg?style=for-the-badge)
  
</div>

<p align="center">
  <a href="#overview">Overview</a> •
  <a href="#features">Features</a> •
  <a href="#building-and-usage">Usage</a> •
  <a href="#architecture">Architecture</a> •
  <a href="#transformation-pipeline">Pipeline</a> •
  <a href="#examples">Examples</a> •
  <a href="#contributing">Contributing</a>
</p>

<hr>

<br>

## 🎯 OVERVIEW

**cunfyooz** is a metamorphic code engine designed to transform PE binaries into functionally equivalent but structurally unique variants.

Metamorphic code changes its appearance with each generation while preserving its core functionality, making it resistant to signature-based detection and static analysis.

### 🔄 THE PIPELINE

---

<div align="center">
  <img src="./assets/sigil2.gif" alt="Transformation visualization" width="600">
</div>

---

**cunfyooz**:

1. **PARSES** PE binary structure and extracts executable sections
2. **DISASSEMBLES** instructions using Capstone engine
3. **ANALYZES** control flow and data dependencies
4. **TRANSFORMS** code through multiple obfuscation passes
5. **REASSEMBLES** and reconstructs the modified PE binary

The engine applies sophisticated transformations including NOP insertion, instruction substitution, register shuffling, control flow obfuscation, and optional virtualization.

<br>

## 🚀 BUILDING AND USAGE

### PREREQUISITES

- **GCC** - C compiler for building the project
- **Capstone** - Disassembly framework
- **Keystone** - Assembly framework
- **GNU Make** - Build automation

### 🔨 BUILDING

**INSTALL CAPSTONE:**

```bash
git clone https://github.com/aquynh/capstone.git
cd capstone
make
sudo make install
```

**INSTALL KEYSTONE:**

```bash
git clone https://github.com/keystone-engine/keystone.git
cd keystone
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON ..
make
sudo make install
```

**BUILD CUNFYOOZ:**

```bash
make
```

This will compile the source code and create the `cunfyooz` executable in the `bin` directory.

### 📝 BASIC USAGE

**1. PREPARE YOUR PE BINARY**

Ensure you have a valid PE executable file (e.g., `target.exe`)

**2. RUN CUNFYOOZ**

```bash
./bin/cunfyooz target.exe
```

The tool will generate a transformed binary named:

```bash
cunfyoozed_target.exe
```

### 🔧 CONFIGURATION

`cunfyooz` can be customized using a `config.json` file placed in the working directory:

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

**CONFIGURATION PARAMETERS:**

- `enabled` - Enable/disable specific transformations
- `probability` - Percentage chance (1-100) for applying transformation
- `verbose` - Output detailed transformation logs
- `validate_functionality` - Verify transformed binary maintains original behavior

<br>

## ⚡ FEATURES

<table>
<tr>
<td width="50%">

### CORE CAPABILITIES

- ✅ **True random metamorphosis** using time-based entropy
- 🔀 **Sophisticated NOP insertion** at safe locations
- 🔄 **Functional instruction substitution** preserving semantics
- 🎲 **Register shuffling** with dependency preservation
- 🌐 **Control flow obfuscation** via opaque predicates
- 📦 **Stack frame manipulation** for analysis resistance
- 🔢 **Instruction reordering** within basic blocks
- 🛡️ **Anti-debugging techniques** for runtime protection
- 💾 **Optional virtualization** for maximum obfuscation

</td>
<td width="50%">

### TRANSFORMATION TECHNIQUES

#### 🎯 BASIC TRANSFORMATIONS
- NOP insertion at safe points
- Multi-byte NOP variations (XCHG, LEA, TEST)
- Instruction substitution (LEA↔MOV, TEST↔CMP)
- Register renaming with constraint satisfaction

#### 🧩 ADVANCED TECHNIQUES
- Opaque predicate insertion
- Dead code injection
- Stack manipulation sequences
- Control flow flattening
- Code virtualization

</td>
</tr>
</table>

<br>

## 🗂️ MODULAR ARCHITECTURE

`cunfyooz` features a clean, layered architecture for binary transformation:

### 📦 CORE COMPONENTS

| Component | Purpose |
|-----------|---------|
| **PE Parser** | Extracts and analyzes PE structure |
| **Disassembler** | Powered by Capstone for instruction analysis |
| **Control Flow Analyzer** | Maps execution paths and basic blocks |
| **Data Flow Analyzer** | Tracks register and memory dependencies |
| **Transformer** | Applies obfuscation techniques |
| **Assembler** | Reconstructs code using Keystone |
| **PE Reconstructor** | Rebuilds transformed binary |

### 🎨 TRANSFORMATION MODULES

Specialized modules for different obfuscation techniques:

- **NOP Insertion Engine** - Strategic insertion of padding instructions
- **Instruction Substitutor** - Functional equivalence replacements
- **Register Shuffler** - Register allocation randomization
- **Control Flow Obfuscator** - Flow graph complexity injection
- **Stack Frame Manipulator** - Call frame structure obfuscation
- **Instruction Reorderer** - Dependency-aware instruction scheduling
- **Anti-Analysis Module** - Runtime detection and evasion
- **Virtualization Engine** - Code-to-bytecode transformation

### 🎨 ARCHITECTURE BENEFITS

<table>
<tr>
<td>🔧 <b>Maintainability</b></td>
<td>Modular design with clear separation</td>
</tr>
<tr>
<td>📈 <b>Extensibility</b></td>
<td>Easy addition of new transformations</td>
</tr>
<tr>
<td>✅ <b>Reliability</b></td>
<td>Validation at each transformation stage</td>
</tr>
<tr>
<td>🚀 <b>Performance</b></td>
<td>Efficient multi-pass architecture</td>
</tr>
<tr>
<td>🔒 <b>Safety</b></td>
<td>Dependency analysis prevents corruption</td>
</tr>
</table>

<br>

## 🔄 TRANSFORMATION PIPELINE

`cunfyooz` employs a sophisticated 10-stage transformation pipeline:

### STAGE 1: CONFIGURATION LOADING

<details>
<summary><b>Click to expand configuration details</b></summary>

- Parse `config.json` if present
- Load transformation probabilities and settings
- Apply default parameters for missing values
- Validate configuration integrity

</details>

### STAGE 2: PE PARSING

<details>
<summary><b>Click to expand PE parsing details</b></summary>

- Parse DOS and NT headers
- Extract section table information
- Locate `.text` section containing executable code
- Preserve metadata for reconstruction

</details>

### STAGE 3: DISASSEMBLY

<details>
<summary><b>Click to expand disassembly details</b></summary>

- Use Capstone to disassemble `.text` section
- Create instruction list with metadata
- Identify operand types and addressing modes
- Map instruction boundaries and offsets

</details>

### STAGE 4: CONTROL FLOW ANALYSIS

<details>
<summary><b>Click to expand control flow details</b></summary>

- Identify basic blocks and entry points
- Map jump targets and branch destinations
- Detect loops and recursive structures
- Build control flow graph

</details>

### STAGE 5: DATA FLOW ANALYSIS

<details>
<summary><b>Click to expand data flow details</b></summary>

- Track register dependencies
- Analyze memory access patterns
- Build def-use chains
- Identify critical registers (RSP, RBP)

</details>

### STAGE 6: NOP INSERTION

<details>
<summary><b>Click to expand NOP insertion details</b></summary>

**Basic NOPs:**
- Insert `0x90` NOP instructions at safe locations
- Avoid insertion after branches or calls

**Enhanced NOPs:**
- `xchg rax, rax` - Self-exchange (2 bytes)
- `lea rax, [rax + 0]` - Zero-offset LEA (3+ bytes)
- `test rax, rax` - Self-test (3 bytes)

</details>

### STAGE 7: INSTRUCTION SUBSTITUTION

<details>
<summary><b>Click to expand substitution details</b></summary>

| Original | Replacement |
|----------|-------------|
| `lea rax, [rbx]` | `mov rax, rbx` |
| `test rax, rax` | `cmp rax, 0` |
| `add rax, 0` | `nop` or removal |
| `sub rax, 0` | `nop` or removal |

</details>

### STAGE 8: REGISTER SHUFFLING

<details>
<summary><b>Click to expand register shuffling details</b></summary>

- Rename registers while preserving dependencies
- Maintain critical registers (RSP, RBP) unchanged
- Update all operand references consistently
- Validate register liveness ranges

</details>

### STAGE 9: CONTROL FLOW OBFUSCATION

<details>
<summary><b>Click to expand control flow obfuscation details</b></summary>

**Opaque Predicates:**
```asm
; Original
cmp eax, ebx
jne skip
call function
skip:

; Obfuscated
cmp eax, ebx
je else_part
jmp skip
else_part:
call function
skip:
```

**Dead Code Injection:**
- Insert unreachable basic blocks
- Add false branch targets
- Create phantom execution paths

</details>

### STAGE 10: RECONSTRUCTION

<details>
<summary><b>Click to expand reconstruction details</b></summary>

- Reassemble instructions using Keystone
- Update section sizes and offsets
- Reconstruct PE headers and metadata
- Validate binary integrity
- Write transformed executable

</details>

<br>

## 📋 TRANSFORMATION EXAMPLES

### 🔹 NOP INSERTION

<details>
<summary><b>Click to expand NOP examples</b></summary>

**Basic NOP Insertion:**

```asm
; Original
mov rax, rbx
add rax, 5

; Transformed
mov rax, rbx
nop
add rax, 5
```

**Enhanced NOP Insertion:**

```asm
; Original
mov rax, rbx
add rax, 5

; Transformed
mov rax, rbx
xchg rax, rax
add rax, 5
```

</details>

### 🔹 INSTRUCTION SUBSTITUTION

<details>
<summary><b>Click to expand substitution examples</b></summary>

**LEA to MOV:**

```asm
; Original
lea rax, [rbx]

; Transformed
mov rax, rbx
```

**TEST to CMP:**

```asm
; Original
test rax, rax
je label

; Transformed
cmp rax, 0
je label
```

</details>

### 🔹 REGISTER SHUFFLING

<details>
<summary><b>Click to expand register shuffling examples</b></summary>

```asm
; Original
mov rax, 1
mov rbx, 2
add rax, rbx

; Transformed
mov rcx, 1
mov rdx, 2
add rcx, rdx
```

</details>

### 🔹 CONTROL FLOW OBFUSCATION

<details>
<summary><b>Click to expand control flow examples</b></summary>

**Opaque Predicate Insertion:**

```asm
; Original
cmp eax, ebx
jne skip
call function
skip:

; Transformed with opaque predicate
cmp eax, ebx
je else_part
jmp skip
else_part:
call function
skip:
```

</details>

### 🔹 STACK FRAME OBFUSCATION

<details>
<summary><b>Click to expand stack frame examples</b></summary>

```asm
; Original
mov rax, 5
add rbx, rax

; Transformed
push rax
mov rax, 5
add rbx, rax
pop rax
```

</details>

<br>

## 🔬 CORE CONCEPTS

### METAMORPHIC CODE

Metamorphic code transforms itself while maintaining functional equivalence. Key principles:

**1️⃣ SEMANTIC PRESERVATION**
- Original functionality remains intact
- All execution paths preserved
- Register and memory state consistency maintained

**2️⃣ STRUCTURAL VARIATION**
- Each transformation produces unique binary
- Time-based random seed ensures uniqueness
- Multiple transformation techniques applied simultaneously

**3️⃣ ANALYSIS RESISTANCE**
- Static signatures become ineffective
- Pattern matching fails due to variation
- Control flow complexity increases analysis difficulty

### TRANSFORMATION STRATEGY

**SAFE TRANSFORMATION:**
- Dependency analysis prevents corruption
- Critical registers (RSP, RBP) preserved
- Branch targets automatically updated
- Validation ensures correctness

**RANDOMIZATION:**
- Probability-based transformation selection
- Time-seeded random number generation
- Multiple valid transformations for same instruction

<br>

## ⚠️ LIMITATIONS AND FUTURE DEVELOPMENT

`cunfyooz` is under active development with planned enhancements:

### CURRENT LIMITATIONS

- **PE Format Only** - Currently supports Windows PE binaries exclusively
- **x86/x64 Architecture** - Limited to Intel/AMD instruction sets
- **Basic Block Preservation** - Some advanced optimizations not yet implemented
- **Section Expansion** - Transformed binaries may grow significantly in size

### FUTURE ROADMAP

- 🔄 ELF binary support for Linux executables
- 📚 ARM architecture support
- 🎯 Enhanced virtualization with custom VM
- ⚡ Code compression to reduce size overhead
- 🧪 Automated testing and validation framework
- 🔐 Integration with packing techniques

<br>

## 🛡️ SECURITY CONSIDERATIONS

### ANTIVIRUS DETECTION

Metamorphic transformations may trigger antivirus heuristics due to:
- Code structure changes
- Anti-debugging techniques
- Unusual instruction patterns

**Recommendation:** Test transformed binaries in isolated environments

### FUNCTIONALITY VALIDATION

Always validate transformed binaries:
- Test all execution paths
- Verify input/output behavior
- Check edge cases and error handling

### RESPONSIBLE USE

This tool is intended for:
- Security research
- Malware analysis education
- Software protection research
- Academic study of code transformation

**Do not use for malicious purposes.**

<br>

## 🤝 CONTRIBUTING

Contributions are welcome! Areas for contribution:

- 🐛 Bug reports and fixes
- 💡 New transformation techniques
- 🔧 Architecture improvements
- 📖 Documentation enhancements
- 🧪 Test case development

<br>

## 📄 LICENSE

`cunfyooz` is released into the **public domain** under the Unlicense.

```
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or 
distribute this software, either in source code form or as a compiled 
binary, for any purpose, commercial or non-commercial, and by any 
means.

In jurisdictions that recognize copyright laws, the author or authors 
of this software dedicate any and all copyright interest in the 
software to the public domain. We make this dedication for the benefit 
of the public at large and to the detriment of our heirs and 
successors. We intend this dedication to be an overt act of 
relinquishment in perpetuity of all present and future rights to this 
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF 
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR 
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, 
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR 
OTHER DEALINGS IN THE SOFTWARE.
```

<br>

<div align="center">
  <hr>
  <p><i>May your binaries shift ceaselessly</i></p>
  <p><b>cunfyooz</b> - metamorphic code engine for PE binaries</p>
</div>