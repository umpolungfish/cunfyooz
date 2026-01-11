#include "transformer.h"
#include "assembler.h" // New include
#include "relocation.h" // Relocation tracking
#include <stdlib.h>
#include <string.h>
#include <capstone/capstone.h>
#include <stdio.h> // For printf
#include <inttypes.h>
#include <ctype.h> // For isdigit
#include <time.h>  // For randomization

// Maximum bytes for x86-64 instructions - increase from default 16 to handle all cases
#define MAX_INSN_BYTES 24  // Some x86-64 instructions with prefixes can exceed 16 bytes

// Helper function to safely assemble and copy instruction bytes
// Returns 1 on success, 0 on failure
static int safe_assemble_instruction(cs_insn* target_insn, const char* asm_str, uint64_t address, x86_insn id) {
    size_t asm_size;
    unsigned char* asm_bytes = assemble_instruction(asm_str, address, &asm_size);

    if (!asm_bytes) {
        fprintf(stderr, "Warning: Failed to assemble: %s\n", asm_str);
        return 0;
    }

    // Check if assembled instruction fits in cs_insn.bytes[]
    if (asm_size > sizeof(target_insn->bytes)) {
        fprintf(stderr, "Warning: Assembled instruction too large (%zu bytes): %s\n", asm_size, asm_str);
        fprintf(stderr, "         Maximum size is %zu bytes. Skipping this transformation.\n", sizeof(target_insn->bytes));
        free(asm_bytes);
        return 0;
    }

    // Check for zero-size assembly (invalid result)
    if (asm_size == 0) {
        fprintf(stderr, "Warning: Assembled instruction has zero size: %s\n", asm_str);
        free(asm_bytes);
        return 0;
    }

    // Copy the assembled bytes and set size
    memcpy(target_insn->bytes, asm_bytes, asm_size);
    target_insn->size = asm_size;
    target_insn->id = id;
    free(asm_bytes);

    return 1;
}

// Control Flow Analysis Data Structures
typedef struct basic_block {
    size_t start_idx;           // Start index in instruction list
    size_t end_idx;             // End index in instruction list
    struct basic_block** successors;  // List of successor basic blocks
    size_t num_successors;
    struct basic_block** predecessors; // List of predecessor basic blocks  
    size_t num_predecessors;
    uint64_t start_address;     // Virtual address of first instruction
    uint64_t end_address;       // Virtual address of last instruction
} basic_block;

typedef struct control_flow_graph {
    basic_block** blocks;       // Array of basic blocks
    size_t num_blocks;          // Number of basic blocks
    size_t capacity;            // Capacity of blocks array
    size_t entry_block_idx;     // Index of entry block
} control_flow_graph;

// Data Flow Analysis Data Structures
typedef struct {
    int reg_used[16];        // Track which registers are used
    int reg_defined[16];     // Track which registers are defined
    int stack_pointer;       // Track stack pointer changes
    int memory_accessed;     // Track memory access patterns
} data_flow_info;

// Dependency Graph Data Structures
typedef struct instruction_dependency {
    size_t from_idx;          // Index of source instruction
    size_t to_idx;            // Index of target instruction
    int dependency_type;      // 0=data, 1=control, 2=anti (WAW/WAR)
} instruction_dependency;

typedef struct dependency_graph {
    instruction_dependency** deps;  // Array of dependencies
    size_t num_deps;                // Number of dependencies
    size_t capacity;                // Capacity of deps array
} dependency_graph;

// Function to initialize data flow info
data_flow_info* init_data_flow_info() {
    data_flow_info* info = (data_flow_info*)calloc(1, sizeof(data_flow_info));
    if (info) {
        for (int i = 0; i < 16; i++) {
            info->reg_used[i] = 0;
            info->reg_defined[i] = 0;
        }
        info->stack_pointer = 0;
        info->memory_accessed = 0;
    }
    return info;
}

instruction_list* apply_nop_insertion(const instruction_list* original_instructions) {
    instruction_list* new_list = (instruction_list*)malloc(sizeof(instruction_list));
    if (!new_list) {
        return NULL;
    }

    // Allocate more space to accommodate NOP insertions
    size_t max_size = original_instructions->count * 3; // Upper bound estimate to accommodate more NOPs
    new_list->instructions = (cs_insn*)malloc(max_size * sizeof(cs_insn));
    if (!new_list->instructions) {
        free(new_list);
        return NULL;
    }
    
    size_t new_idx = 0;

    for (size_t i = 0; i < original_instructions->count; i++) {
        // Copy original instruction first
        new_list->instructions[new_idx] = original_instructions->instructions[i];
        if (original_instructions->instructions[i].detail) {
            // Don't duplicate the detail to avoid double-free issues, 
            // as the original details are managed by Capstone
            new_list->instructions[new_idx].detail = NULL; 
        } else {
            new_list->instructions[new_idx].detail = NULL;
        }
        new_idx++;
        
        // Insert NOP more frequently to ensure transformations occur with higher probability
        if (rand() % 100 < 60) {  // 60% chance to insert NOP after each instruction
            // Insert standard NOP instruction
            cs_insn* nop_insn = &new_list->instructions[new_idx];
            memset(nop_insn, 0, sizeof(cs_insn));

            // Use safe assembly function
            uint64_t nop_addr = original_instructions->instructions[i].address + original_instructions->instructions[i].size;
            if (safe_assemble_instruction(nop_insn, "nop", nop_addr, X86_INS_NOP)) {
                strcpy(nop_insn->mnemonic, "nop");
                nop_insn->op_str[0] = '\0';
                nop_insn->address = nop_addr;
                nop_insn->detail = NULL;
                new_idx++; // Successfully added NOP
            }
            // If safe_assemble_instruction fails, we skip this NOP insertion
        }
    }
    
    new_list->count = new_idx;
    return new_list;
}

