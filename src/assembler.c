#include "assembler.h"
#include <stdio.h>
#include <string.h>
#include <keystone/keystone.h>

unsigned char* assemble_instruction(const char* assembly, uint64_t address, size_t* size) {
    ks_engine *ks;
    unsigned char *encode;
    size_t count;

    // Validate input parameters
    if (!assembly || !size) {
        fprintf(stderr, "Invalid parameters to assemble_instruction\n");
        return NULL;
    }

    if (ks_open(KS_ARCH_X86, KS_MODE_64, &ks) != KS_ERR_OK) {
        fprintf(stderr, "Failed to initialize Keystone\n");
        return NULL;
    }

    // Set Intel syntax
    if (ks_option(ks, KS_OPT_SYNTAX, KS_OPT_SYNTAX_INTEL) != KS_ERR_OK) {
        fprintf(stderr, "Failed to set Keystone syntax to Intel\n");
        ks_close(ks);
        return NULL;
    }

    if (ks_asm(ks, assembly, address, &encode, size, &count) != KS_ERR_OK) {
        fprintf(stderr, "Failed to assemble code: %s\n", ks_strerror(ks_errno(ks)));
        ks_close(ks);
        return NULL;
    }

    ks_close(ks);
    return encode;
}


// Function to reassemble an entire instruction list into binary format
unsigned char* reassemble_instructions(const instruction_list* instructions, size_t* total_size) {
    if (!instructions || !total_size) {
        return NULL;
    }

    // First pass: Calculate size with validation
    size_t calculated_size = 0;
    size_t invalid_count = 0;
    for (size_t i = 0; i < instructions->count; i++) {
        // Validate instruction size
        if (instructions->instructions[i].size == 0) {
            fprintf(stderr, "Warning: Instruction %zu has zero size (mnemonic: %s)\n",
                    i, instructions->instructions[i].mnemonic);
            invalid_count++;
            // We'll try to reassemble it in the second pass
        }
        calculated_size += instructions->instructions[i].size;
    }

    if (invalid_count > 0) {
        fprintf(stderr, "Warning: Found %zu instructions with zero size. Will attempt to reassemble them.\n", invalid_count);
    }

    if (calculated_size == 0) {
        fprintf(stderr, "Error: Total calculated size is 0\n");
        return NULL;
    }

    // Allocate buffer with extra space for reassembled instructions (may be larger)
    size_t buffer_size = calculated_size * 2;  // 2x for safety
    unsigned char* binary_buffer = (unsigned char*)malloc(buffer_size);
    if (!binary_buffer) {
        fprintf(stderr, "Failed to allocate memory for binary buffer of size %zu\n", buffer_size);
        return NULL;
    }

    size_t offset = 0;
    for (size_t i = 0; i < instructions->count; i++) {
        const cs_insn* insn = &instructions->instructions[i];
        
        // For each instruction, use its bytes directly if available and valid
        // Instructions created during transformation should have properly assembled bytes
        if (!insn->bytes || insn->size == 0) {
            // If the instruction doesn't have valid bytes, we need to assemble it from the mnemonic and operands
            char assembly_str[256];
            if (insn->op_str[0] == '\0') {
                // No operands
                snprintf(assembly_str, sizeof(assembly_str), "%s", insn->mnemonic);
            } else {
                // Has operands
                snprintf(assembly_str, sizeof(assembly_str), "%s %s", insn->mnemonic, insn->op_str);
            }
            
            // Assemble this instruction to get the actual bytes
            size_t real_insn_size;
            unsigned char* real_insn_bytes = assemble_instruction(assembly_str, 0, &real_insn_size);
            if (!real_insn_bytes || real_insn_size == 0) {
                fprintf(stderr, "Failed to reassemble instruction at index %zu: %s %s\n", i, insn->mnemonic, insn->op_str);
                free(binary_buffer);
                return NULL;
            }
            
            // Check if copying would exceed buffer bounds
            if (offset + real_insn_size > buffer_size) {
                fprintf(stderr, "Error: Buffer overflow during reassembly at index %zu\n", i);
                fprintf(stderr, "       Needed %zu bytes, buffer size is %zu bytes\n",
                        offset + real_insn_size, buffer_size);
                free(real_insn_bytes);
                free(binary_buffer);
                return NULL;
            }
            
            // Copy the freshly assembled bytes
            memcpy(binary_buffer + offset, real_insn_bytes, real_insn_size);
            offset += real_insn_size;
            free(real_insn_bytes);
        } else {
            // Check if copying would exceed buffer bounds
            if (offset + insn->size > buffer_size) {
                fprintf(stderr, "Error: Buffer overflow detected during reassembly at index %zu\n", i);
                fprintf(stderr, "       Needed %zu bytes, buffer size is %zu bytes\n",
                        offset + insn->size, buffer_size);
                free(binary_buffer);
                return NULL;
            }
            
            // Copy the instruction bytes
            memcpy(binary_buffer + offset, insn->bytes, insn->size);
            offset += insn->size;
        }
    }
    
    *total_size = offset;
    return binary_buffer;
}