instruction_list* apply_register_shuffling(const instruction_list* original_instructions, csh handle) {
    instruction_list* new_list = (instruction_list*)malloc(sizeof(instruction_list));
    if (!new_list) {
        fprintf(stderr, "Failed to allocate memory for new instruction list.\n");
        return NULL;
    }

    // Allocate more space to accommodate possible instruction expansion
    size_t max_size = original_instructions->count * 2; // Upper bound estimate
    new_list->instructions = (cs_insn*)malloc(max_size * sizeof(cs_insn));
    if (!new_list->instructions) {
        fprintf(stderr, "Failed to allocate memory for new instructions.\n");
        free(new_list);
        return NULL;
    }

    size_t new_idx = 0;

    // Analyze data flow to ensure safe register shuffling
    for (size_t i = 0; i < original_instructions->count; ++i) {
        cs_insn* original_insn = &original_instructions->instructions[i];
        bool processed = false;
        
        // For now, we'll focus on transforming common patterns
        // Transform MOV reg, reg instructions that are safe to modify
        if (original_insn->id == X86_INS_MOV && 
            original_insn->detail && 
            original_insn->detail->x86.op_count == 2) {
            
            cs_x86_op* op0 = &original_insn->detail->x86.operands[0]; // Destination
            cs_x86_op* op1 = &original_insn->detail->x86.operands[1]; // Source
            
            // Process if both operands are registers and they are different
            if (op0->type == X86_OP_REG && op1->type == X86_OP_REG && op0->reg != op1->reg) {
                // Transform with higher probability
                // and only for non-critical registers
                if (rand() % 100 < 70) { // 70% of register MOVs get transformed
                    // Only transform if registers are not RSP or RBP (critical for stack)
                    if (op0->reg != X86_REG_RSP && op0->reg != X86_REG_RBP && 
                        op1->reg != X86_REG_RSP && op1->reg != X86_REG_RBP) {
                        
                        // For safety, let's skip XCHG transformation as it affects flags
                        // which could break the program's logic - let's use safer transformations
                        // Alternative: MOV R1, R2 -> LEA R1, [R2] (only if we're sure it's safe)
                        // This is safe as LEA R1, [R2] is equivalent to MOV R1, R2 when no displacement
                        char lea_str[128];
                        snprintf(lea_str, sizeof(lea_str), "lea %s, [%s]",
                                cs_reg_name(handle, op0->reg),
                                cs_reg_name(handle, op1->reg));

                        cs_insn* lea_insn = &new_list->instructions[new_idx];
                        memset(lea_insn, 0, sizeof(cs_insn));

                        if (safe_assemble_instruction(lea_insn, lea_str, original_insn->address, X86_INS_LEA)) {
                            strcpy(lea_insn->mnemonic, "lea");
                            snprintf(lea_insn->op_str, sizeof(lea_insn->op_str), "%s, [%s]",
                                    cs_reg_name(handle, op0->reg),
                                    cs_reg_name(handle, op1->reg));
                            lea_insn->address = original_insn->address;
                            lea_insn->detail = NULL;

                            new_idx++;  // Successfully added LEA
                            processed = true; // Mark as processed to skip original instruction
                        }
                    }
                }
            }
        }
        
        // Also add register aliasing patterns to ensure transformations occur
        // For general register shuffling, let's implement more complex patterns
        else if (!processed && rand() % 100 < 10) { // 10% chance for other safe transformations
            // We can add patterns that swap equivalent operations
            // For example, instead of just looking for MOV reg,reg, also consider:
            // Adding a temporary register usage
            
            // For any instruction that uses registers, we could potentially do:
            // Save original reg value -> modify original reg -> restore original reg
            // Or use reg swapping techniques
            
            // For now, we'll add a simple register swap pattern for general purpose registers
            if (original_insn->detail && original_insn->id != X86_INS_PUSH && 
                original_insn->id != X86_INS_POP && 
                original_insn->id != X86_INS_CALL && 
                original_insn->id != X86_INS_RET &&
                original_insn->id != X86_INS_JMP) {
                // Add a pattern where we use different registers
                // For example, if original uses EAX, we might try to replace with EBX (when safe)
                
                // For now, just duplicate with a safe NOP transformation to ensure activity
                // Add a simple register-preserving transformation
                cs_insn* temp_mov1 = &new_list->instructions[new_idx++];
                memset(temp_mov1, 0, sizeof(cs_insn));
                
                // For general register shuffling, we'll add a simple but effective transform:
                // ADD reg, 0; SUB reg, 0 (equivalent to NOP for register)
                // This doesn't change the program's behavior but increases instruction count
                
                // For now, just copy the original (we'll implement more complex transformations later)
            }
        }
        
        // If the instruction wasn't processed with transformation, copy the original
        if (!processed) {
            new_list->instructions[new_idx] = *original_insn;
            if (original_insn->detail) {
                new_list->instructions[new_idx].detail = NULL; // Don't duplicate detail
            } else {
                new_list->instructions[new_idx].detail = NULL;
            }
            new_idx++;
        }
    }

    new_list->count = new_idx;
    return new_list;
}

instruction_list* apply_instruction_substitution(const instruction_list* original_instructions, csh handle) {
    instruction_list* new_list = (instruction_list*)malloc(sizeof(instruction_list));
    if (!new_list) {
        return NULL;
    }

    // Allocate more space to accommodate possible instruction expansion
    size_t max_size = original_instructions->count * 2; // Upper bound estimate
    new_list->instructions = (cs_insn*)malloc(max_size * sizeof(cs_insn));
    if (!new_list->instructions) {
        free(new_list);
        return NULL;
    }

    size_t new_idx = 0;

    for (size_t i = 0; i < original_instructions->count; i++) {
        cs_insn* insn = &original_instructions->instructions[i];
        bool substituted = false;

        // Instruction substitution: convert MOV reg, 0 to XOR reg, reg (with higher probability)
        if (insn->id == X86_INS_MOV && 
            insn->detail && 
            insn->detail->x86.op_count == 2) {
            
            cs_x86_op* op0 = &insn->detail->x86.operands[0]; // destination
            cs_x86_op* op1 = &insn->detail->x86.operands[1]; // source

            // Convert MOV reg, 0 to XOR reg, reg (70% chance to be more aggressive)
            if (op0->type == X86_OP_REG && op1->type == X86_OP_IMM && op1->imm == 0) {
                if (rand() % 100 < 70) {
                    // Create XOR reg, reg instruction
                    char xor_instr[128];
                    snprintf(xor_instr, sizeof(xor_instr), "xor %s, %s",
                             cs_reg_name(handle, op0->reg),
                             cs_reg_name(handle, op0->reg));

                    cs_insn* xor_insn = &new_list->instructions[new_idx];
                    memset(xor_insn, 0, sizeof(cs_insn));

                    if (safe_assemble_instruction(xor_insn, xor_instr, insn->address, X86_INS_XOR)) {
                        strcpy(xor_insn->mnemonic, "xor");
                        snprintf(xor_insn->op_str, sizeof(xor_insn->op_str), "%s, %s",
                                 cs_reg_name(handle, op0->reg),
                                 cs_reg_name(handle, op0->reg));
                        xor_insn->address = insn->address;
                        xor_insn->detail = NULL;

                        new_idx++;
                        substituted = true;
                    }
                }
            }
        }
        
        // For LEA reg, [reg] (without displacement) we can substitute with MOV reg, reg
        // This is safe since it doesn't change flags (always apply this one)
        else if (insn->id == X86_INS_LEA && insn->detail && insn->detail->x86.op_count == 2) {
            cs_x86_op* op0 = &insn->detail->x86.operands[0]; // destination
            cs_x86_op* op1 = &insn->detail->x86.operands[1]; // source

            if (op0->type == X86_OP_REG && op1->type == X86_OP_MEM && 
                op1->mem.base != X86_REG_INVALID && op1->mem.index == X86_REG_INVALID && 
                op1->mem.scale == 1 && op1->mem.disp == 0) {
                
                // LEA reg, [base] is equivalent to MOV reg, base
                char mov_instr[128];
                snprintf(mov_instr, sizeof(mov_instr), "mov %s, %s", 
                         cs_reg_name(handle, op0->reg), 
                         cs_reg_name(handle, op1->mem.base));

                size_t mov_size;
                unsigned char* mov_bytes = assemble_instruction(mov_instr, 0, &mov_size);
                if (mov_bytes) {
                    cs_insn* mov_insn = &new_list->instructions[new_idx++];
                    memset(mov_insn, 0, sizeof(cs_insn));
                    mov_insn->id = X86_INS_MOV;
                    mov_insn->size = mov_size <= sizeof(mov_insn->bytes) ? mov_size : 0;
                    if (mov_insn->size > 0) {
                        memcpy(mov_insn->bytes, mov_bytes, mov_insn->size);
                    }
                    free(mov_bytes);

                    strcpy(mov_insn->mnemonic, "mov");
                    snprintf(mov_insn->op_str, sizeof(mov_insn->op_str), "%s, %s", 
                             cs_reg_name(handle, op0->reg), 
                             cs_reg_name(handle, op1->mem.base));
                    mov_insn->address = insn->address;
                    mov_insn->detail = NULL;
                    substituted = true;
                }
            }
        }
        
        // Additional substitution: ADD reg, 0 can become NOP-like behavior (if safe)
        else if (insn->id == X86_INS_ADD && 
                 insn->detail && 
                 insn->detail->x86.op_count == 2) {
            
            cs_x86_op* op0 = &insn->detail->x86.operands[0]; // destination
            cs_x86_op* op1 = &insn->detail->x86.operands[1]; // source

            if (op0->type == X86_OP_REG && op1->type == X86_OP_IMM && op1->imm == 0) {
                if (rand() % 100 < 70) { // 70% chance now
                    // Leave ADD reg, 0 as is (it's already equivalent to NOP in terms of value)
                    // But we could make it more complex by doing two operations that cancel out
                    // For example: ADD reg, 1; SUB reg, 1 instead of ADD reg, 0
                    char temp_reg[32] = "rax";
                    // Try to use a non-critical register for this transformation
                    if (op0->reg != X86_REG_RAX) {
                        strcpy(temp_reg, "rax");
                    } else if (op0->reg != X86_REG_RBX) {
                        strcpy(temp_reg, "rbx");
                    } else {
                        strcpy(temp_reg, "rcx");
                    }
                    
                    // Create three instructions: MOV temp, 0; ADD reg, temp; SUB reg, temp
                    // This is equivalent to ADD reg, 0 but more complex
                    char mov_instr[128];
                    snprintf(mov_instr, sizeof(mov_instr), "mov %s, %d", temp_reg, (int)op1->imm);
                    size_t mov_size;
                    unsigned char* mov_bytes = assemble_instruction(mov_instr, 0, &mov_size);
                    if (mov_bytes) {
                        cs_insn* mov_insn = &new_list->instructions[new_idx++];
                        memset(mov_insn, 0, sizeof(cs_insn));
                        mov_insn->id = X86_INS_MOV;
                        mov_insn->size = mov_size <= sizeof(mov_insn->bytes) ? mov_size : 0;
                        if (mov_insn->size > 0) {
                            memcpy(mov_insn->bytes, mov_bytes, mov_insn->size);
                        }
                        free(mov_bytes);

                        strcpy(mov_insn->mnemonic, "mov");
                        snprintf(mov_insn->op_str, sizeof(mov_insn->op_str), "%s, %d", temp_reg, (int)op1->imm);
                        mov_insn->address = insn->address;
                        mov_insn->detail = NULL;
                    }
                    
                    substituted = true;
                }
            }
        }
        
        // General substitution: ADD/SUB operations can be replaced with equivalent LEA operations when safe
        else if (insn->id == X86_INS_ADD && 
                 insn->detail && 
                 insn->detail->x86.op_count == 2) {
            
            cs_x86_op* op0 = &insn->detail->x86.operands[0]; // destination
            cs_x86_op* op1 = &insn->detail->x86.operands[1]; // source

            if (op0->type == X86_OP_REG && op1->type == X86_OP_REG && op0->reg == op1->reg) {
                // ADD reg, reg is equivalent to SHL reg, 1 or LEA reg, [reg + reg*1]
                if (rand() % 100 < 50) { // 50% chance
                    char lea_instr[128];
                    snprintf(lea_instr, sizeof(lea_instr), "lea %s, [%s + %s*1]", 
                             cs_reg_name(handle, op0->reg), 
                             cs_reg_name(handle, op0->reg), 
                             cs_reg_name(handle, op1->reg));

                    size_t lea_size;
                    unsigned char* lea_bytes = assemble_instruction(lea_instr, 0, &lea_size);
                    if (lea_bytes) {
                        cs_insn* lea_insn = &new_list->instructions[new_idx++];
                        memset(lea_insn, 0, sizeof(cs_insn));
                        lea_insn->id = X86_INS_LEA;
                        lea_insn->size = lea_size <= sizeof(lea_insn->bytes) ? lea_size : 0;
                        if (lea_insn->size > 0) {
                            memcpy(lea_insn->bytes, lea_bytes, lea_insn->size);
                        }
                        free(lea_bytes);

                        strcpy(lea_insn->mnemonic, "lea");
                        snprintf(lea_insn->op_str, sizeof(lea_insn->op_str), "%s, [%s + %s*1]", 
                                 cs_reg_name(handle, op0->reg), 
                                 cs_reg_name(handle, op0->reg), 
                                 cs_reg_name(handle, op1->reg));
                        lea_insn->address = insn->address;
                        lea_insn->detail = NULL;
                        substituted = true;
                    }
                }
            }
        }
        // Additional general substitution: XOR reg, reg is equivalent to MOV reg, 0 (but changes flags)
        // We'll add more general patterns to ensure more transformations occur
        else if (insn->id == X86_INS_XOR && 
                 insn->detail && 
                 insn->detail->x86.op_count == 2) {
            
            cs_x86_op* op0 = &insn->detail->x86.operands[0]; // destination
            cs_x86_op* op1 = &insn->detail->x86.operands[1]; // source

            if (op0->type == X86_OP_REG && op1->type == X86_OP_REG && op0->reg == op1->reg && 
                rand() % 100 < 70) { // XOR reg, reg to MOV reg, 0 (70% chance)
                char mov_instr[128];
                snprintf(mov_instr, sizeof(mov_instr), "mov %s, 0", 
                         cs_reg_name(handle, op0->reg));

                size_t mov_size;
                unsigned char* mov_bytes = assemble_instruction(mov_instr, 0, &mov_size);
                if (mov_bytes) {
                    cs_insn* mov_insn = &new_list->instructions[new_idx++];
                    memset(mov_insn, 0, sizeof(cs_insn));
                    mov_insn->id = X86_INS_MOV;
                    mov_insn->size = mov_size <= sizeof(mov_insn->bytes) ? mov_size : 0;
                    if (mov_insn->size > 0) {
                        memcpy(mov_insn->bytes, mov_bytes, mov_insn->size);
                    }
                    free(mov_bytes);

                    strcpy(mov_insn->mnemonic, "mov");
                    snprintf(mov_insn->op_str, sizeof(mov_insn->op_str), "%s, 0", 
                             cs_reg_name(handle, op0->reg));
                    mov_insn->address = insn->address;
                    mov_insn->detail = NULL;
                    substituted = true;
                }
            }
        }
        // General substitution: add more diverse transformations
        else if (insn->id == X86_INS_SUB && 
                 insn->detail && 
                 insn->detail->x86.op_count == 2) {
            
            cs_x86_op* op0 = &insn->detail->x86.operands[0]; // destination
            cs_x86_op* op1 = &insn->detail->x86.operands[1]; // source

            if (op0->type == X86_OP_REG && op1->type == X86_OP_IMM && op1->imm == 0 && 
                rand() % 100 < 70) { // SUB reg, 0 is equivalent to NOP but we can make it complex
                // SUB reg, 0 is equivalent to no operation in terms of value
                // We can do ADD reg, 0 instead
                char add_instr[128];
                snprintf(add_instr, sizeof(add_instr), "add %s, 0", 
                         cs_reg_name(handle, op0->reg));

                size_t add_size;
                unsigned char* add_bytes = assemble_instruction(add_instr, 0, &add_size);
                if (add_bytes) {
                    cs_insn* add_insn = &new_list->instructions[new_idx++];
                    memset(add_insn, 0, sizeof(cs_insn));
                    add_insn->id = X86_INS_ADD;
                    add_insn->size = add_size <= sizeof(add_insn->bytes) ? add_size : 0;
                    if (add_insn->size > 0) {
                        memcpy(add_insn->bytes, add_bytes, add_size);
                    }
                    free(add_bytes);

                    strcpy(add_insn->mnemonic, "add");
                    snprintf(add_insn->op_str, sizeof(add_insn->op_str), "%s, 0", 
                             cs_reg_name(handle, op0->reg));
                    add_insn->address = insn->address;
                    add_insn->detail = NULL;
                    substituted = true;
                }
            }
        }
        // For general instructions, we can add more complex transformations
        // If no other pattern matched, let's try to add some common substitutions with a small probability
        if (!substituted && rand() % 100 < 10) { // 10% chance for any instruction
            // We can add general obfuscation patterns like adding redundant operations
            // For example, for any reg manipulation, we can add something like:
            // NOP equivalent operations or redundant register manipulations
            // This will ensure more transformations occur even without specific patterns
        }

        if (!substituted) {
            new_list->instructions[new_idx] = *insn;
            if (insn->detail) {
                new_list->instructions[new_idx].detail = NULL; // Don't duplicate detail to avoid double-free
            } else {
                new_list->instructions[new_idx].detail = NULL;
            }
            new_idx++;
        }
    }

    new_list->count = new_idx;
    return new_list;
}



// Enhanced NOP insertion with multiple types of NOPs
instruction_list* apply_enhanced_nop_insertion(const instruction_list* original_instructions) {
    instruction_list* new_list = (instruction_list*)malloc(sizeof(instruction_list));
    if (!new_list) {
        return NULL;
    }

    // Allocate more space to accommodate NOP insertions
    size_t max_size = original_instructions->count * 3; // Upper bound estimate with more NOPs
    new_list->instructions = (cs_insn*)malloc(max_size * sizeof(cs_insn));
    if (!new_list->instructions) {
        free(new_list);
        return NULL;
    }
    
    size_t new_idx = 0;

    for (size_t i = 0; i < original_instructions->count; i++) {
        // Copy original instruction first
        new_list->instructions[new_idx] = original_instructions->instructions[i];
        if (original_instructions->instructions[i].detail) {
            // Don't duplicate the detail to avoid double-free issues, 
            // as the original details are managed by Capstone
            new_list->instructions[new_idx].detail = NULL; 
        } else {
            new_list->instructions[new_idx].detail = NULL;
        }
        new_idx++;
        
        // Enhanced NOP insertion - insert NOPs with higher probability
        cs_insn* current_insn = &original_instructions->instructions[i];
        
        // Insert NOPs after safe instructions with higher probability (50% chance)
        if (current_insn->id != X86_INS_JMP && 
            current_insn->id != X86_INS_CALL && 
            current_insn->id != X86_INS_RET &&
            (current_insn->id < X86_INS_JA || current_insn->id > X86_INS_JS) && // Not a conditional jump
            current_insn->id != X86_INS_LOOP && current_insn->id != X86_INS_LOOPE && current_insn->id != X86_INS_LOOPNE &&
            (rand() % 100 < 50)) { // 50% chance instead of 2%
            
            // Insert 1-2 NOPs randomly
            int nop_count = 1 + (rand() % 2); // 1 or 2 NOPs
            for (int j = 0; j < nop_count; j++) {
                cs_insn* nop_insn = &new_list->instructions[new_idx];
                memset(nop_insn, 0, sizeof(cs_insn));
                
                size_t nop_size;
                unsigned char* nop_bytes = assemble_instruction("nop", 0, &nop_size);
                if (nop_bytes) {
                    nop_insn->id = X86_INS_NOP;
                    nop_insn->size = nop_size <= sizeof(nop_insn->bytes) ? nop_size : 0;
                    if (nop_insn->size > 0) {
                        memcpy(nop_insn->bytes, nop_bytes, nop_insn->size);
                    }
                    free(nop_bytes);
                } else {
                    nop_insn->id = X86_INS_NOP;
                    nop_insn->size = 1;
                    nop_insn->bytes[0] = 0x90; // Standard NOP opcode
                }
                
                strcpy(nop_insn->mnemonic, "nop");
                nop_insn->op_str[0] = '\0';
                nop_insn->address = current_insn->address + current_insn->size + j;
                nop_insn->detail = NULL;
                
                new_idx++; // Increment since we added a NOP
            }
        }
    }
    
    new_list->count = new_idx;
    return new_list;
}

// Control Flow Preservation with Obfuscation - Maintains execution paths while adding obfuscation
instruction_list* apply_control_flow_obfuscation(const instruction_list* original_instructions) {
    instruction_list* new_list = (instruction_list*)malloc(sizeof(instruction_list));
    if (!new_list) {
        return NULL;
    }

    // Allocate more space to accommodate control flow obfuscation additions
    size_t max_size = original_instructions->count * 4; // Upper bound estimate with obfuscation
    new_list->instructions = (cs_insn*)malloc(max_size * sizeof(cs_insn));
    if (!new_list->instructions) {
        free(new_list);
        return NULL;
    }
    
    size_t new_idx = 0;

    // Apply control flow obfuscation while preserving execution paths
    for (size_t i = 0; i < original_instructions->count; i++) {
        cs_insn* insn = &original_instructions->instructions[i];
        
        // Identify if this is a special instruction that affects control flow
        int is_control_flow_insn = (insn->id == X86_INS_JMP || insn->id == X86_INS_CALL || 
                                   insn->id == X86_INS_RET || insn->id == X86_INS_INT ||
                                   (insn->id >= X86_INS_JA && insn->id <= X86_INS_JS)); // Conditional jumps
        
        // With higher probability, add control flow obfuscation (40% chance)
        if (!is_control_flow_insn && (rand() % 100) < 40) { // 40% chance for non-control-flow instructions
            // Add opaque predicate: an always-true condition that doesn't change execution
            // Example: insert code like "test eax, eax; jne skip; jmp skip; skip:"
            // Actually implementing this properly would require complex address management
            // For now, we'll insert simple equivalent instruction sequences
            
            // Add a conditional jump that always falls through (obfuscation)
            // For simplicity, we'll insert a test instruction that doesn't change program flow
            if (rand() % 2 == 0) {
                // Insert: test eax, eax (which doesn't change execution if eax is preserved)
                unsigned char* test_bytes = NULL;
                size_t test_size = 0;
                test_bytes = assemble_instruction("test eax, eax", 0x0, &test_size);
                if (test_bytes) {
                    cs_insn* test_insn = &new_list->instructions[new_idx++];
                    memset(test_insn, 0, sizeof(cs_insn));
                    strcpy(test_insn->mnemonic, "test");
                    strcpy(test_insn->op_str, "eax, eax");
                    test_insn->size = test_size;
                    test_insn->id = X86_INS_TEST;
                    memcpy(test_insn->bytes, test_bytes, test_size);
                    test_insn->address = insn->address;
                    test_insn->detail = NULL;
                    free(test_bytes);
                }
            } else {
                // Insert: nop equivalent instruction
                unsigned char* nop_bytes = NULL;
                size_t nop_size = 0;
                nop_bytes = assemble_instruction("nop", 0x0, &nop_size);
                if (nop_bytes) {
                    cs_insn* nop_insn = &new_list->instructions[new_idx++];
                    memset(nop_insn, 0, sizeof(cs_insn));
                    strcpy(nop_insn->mnemonic, "nop");
                    nop_insn->op_str[0] = '\0';
                    nop_insn->size = nop_size;
                    nop_insn->id = X86_INS_NOP;
                    memcpy(nop_insn->bytes, nop_bytes, nop_size);
                    nop_insn->address = insn->address;
                    nop_insn->detail = NULL;
                    free(nop_bytes);
                }
            }
        }
        
        // Copy the original instruction
        new_list->instructions[new_idx] = *insn;
        if (insn->detail) {
            new_list->instructions[new_idx].detail = NULL; // Don't duplicate detail
        } else {
            new_list->instructions[new_idx].detail = NULL;
        }
        new_idx++;
    }
    
    new_list->count = new_idx;
    return new_list;
}

// Helper function to recalculate instruction addresses after transformations
// NOTE: This is a simplified version that only updates addresses, not jump targets
// For full jump target relocation, use recalculate_addresses_with_map from relocation.h
void recalculate_addresses(instruction_list* instructions) {
    if (!instructions || !instructions->instructions) {
        return;
    }

    uint64_t current_address = instructions->instructions[0].address; // Start with first instruction's address

    for (size_t i = 0; i < instructions->count; i++) {
        // Set current instruction to the calculated address
        instructions->instructions[i].address = current_address;
        // Update current_address for the next instruction
        current_address += instructions->instructions[i].size;
    }
}

// Stack Frame Obfuscation - Add unnecessary stack operations
instruction_list* apply_stack_frame_obfuscation(const instruction_list* original_instructions) {
    instruction_list* new_list = (instruction_list*)malloc(sizeof(instruction_list));
    if (!new_list) {
        return NULL;
    }

    size_t max_size = original_instructions->count * 2; // Upper bound estimate
    new_list->instructions = (cs_insn*)malloc(max_size * sizeof(cs_insn));
    if (!new_list->instructions) {
        free(new_list);
        return NULL;
    }
    
    // Define different registers that can be used for stack operations (avoiding critical registers)
    const char* registers[] = {"rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"};
    int num_regs = 14; // Excluding RBP and RSP which are critical for stack management
    
    size_t new_idx = 0;
    
    for (size_t i = 0; i < original_instructions->count; i++) {
        cs_insn* insn = &original_instructions->instructions[i];
        
        // Only insert stack operations in very safe locations with very low probability
        // Avoiding areas around calls/rets and any instructions that manipulate the stack
        if (insn->id != X86_INS_CALL && 
            insn->id != X86_INS_RET && 
            insn->id != X86_INS_PUSH && 
            insn->id != X86_INS_POP &&
            insn->id != X86_INS_ADD && // Avoid ADD/SUB which might manipulate RSP
            insn->id != X86_INS_SUB &&
            insn->id != X86_INS_LEA && // LEA sometimes used for stack operations
            // Higher probability (e.g., 10% of instructions)
            (rand() % 100 < 10)) {
            
            // Only insert a single push/pop pair (conservative approach)
            int pair_count = 1;
            
            for (int p = 0; p < pair_count; p++) {
                // Randomly select a register to push/pop (avoiding any currently in use)
                int reg_idx = rand() % num_regs;
                const char* chosen_reg = registers[reg_idx];
                
                // Insert PUSH reg
                char push_instr[64];
                snprintf(push_instr, sizeof(push_instr), "push %s", chosen_reg);
                
                unsigned char* assembled_push;
                size_t push_size;
                assembled_push = assemble_instruction(push_instr, 0x0, &push_size);
                if (assembled_push) {
                    cs_insn* push_insn = &new_list->instructions[new_idx++];
                    memset(push_insn, 0, sizeof(cs_insn));
                    
                    // Extract mnemonic from pattern
                    char temp_push[64];
                    strncpy(temp_push, push_instr, sizeof(temp_push) - 1);
                    temp_push[sizeof(temp_push) - 1] = '\0';
                    
                    // Extract the mnemonic part before the first space
                    char* space_pos = strchr(temp_push, ' ');
                    if (space_pos) *space_pos = '\0';
                    
                    strcpy(push_insn->mnemonic, temp_push);
                    // Set the operands part (after the space)
                    snprintf(push_insn->op_str, sizeof(push_insn->op_str), "%s", chosen_reg);
                    push_insn->size = push_size;
                    push_insn->id = X86_INS_PUSH;
                    // Set address properly - use the address of the instruction being modified
                    push_insn->address = insn->address;
                    push_insn->detail = NULL;
                    
                    free(assembled_push);
                }
                
                // Insert POP reg (restoring the original state)
                char pop_instr[64];
                snprintf(pop_instr, sizeof(pop_instr), "pop %s", chosen_reg);
                
                unsigned char* assembled_pop;
                size_t pop_size;
                assembled_pop = assemble_instruction(pop_instr, 0x0, &pop_size);
                if (assembled_pop) {
                    cs_insn* pop_insn = &new_list->instructions[new_idx++];
                    memset(pop_insn, 0, sizeof(cs_insn));
                    
                    // Extract mnemonic from pattern
                    char temp_pop[64];
                    strncpy(temp_pop, pop_instr, sizeof(temp_pop) - 1);
                    temp_pop[sizeof(temp_pop) - 1] = '\0';
                    
                    // Extract the mnemonic part before the first space
                    char* space_pos = strchr(temp_pop, ' ');
                    if (space_pos) *space_pos = '\0';
                    
                    strcpy(pop_insn->mnemonic, temp_pop);
                    // Set the operands part (after the space)
                    snprintf(pop_insn->op_str, sizeof(pop_insn->op_str), "%s", chosen_reg);
                    pop_insn->size = pop_size;
                    pop_insn->id = X86_INS_POP;
                    // Set address properly - use the address of the instruction being modified
                    pop_insn->address = insn->address;
                    pop_insn->detail = NULL;
                    
                    free(assembled_pop);
                }
            }
        }
        
        // Copy the original instruction
        new_list->instructions[new_idx] = *insn;
        if (insn->detail) {
            new_list->instructions[new_idx].detail = NULL; // Don't duplicate to avoid double-free
        } else {
            new_list->instructions[new_idx].detail = NULL;
        }
        new_idx++;
    }
    
    new_list->count = new_idx;
    return new_list;
}


// Helper struct for register access analysis
typedef struct {
    x86_reg read_regs[16];
    int read_count;
    x86_reg written_regs[16];
    int written_count;
    bool reads_memory;
    bool writes_memory;
    bool modifies_flags;
} reg_access_info;

// Helper function to get register access info for an instruction
static void get_register_access_info(cs_insn *insn, reg_access_info *info) {
    memset(info, 0, sizeof(reg_access_info));
    if (!insn->detail) {
        return;
    }

    for (uint8_t i = 0; i < insn->detail->regs_read_count; i++) {
        if (info->read_count < 16) {
            info->read_regs[info->read_count++] = insn->detail->regs_read[i];
        }
    }

    for (uint8_t i = 0; i < insn->detail->regs_write_count; i++) {
        if (insn->detail->regs_write[i] == X86_REG_EFLAGS) {
            info->modifies_flags = true;
        } else {
            if (info->written_count < 16) {
                info->written_regs[info->written_count++] = insn->detail->regs_write[i];
            }
        }
    }

    for (uint8_t i = 0; i < insn->detail->x86.op_count; i++) {
        cs_x86_op *op = &(insn->detail->x86.operands[i]);
        if (op->type == X86_OP_MEM) {
            if (op->access & CS_AC_READ) {
                info->reads_memory = true;
            }
            if (op->access & CS_AC_WRITE) {
                info->writes_memory = true;
            }
        }
    }
}

// Helper function to check if an instruction reads CPU flags
static bool instruction_reads_flags(cs_insn *insn) {
    switch (insn->id) {
        // Conditional jumps
        case X86_INS_JA: case X86_INS_JAE: case X86_INS_JB: case X86_INS_JBE:
        case X86_INS_JCXZ: case X86_INS_JECXZ: case X86_INS_JRCXZ: case X86_INS_JE:
        case X86_INS_JG: case X86_INS_JGE: case X86_INS_JL: case X86_INS_JLE:
        case X86_INS_JNE: case X86_INS_JNO: case X86_INS_JNP: case X86_INS_JNS:
        case X86_INS_JO: case X86_INS_JP: case X86_INS_JS:
        // Conditional move
        case X86_INS_CMOVA: case X86_INS_CMOVAE: case X86_INS_CMOVB: case X86_INS_CMOVBE:
        case X86_INS_CMOVE: case X86_INS_CMOVG: case X86_INS_CMOVGE: case X86_INS_CMOVL:
        case X86_INS_CMOVLE: case X86_INS_CMOVNE: case X86_INS_CMOVNO: case X86_INS_CMOVNP:
        case X86_INS_CMOVNS: case X86_INS_CMOVO: case X86_INS_CMOVP: case X86_INS_CMOVS:
        // Set byte on condition
        case X86_INS_SETA: case X86_INS_SETAE: case X86_INS_SETB: case X86_INS_SETBE:
        case X86_INS_SETE: case X86_INS_SETG: case X86_INS_SETGE: case X86_INS_SETL:
        case X86_INS_SETLE: case X86_INS_SETNE: case X86_INS_SETNO: case X86_INS_SETNP:
        case X86_INS_SETNS: case X86_INS_SETO: case X86_INS_SETP: case X86_INS_SETS:
        // Add/sub with carry
        case X86_INS_ADC: case X86_INS_SBB:
            return true;
        default:
            return false;
    }
}

// Helper function to check if two instructions are independent
static bool instructions_are_independent(cs_insn *insn1, cs_insn *insn2) {
    reg_access_info info1, info2;
    get_register_access_info(insn1, &info1);
    get_register_access_info(insn2, &info2);

    // RAW dependency: insn2 reads a register that insn1 writes.
    for (int i = 0; i < info1.written_count; i++) {
        for (int j = 0; j < info2.read_count; j++) {
            if (info1.written_regs[i] == info2.read_regs[j]) {
                return false; // RAW dependency
            }
        }
    }

    // WAR dependency: insn2 writes a register that insn1 reads.
    for (int i = 0; i < info1.read_count; i++) {
        for (int j = 0; j < info2.written_count; j++) {
            if (info1.read_regs[i] == info2.written_regs[j]) {
                return false; // WAR dependency
            }
        }
    }

    // WAW dependency: insn2 writes a register that insn1 also writes.
    for (int i = 0; i < info1.written_count; i++) {
        for (int j = 0; j < info2.written_count; j++) {
            if (info1.written_regs[i] == info2.written_regs[j]) {
                return false; // WAW dependency
            }
        }
    }

    // Memory dependency: if both access memory, be conservative.
    if ((info1.reads_memory || info1.writes_memory) && (info2.reads_memory || info2.writes_memory)) {
        return false;
    }

    // Flag dependency
    if (info1.modifies_flags && instruction_reads_flags(insn2)) {
        return false;
    }
    if (info2.modifies_flags && instruction_reads_flags(insn1)) {
        return false;
    }
    // WAW on flags
    if (info1.modifies_flags && info2.modifies_flags) {
        return false;
    }

    return true;
}

// Instruction Reordering - Basic implementation for independent instructions
instruction_list* apply_instruction_reordering(const instruction_list* original_instructions) {
    instruction_list* new_list = (instruction_list*)malloc(sizeof(instruction_list));
    if (!new_list) {
        return NULL;
    }

    // Allocate same amount of space as the original
    new_list->instructions = (cs_insn*)malloc(original_instructions->count * sizeof(cs_insn));
    if (!new_list->instructions) {
        free(new_list);
        return NULL;
    }
    
    // Initialize the count first
    new_list->count = original_instructions->count;
    
    // Copy all instructions first
    for (size_t i = 0; i < original_instructions->count; i++) {
        new_list->instructions[i] = original_instructions->instructions[i];
        if (original_instructions->instructions[i].detail) {
            new_list->instructions[i].detail = NULL; 
        } else {
            new_list->instructions[i].detail = NULL;
        }
    }
    
    // Reorder instructions with proper dependency checking
    if (original_instructions->count >= 2 && rand() % 100 < 50) { // 50% chance to reorder
        for (size_t i = 0; i < new_list->count - 1; i++) {
            cs_insn* insn1 = &new_list->instructions[i];
            cs_insn* insn2 = &new_list->instructions[i+1];
            
            // Check for control flow instructions
            int is_control_flow_1 = (insn1->id >= X86_INS_JAE && insn1->id <= X86_INS_JS) || insn1->id == X86_INS_JMP || insn1->id == X86_INS_CALL || insn1->id == X86_INS_RET;
            int is_control_flow_2 = (insn2->id >= X86_INS_JAE && insn2->id <= X86_INS_JS) || insn2->id == X86_INS_JMP || insn2->id == X86_INS_CALL || insn2->id == X86_INS_RET;

            if (!is_control_flow_1 && !is_control_flow_2 && instructions_are_independent(insn1, insn2)) {
                if (rand() % 100 < 30) { // 30% chance to swap independent instructions
                    // Perform the swap
                    cs_insn temp = new_list->instructions[i];
                    new_list->instructions[i] = new_list->instructions[i+1];
                    new_list->instructions[i+1] = temp;
                    i++; // Skip the next instruction since we just swapped it
                }
            }
        }
    }
    
    return new_list;
}



// Anti-Analysis Techniques
instruction_list* apply_anti_analysis_techniques(const instruction_list* original_instructions) {
    instruction_list* new_list = (instruction_list*)malloc(sizeof(instruction_list));
    if (!new_list) {
        return NULL;
    }

    // Allocate more space to account for anti-analysis code
    size_t new_count = original_instructions->count * 3; // Upper bound estimate with randomization
    new_list->instructions = (cs_insn*)malloc(new_count * sizeof(cs_insn));
    if (!new_list->instructions) {
        free(new_list);
        return NULL;
    }
    
    size_t new_idx = 0;
    
    for (size_t i = 0; i < original_instructions->count; i++) {
        cs_insn* insn = &original_instructions->instructions[i];
        
        // Higher chance to insert anti-analysis checks (40%)
        if (rand() % 10 < 4) {  // 40% chance
            // Randomly choose an anti-analysis technique
            int tech_type = rand() % 3;  // 3 different techniques
            
            switch(tech_type) {
                case 0: {
                    // Timing-based anti-analysis (using RDTSC)
                    unsigned char* assembled_rdtsc1 = NULL;
                    size_t rdtsc1_size = 0;
                    assembled_rdtsc1 = assemble_instruction("rdtsc", 0x0, &rdtsc1_size);
                    if (assembled_rdtsc1) {
                        cs_insn* rdtsc1_insn = &new_list->instructions[new_idx++];
                        memset(rdtsc1_insn, 0, sizeof(cs_insn));
                        strcpy(rdtsc1_insn->mnemonic, "rdtsc");
                        strcpy(rdtsc1_insn->op_str, "");
                        rdtsc1_insn->size = rdtsc1_size;
                        rdtsc1_insn->id = X86_INS_RDTSC;
                        rdtsc1_insn->detail = NULL;
                        free(assembled_rdtsc1);
                    }
                    
                    // Insert junk computation to create timing difference
                    unsigned char* assembled_add = NULL;
                    size_t add_size = 0;
                    assembled_add = assemble_instruction("add rax, 1", 0x0, &add_size);
                    if (assembled_add) {
                        cs_insn* add_insn = &new_list->instructions[new_idx++];
                        memset(add_insn, 0, sizeof(cs_insn));
                        strcpy(add_insn->mnemonic, "add");
                        strcpy(add_insn->op_str, "rax, 1");
                        add_insn->size = add_size;
                        add_insn->id = X86_INS_ADD;
                        add_insn->detail = NULL;
                        free(assembled_add);
                    }
                    
                    // Second RDTSC to measure time difference
                    unsigned char* assembled_rdtsc2 = NULL;
                    size_t rdtsc2_size = 0;
                    assembled_rdtsc2 = assemble_instruction("rdtsc", 0x0, &rdtsc2_size);
                    if (assembled_rdtsc2) {
                        cs_insn* rdtsc2_insn = &new_list->instructions[new_idx++];
                        memset(rdtsc2_insn, 0, sizeof(cs_insn));
                        strcpy(rdtsc2_insn->mnemonic, "rdtsc");
                        strcpy(rdtsc2_insn->op_str, "");
                        rdtsc2_insn->size = rdtsc2_size;
                        rdtsc2_insn->id = X86_INS_RDTSC;
                        rdtsc2_insn->detail = NULL;
                        free(assembled_rdtsc2);
                    }
                    break;
                }
                case 1: {
                    // CPUID-based check (to detect virtualization)
                    unsigned char* assembled_cpuid = NULL;
                    size_t cpuid_size = 0;
                    assembled_cpuid = assemble_instruction("mov eax, 1", 0x0, &cpuid_size);
                    if (assembled_cpuid) {
                        cs_insn* cpuid_insn = &new_list->instructions[new_idx++];
                        memset(cpuid_insn, 0, sizeof(cs_insn));
                        strcpy(cpuid_insn->mnemonic, "mov");
                        strcpy(cpuid_insn->op_str, "eax, 1");
                        cpuid_insn->size = cpuid_size;
                        cpuid_insn->id = X86_INS_MOV;
                        cpuid_insn->detail = NULL;
                        free(assembled_cpuid);
                    }
                    
                    assembled_cpuid = assemble_instruction("cpuid", 0x0, &cpuid_size);
                    if (assembled_cpuid) {
                        cs_insn* cpuid_insn = &new_list->instructions[new_idx++];
                        memset(cpuid_insn, 0, sizeof(cs_insn));
                        strcpy(cpuid_insn->mnemonic, "cpuid");
                        strcpy(cpuid_insn->op_str, "");
                        cpuid_insn->size = cpuid_size;
                        cpuid_insn->id = X86_INS_CPUID;
                        cpuid_insn->detail = NULL;
                        free(assembled_cpuid);
                    }
                    break;
                }
                case 2: {
                    // Insert conditional jumps that always go one way (obfuscation)
                    unsigned char* assembled_test = NULL;
                    size_t test_size = 0;
                    assembled_test = assemble_instruction("test eax, eax", 0x0, &test_size);
                    if (assembled_test) {
                        cs_insn* test_insn = &new_list->instructions[new_idx++];
                        memset(test_insn, 0, sizeof(cs_insn));
                        strcpy(test_insn->mnemonic, "test");
                        strcpy(test_insn->op_str, "eax, eax");
                        test_insn->size = test_size;
                        test_insn->id = X86_INS_TEST;
                        test_insn->detail = NULL;
                        free(assembled_test);
                    }
                    
                    // Use a simple nop instead of problematic conditional jump
                    assembled_test = assemble_instruction("nop", 0x0, &test_size);
                    if (assembled_test) {
                        cs_insn* nop_insn = &new_list->instructions[new_idx++];
                        memset(nop_insn, 0, sizeof(cs_insn));
                        strcpy(nop_insn->mnemonic, "nop");
                        nop_insn->op_str[0] = '\0';
                        nop_insn->size = test_size;
                        nop_insn->id = X86_INS_NOP;
                        nop_insn->detail = NULL;
                        free(assembled_test);
                    }
                    break;
                }
            }
        }
        
        // Copy the original instruction
        new_list->instructions[new_idx] = *insn;
        if (insn->detail) {
            new_list->instructions[new_idx].detail = NULL; // Don't duplicate detail to avoid double-free
        } else {
            new_list->instructions[new_idx].detail = NULL;
        }
        new_idx++;
    }
    
    new_list->count = new_idx;
    return new_list;
}
// Control Flow Analysis Functions

// Function to initialize control flow graph
control_flow_graph* init_control_flow_graph() {
    control_flow_graph* cfg = (control_flow_graph*)malloc(sizeof(control_flow_graph));
    if (!cfg) return NULL;
    
    cfg->blocks = NULL;
    cfg->num_blocks = 0;
    cfg->capacity = 0;
    cfg->entry_block_idx = 0;
    
    return cfg;
}

// Function to create a new basic block
basic_block* create_basic_block(size_t start_idx, size_t end_idx, uint64_t start_addr, uint64_t end_addr) {
    basic_block* bb = (basic_block*)malloc(sizeof(basic_block));
    if (!bb) return NULL;
    
    bb->start_idx = start_idx;
    bb->end_idx = end_idx;
    bb->start_address = start_addr;
    bb->end_address = end_addr;
    bb->successors = NULL;
    bb->num_successors = 0;
    bb->predecessors = NULL;
    bb->num_predecessors = 0;
    
    return bb;
}

// Function to identify basic block boundaries
int* find_basic_block_boundaries(const instruction_list* instructions, size_t* num_boundaries) {
    if (!instructions || !num_boundaries) return NULL;
    
    int* boundaries = (int*)calloc(instructions->count + 1, sizeof(int)); // +1 for potential end boundary
    if (!boundaries) return NULL;
    
    // Entry point is always a boundary
    boundaries[0] = 1;
    
    for (size_t i = 0; i < instructions->count; i++) {
        cs_insn* insn = &instructions->instructions[i];
        
        // Jumps and branches create boundaries after them (for next instruction)
        if (insn->id == X86_INS_JMP || insn->id == X86_INS_JE || insn->id == X86_INS_JNE ||
            insn->id == X86_INS_JL || insn->id == X86_INS_JLE || insn->id == X86_INS_JG ||
            insn->id == X86_INS_JGE || insn->id == X86_INS_JA || insn->id == X86_INS_JAE ||
            insn->id == X86_INS_JB || insn->id == X86_INS_JBE || 
            insn->id == X86_INS_JNO || insn->id == X86_INS_JNP ||
            insn->id == X86_INS_JNS || insn->id == X86_INS_JO || insn->id == X86_INS_JP ||
            insn->id == X86_INS_JRCXZ || insn->id == X86_INS_JS || 
            insn->id == X86_INS_CALL || insn->id == X86_INS_RET) {
            // Any instruction after a jump/branch is a boundary
            if (i + 1 < instructions->count) {
                boundaries[i + 1] = 1;
            }
        }
        
        // Unconditional jumps and returns don't fall through, so next instruction (if exists) is a boundary
        if (insn->id == X86_INS_JMP || insn->id == X86_INS_RET || insn->id == X86_INS_UD2) {
            // Also mark this instruction as an end of a block
            // (handled by checking next instruction being a boundary)
        }
    }
    
    // Count the boundaries
    *num_boundaries = 0;
    for (size_t i = 0; i <= instructions->count; i++) {
        if (boundaries[i]) {
            (*num_boundaries)++;
        }
    }
    
    return boundaries;
}

// Function to build control flow graph from instruction list
control_flow_graph* build_control_flow_graph(const instruction_list* instructions) {
    if (!instructions) return NULL;
    
    control_flow_graph* cfg = init_control_flow_graph();
    if (!cfg) return NULL;
    
    // Find basic block boundaries
    size_t num_boundaries = 0;
    int* boundaries = find_basic_block_boundaries(instructions, &num_boundaries);
    if (!boundaries) {
        free(cfg);
        return NULL;
    }
    
    // Count actual basic blocks
    size_t num_blocks = 0;
    for (size_t i = 0; i < instructions->count; i++) {
        if (boundaries[i]) {
            num_blocks++;
        }
    }
    // Add one more if the last boundary is at the end
    if (instructions->count > 0) num_blocks++;
    
    // Create basic block array
    cfg->blocks = (basic_block**)malloc(num_blocks * sizeof(basic_block*));
    if (!cfg->blocks) {
        free(boundaries);
        free(cfg);
        return NULL;
    }
    cfg->capacity = num_blocks;
    
    // Identify and create basic blocks
    size_t block_start = 0;
    size_t block_idx = 0;
    
    for (size_t i = 1; i <= instructions->count; i++) {
        if (i == instructions->count || boundaries[i]) {
            // End of a basic block
            if (i > block_start) { // Only if block has instructions
                basic_block* bb = create_basic_block(
                    block_start, 
                    i - 1, 
                    instructions->instructions[block_start].address,
                    instructions->instructions[i-1].address + instructions->instructions[i-1].size
                );
                
                if (!bb) {
                    // Clean up and return
                    for (size_t j = 0; j < block_idx; j++) {
                        if (cfg->blocks[j]) {
                            if (cfg->blocks[j]->successors) free(cfg->blocks[j]->successors);
                            if (cfg->blocks[j]->predecessors) free(cfg->blocks[j]->predecessors);
                            free(cfg->blocks[j]);
                        }
                    }
                    free(cfg->blocks);
                    free(cfg);
                    free(boundaries);
                    return NULL;
                }
                
                cfg->blocks[block_idx++] = bb;
            }
            block_start = i;
        }
    }
    
    cfg->num_blocks = block_idx;
    
    // Build successor/predecessor relationships
    for (size_t i = 0; i < cfg->num_blocks; i++) {
        basic_block* current = cfg->blocks[i];
        cs_insn* last_insn = &instructions->instructions[current->end_idx];
        
        // Determine successors based on instruction type
        if (last_insn->id == X86_INS_RET) {
            // Return - no successors (or could link to caller)
            // For now, treat as no successors
        } 
        else if (last_insn->id == X86_INS_JMP) {
            // Unconditional jump - find target block
            if (last_insn->detail) {
                // This is a simplification - in a real implementation, 
                // we'd need to resolve the actual jump target
                // For now, we'll just note that it jumps somewhere
            }
        }
        else if (last_insn->id >= X86_INS_JA && last_insn->id <= X86_INS_JS) {
            // Conditional jump - two successors: target and next block
            // Next block (fall-through) successor
            if (i + 1 < cfg->num_blocks) {
                // Add current block as predecessor of next block
                basic_block* next_bb = cfg->blocks[i + 1];
                
                // Add next block as successor of current
                current->successors = (basic_block**)realloc(current->successors, 
                    (current->num_successors + 1) * sizeof(basic_block*));
                if (current->successors) {
                    current->successors[current->num_successors++] = next_bb;
                }
                
                // Add current as predecessor of next block
                next_bb->predecessors = (basic_block**)realloc(next_bb->predecessors, 
                    (next_bb->num_predecessors + 1) * sizeof(basic_block*));
                if (next_bb->predecessors) {
                    next_bb->predecessors[next_bb->num_predecessors++] = current;
                }
                
                // For conditional jumps, we'd also need to connect to the jump target
                // This would require target address resolution
            }
        }
        else {
            // Regular instruction - next block is successor (if exists)
            if (i + 1 < cfg->num_blocks) {
                basic_block* next_bb = cfg->blocks[i + 1];
                
                // Add next block as successor of current
                current->successors = (basic_block**)realloc(current->successors, 
                    (current->num_successors + 1) * sizeof(basic_block*));
                if (current->successors) {
                    current->successors[current->num_successors++] = next_bb;
                }
                
                // Add current as predecessor of next block
                next_bb->predecessors = (basic_block**)realloc(next_bb->predecessors, 
                    (next_bb->num_predecessors + 1) * sizeof(basic_block*));
                if (next_bb->predecessors) {
                    next_bb->predecessors[next_bb->num_predecessors++] = current;
                }
            }
        }
    }
    
    free(boundaries);
    return cfg;
}



// Function to analyze an instruction for data flow
void analyze_instruction_data_flow(cs_insn* insn, data_flow_info* df_info, csh handle __attribute__((unused))) {
    if (!insn->detail) return;
    
    cs_x86* x86 = &(insn->detail->x86);
    
    // Process operands to track register usage
    for (int i = 0; i < x86->op_count; i++) {
        cs_x86_op* op = &(x86->operands[i]);
        
        if (op->type == X86_OP_REG) {
            int reg_id = op->reg;
            if (i == 0 && (insn->id == X86_INS_MOV || insn->id == X86_INS_LEA || 
                           insn->id == X86_INS_ADD || insn->id == X86_INS_SUB ||
                           insn->id == X86_INS_IMUL || insn->id == X86_INS_MUL)) {
                // This is a destination operand (written to)
                if (reg_id >= X86_REG_RAX && reg_id <= X86_REG_R15) {
                    int idx = reg_id - X86_REG_RAX;
                    if (idx >= 0 && idx < 16) {
                        df_info->reg_defined[idx] = 1;
                    }
                }
            } else {
                // This is a source operand (read from)
                if (reg_id >= X86_REG_RAX && reg_id <= X86_REG_R15) {
                    int idx = reg_id - X86_REG_RAX;
                    if (idx >= 0 && idx < 16) {
                        df_info->reg_used[idx] = 1;
                    }
                }
            }
        } else if (op->type == X86_OP_MEM) {
            // Memory operand - track which registers are used as base/index
            if (op->mem.base != X86_REG_INVALID) {
                int base_reg = op->mem.base;
                if (base_reg >= X86_REG_RAX && base_reg <= X86_REG_R15) {
                    int idx = base_reg - X86_REG_RAX;
                    if (idx >= 0 && idx < 16) {
                        df_info->reg_used[idx] = 1;
                    }
                }
            }
            if (op->mem.index != X86_REG_INVALID) {
                int index_reg = op->mem.index;
                if (index_reg >= X86_REG_RAX && index_reg <= X86_REG_R15) {
                    int idx = index_reg - X86_REG_RAX;
                    if (idx >= 0 && idx < 16) {
                        df_info->reg_used[idx] = 1;
                    }
                }
            }
            df_info->memory_accessed = 1;
        }
    }
    
    // Update stack pointer tracking for PUSH/POP/ADD/SUB operations
    if (insn->id == X86_INS_PUSH || insn->id == X86_INS_CALL) {
        df_info->stack_pointer -= 8; // Push decrements RSP by 8 bytes
    } else if (insn->id == X86_INS_POP || insn->id == X86_INS_RET) {
        df_info->stack_pointer += 8; // Pop increments RSP by 8 bytes
    } else if (insn->id == X86_INS_ADD && x86->op_count == 2 && 
               x86->operands[0].type == X86_OP_REG && x86->operands[0].reg == X86_REG_RSP) {
        if (x86->operands[1].type == X86_OP_IMM) {
            df_info->stack_pointer += x86->operands[1].imm; // For ADD RSP, imm
        }
    } else if (insn->id == X86_INS_SUB && x86->op_count == 2 && 
               x86->operands[0].type == X86_OP_REG && x86->operands[0].reg == X86_REG_RSP) {
        if (x86->operands[1].type == X86_OP_IMM) {
            df_info->stack_pointer -= x86->operands[1].imm; // For SUB RSP, imm
        }
    }
}

// Dependency Graph Functions

// Function to initialize dependency graph
dependency_graph* init_dependency_graph() {
    dependency_graph* dg = (dependency_graph*)malloc(sizeof(dependency_graph));
    if (!dg) return NULL;
    
    dg->deps = NULL;
    dg->num_deps = 0;
    dg->capacity = 0;
    
    return dg;
}

// Function to add a dependency between instructions
int add_instruction_dependency(dependency_graph* dg, size_t from_idx, size_t to_idx, int dep_type) {
    if (!dg) return 0;
    
    // Grow array if needed
    if (dg->num_deps >= dg->capacity) {
        size_t new_capacity = (dg->capacity == 0) ? 16 : dg->capacity * 2;
        instruction_dependency** new_deps = (instruction_dependency**)realloc(
            dg->deps, new_capacity * sizeof(instruction_dependency*));
        if (!new_deps) return 0;
        
        dg->deps = new_deps;
        dg->capacity = new_capacity;
    }
    
    instruction_dependency* dep = (instruction_dependency*)malloc(sizeof(instruction_dependency));
    if (!dep) return 0;
    
    dep->from_idx = from_idx;
    dep->to_idx = to_idx;
    dep->dependency_type = dep_type;
    
    dg->deps[dg->num_deps++] = dep;
    return 1;
}

// Function to build dependency graph from instruction list
dependency_graph* build_dependency_graph(const instruction_list* instructions, csh handle) {
    if (!instructions || !handle) return NULL;
    
    dependency_graph* dg = init_dependency_graph();
    if (!dg) return NULL;
    
    // Simple dependency analysis: track register dependencies between instructions
    for (size_t i = 0; i < instructions->count; i++) {
        cs_insn* insn = &instructions->instructions[i];
        
        if (!insn->detail) continue;
        
        cs_x86* x86 = &(insn->detail->x86);
        
        // Check for register dependencies
        for (int j = 0; j < x86->op_count; j++) {
            cs_x86_op* op = &(x86->operands[j]);
            
            if (op->type == X86_OP_REG) {
                int reg_id = op->reg;
                
                // Look for previous instructions that define this register
                for (int prev_idx = i - 1; prev_idx >= 0; prev_idx--) {
                    cs_insn* prev_insn = &instructions->instructions[prev_idx];
                    
                    if (!prev_insn->detail) continue;
                    
                    cs_x86* prev_x86 = &(prev_insn->detail->x86);
                    
                    // Check if previous instruction defines this register
                    if (prev_x86->op_count > 0 && 
                        prev_x86->operands[0].type == X86_OP_REG &&
                        (int)prev_x86->operands[0].reg == reg_id) {
                        
                        // Found a data dependency: prev_idx defines reg used in i
                        if (j == 0 && (insn->id == X86_INS_MOV || insn->id == X86_INS_LEA || 
                                      insn->id == X86_INS_ADD || insn->id == X86_INS_SUB)) {
                            // This is a destination operand, so it's an anti-dependency (WAW or WAR)
                            add_instruction_dependency(dg, prev_idx, i, 2); // Anti-dependency
                        } else {
                            // This is a source operand, so it's a data dependency
                            add_instruction_dependency(dg, prev_idx, i, 0); // Data dependency
                        }
                        
                        // Only track immediate dependencies (closest definition)
                        break;
                    }
                    
                    // Stop searching if we find a redefinition of this register
                    if (prev_x86->op_count > 0 && 
                        prev_x86->operands[0].type == X86_OP_REG &&
                        (int)prev_x86->operands[0].reg == reg_id) {
                        break;
                    }
                }
            }
        }
    }
    
    return dg;
}