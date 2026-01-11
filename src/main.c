#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <capstone/capstone.h>

#include "pe_parser.h"
#include "disassembler.h"
#include "transformer.h"
#include "assembler.h"
#include "relocation.h"

#include "json_parser.h"
#include <sys/wait.h>

// Function to validate that transformed executable behaves the same as original
int validate_transformation(const char* original_file __attribute__((unused)),
                            const char* transformed_file __attribute__((unused))) {
    // SECURITY UPDATE: This function is now disabled by default to prevent execution of potentially malicious binaries
    // Previous implementation ran both executables and compared outputs using wine and popen
    // This posed a security risk when processing untrusted binaries

    fprintf(stderr, "Functionality validation is disabled by default for security reasons.\n");
    fprintf(stderr, "Validation requires executing both original and transformed binaries for comparison.\n");
    fprintf(stderr, "This is disabled to prevent execution of potentially malicious code.\n");

    // Return -1 to indicate validation is skipped due to security concerns
    return -1; // Indicate that validation was not performed for security reasons
}

int main(int argc, char *argv[]) {
    // Initialize random seed for metamorphic operations
    srand((unsigned int)time(NULL) + clock());
    
    config_t* config = NULL;
    FILE* config_file = fopen("config.json", "r");
    if (config_file) {
        fseek(config_file, 0, SEEK_END);
        long length = ftell(config_file);
        fseek(config_file, 0, SEEK_SET);
        char* buffer = (char*)malloc(length + 1);
        if (buffer) {
            fread(buffer, 1, length, config_file);
            buffer[length] = '\0';
            config = parse_json_config(buffer);
            free(buffer);
        }
        fclose(config_file);
    }

    if (!config) {
        // Create default config if file doesn't exist or parsing fails
        config = (config_t*)malloc(sizeof(config_t));
        if (config) {
            config->transformations.nop_insertion.enabled = true;
            config->transformations.instruction_substitution.enabled = true;
            config->transformations.register_shuffling.enabled = true;
            config->transformations.enhanced_nop_insertion.enabled = true;
            config->transformations.control_flow_obfuscation.enabled = true;
            config->transformations.stack_frame_obfuscation.enabled = true;
            config->transformations.instruction_reordering.enabled = true;
            config->transformations.anti_analysis_techniques.enabled = true;
            config->transformations.virtualization_engine.enabled = false; // Disabled by default

            // Set default security settings - DISABLE EXECUTION BY DEFAULT FOR SECURITY
            config->security.validate_functionality = false;  // Changed from true to false
            config->security.preserve_original_behavior = true;
        }
    }

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <executable_path>\n", argv[0]);
        return 1;
    }

    const char *filepath = argv[1];
    uint8_t *code = NULL;
    size_t code_size = 0;
    uint64_t base_address = 0;

    // Initialize Capstone
    csh handle;
    if (cs_open(CS_ARCH_X86, CS_MODE_64, &handle) != CS_ERR_OK) {
        fprintf(stderr, "Failed to initialize Capstone\n");
        return 1;
    }
    cs_option(handle, CS_OPT_DETAIL, CS_OPT_ON); // Enable detail for operands

    // For simplicity, let's assume it's a PE file for now and extract code section
    // In a real scenario, you'd detect file type (ELF, PE, etc.)
    pe_info *pe = parse_pe(filepath);
    if (!pe) {
        fprintf(stderr, "Failed to parse PE file or not a PE file.\n");
        cs_close(&handle);
        return 1;
    }

    // Assuming .text section contains executable code
    for (int i = 0; i < pe->num_sections; i++) {
        if (strcmp((char*)pe->sections[i].name, ".text") == 0) {
            code = pe->sections[i].data;
            code_size = pe->sections[i].size;
            base_address = pe->sections[i].virtual_address;
            break;
        }
    }

    if (!code || code_size == 0) {
        fprintf(stderr, "Could not find .text section or it's empty.\n");
        free_pe_info(pe);
        cs_close(&handle);
        return 1;
    }

    instruction_list* original_instructions = disassemble(handle, code, code_size, base_address);
    if (!original_instructions) {
        fprintf(stderr, "Failed to disassemble code.\n");
        free_pe_info(pe);
        cs_close(&handle);
        return 1;
    }

    // Build relocation table from original instructions to track jump/call targets
    printf("Building relocation table to track jump/call targets...\n");
    relocation_table* rel_table = build_relocation_table(original_instructions, base_address);
    if (!rel_table) {
        fprintf(stderr, "Warning: Failed to build relocation table. Jump targets may be incorrect.\n");
    } else {
        printf("Found %zu jump/call targets to track\n", rel_table->count);
    }

    // Create address map to track address changes during transformations
    address_map* addr_map = create_address_map();
    if (!addr_map) {
        fprintf(stderr, "Warning: Failed to create address map. Relocations may fail.\n");
    }

    // Add initial mappings (identity mapping for original addresses)
    if (addr_map) {
        for (size_t i = 0; i < original_instructions->count; i++) {
            add_address_mapping(addr_map,
                original_instructions->instructions[i].address,
                original_instructions->instructions[i].address,
                i);
        }
    }

    instruction_list* transformed_instructions = original_instructions;
    instruction_list* to_free = NULL;  // Keep track of intermediate results to free

    if (config && config->transformations.nop_insertion.enabled) {
        instruction_list* next_instructions = apply_nop_insertion(transformed_instructions);
        if (transformed_instructions != original_instructions) {
            // If this isn't the original list, it's an intermediate result we need to free later
            if (to_free) free_instruction_list(to_free);
            to_free = transformed_instructions;
        }
        transformed_instructions = next_instructions;
        recalculate_addresses(transformed_instructions);
    }
    if (config && config->transformations.instruction_substitution.enabled) {
        instruction_list* next_instructions = apply_instruction_substitution(transformed_instructions, handle);
        if (transformed_instructions != original_instructions) {
            // If this isn't the original list, it's an intermediate result we need to free later
            if (to_free) free_instruction_list(to_free);
            to_free = transformed_instructions;
        }
        transformed_instructions = next_instructions;
        recalculate_addresses(transformed_instructions);
    }
    if (config && config->transformations.register_shuffling.enabled) {
        instruction_list* next_instructions = apply_register_shuffling(transformed_instructions, handle);
        if (transformed_instructions != original_instructions) {
            // If this isn't the original list, it's an intermediate result we need to free later
            if (to_free) free_instruction_list(to_free);
            to_free = transformed_instructions;
        }
        transformed_instructions = next_instructions;
        recalculate_addresses(transformed_instructions);
    }
    if (config && config->transformations.enhanced_nop_insertion.enabled) {
        instruction_list* next_instructions = apply_enhanced_nop_insertion(transformed_instructions);
        if (transformed_instructions != original_instructions) {
            // If this isn't the original list, it's an intermediate result we need to free later
            if (to_free) free_instruction_list(to_free);
            to_free = transformed_instructions;
        }
        transformed_instructions = next_instructions;
        recalculate_addresses(transformed_instructions);
    }
    if (config && config->transformations.control_flow_obfuscation.enabled) {
        instruction_list* next_instructions = apply_control_flow_obfuscation(transformed_instructions);
        if (transformed_instructions != original_instructions) {
            // If this isn't the original list, it's an intermediate result we need to free later
            if (to_free) free_instruction_list(to_free);
            to_free = transformed_instructions;
        }
        transformed_instructions = next_instructions;
        recalculate_addresses(transformed_instructions);
    }
    if (config && config->transformations.stack_frame_obfuscation.enabled) {
        instruction_list* next_instructions = apply_stack_frame_obfuscation(transformed_instructions);
        if (transformed_instructions != original_instructions) {
            // If this isn't the original list, it's an intermediate result we need to free later
            if (to_free) free_instruction_list(to_free);
            to_free = transformed_instructions;
        }
        transformed_instructions = next_instructions;
        recalculate_addresses(transformed_instructions);
    }
    if (config && config->transformations.instruction_reordering.enabled) {
        instruction_list* next_instructions = apply_instruction_reordering(transformed_instructions);
        if (transformed_instructions != original_instructions) {
            // If this isn't the original list, it's an intermediate result we need to free later
            if (to_free) free_instruction_list(to_free);
            to_free = transformed_instructions;
        }
        transformed_instructions = next_instructions;
        recalculate_addresses(transformed_instructions);
    }
    if (config && config->transformations.anti_analysis_techniques.enabled) {
        instruction_list* next_instructions = apply_anti_analysis_techniques(transformed_instructions);
        if (transformed_instructions != original_instructions) {
            // If this isn't the original list, it's an intermediate result we need to free later
            if (to_free) free_instruction_list(to_free);
            to_free = transformed_instructions;
        }
        transformed_instructions = next_instructions;
        recalculate_addresses(transformed_instructions);
    }
    if (config && config->transformations.virtualization_engine.enabled) {
        instruction_list* next_instructions = apply_virtualization_engine(transformed_instructions);
        if (transformed_instructions != original_instructions) {
            // If this isn't the original list, it's an intermediate result we need to free later
            if (to_free) free_instruction_list(to_free);
            to_free = transformed_instructions;
        }
        transformed_instructions = next_instructions;
        recalculate_addresses(transformed_instructions);
    }

    // Final address recalculation with mapping
    printf("Performing final address recalculation and relocation updates...\n");
    // Rebuild address map after all transformations
    if (addr_map) {
        free_address_map(addr_map);
    }
    addr_map = recalculate_addresses_with_map(transformed_instructions);
    if (!addr_map) {
        fprintf(stderr, "Warning: Failed to rebuild address map for relocations\n");
        recalculate_addresses(transformed_instructions);
    }

    // Apply relocations to fix jump/call targets
    if (rel_table && addr_map) {
        printf("Applying relocations to %zu jump/call targets...\n", rel_table->count);
        int reloc_result = apply_relocations(transformed_instructions, rel_table, addr_map, handle);
        if (reloc_result != 0) {
            fprintf(stderr, "Warning: Some relocations may have failed\n");
        } else {
            printf("Successfully updated all jump/call targets\n");
        }
    }

    // Reassemble transformed instructions back into binary format
    size_t reassembled_size;
    unsigned char* reassembled_code = reassemble_instructions(transformed_instructions, &reassembled_size);
    if (!reassembled_code) {
        fprintf(stderr, "Failed to reassemble transformed instructions.\n");
        if (transformed_instructions != original_instructions) free_instruction_list(transformed_instructions);
        free_instruction_list(original_instructions);
        free_pe_info(pe);
        cs_close(&handle);
        return 1;
    }
    printf("\nReassembled code size: %zu bytes\n", reassembled_size);

    // Generate output filename - prepend "cunfyoozed_" to the original filename
    char output_filename[512];
    
    // Find the last occurrence of path separator to extract directory path
    const char* last_sep = strrchr(filepath, '/');
    if (!last_sep) {
        last_sep = strrchr(filepath, '\\');  // Windows-style path as well
    }
    
    if (last_sep) {
        // Copy the directory part
        size_t dir_len = last_sep - filepath + 1; // +1 to include the separator
        strncpy(output_filename, filepath, dir_len);
        output_filename[dir_len] = '\0';
        
        // Add "cunfyoozed_" prefix to the basename part
        strcat(output_filename, "cunfyoozed_");
        strcat(output_filename, last_sep + 1); // The filename after separator
    } else {
        // No directory path, just filename
        snprintf(output_filename, sizeof(output_filename), "cunfyoozed_%s", filepath);
    }
    
    // Write the transformed PE file
    int write_result = write_transformed_pe(pe, reassembled_code, reassembled_size, output_filename);
    if (write_result != 0) {
        fprintf(stderr, "Failed to write transformed PE file.\n");
        free(reassembled_code);
        if (transformed_instructions != original_instructions) free_instruction_list(transformed_instructions);
        free_instruction_list(original_instructions);
        free_pe_info(pe);
        cs_close(&handle);
        return 1;
    }
    
    // If validation is enabled in the config, check that the transformed executable behaves the same as the original
    if (config && config->security.validate_functionality) {
        printf("Validating transformed executable functionality...\n");
        int validation_result = validate_transformation(filepath, output_filename);

        if (validation_result == -1) {
            // Validation was skipped for security reasons (execution disabled)
            printf("Validation skipped: Execution disabled for security (would execute binaries for comparison)\n");
        } else if (validation_result == 0) {
            fprintf(stderr, "Validation failed: Transformed executable produces different output than original\n");

            // Remove the invalid transformed file
            remove(output_filename);

            // Clean up resources
            free(reassembled_code);
            free_instruction_list(transformed_instructions);
            free_instruction_list(original_instructions);
            if (to_free && to_free != transformed_instructions && to_free != original_instructions) {
                free_instruction_list(to_free);
            }
            free_pe_info(pe);
            cs_close(&handle);
            if (config) free(config);
            return 1;
        } else {
            printf("Validation passed: Transformed executable produces same output as original\n");
        }
    }
    
    printf("Successfully wrote transformed PE file to: %s\n", output_filename);

    // Clean up
    free(reassembled_code);
    free_instruction_list(original_instructions);
    if (transformed_instructions != original_instructions) {
        free_instruction_list(transformed_instructions);
    }
    if (to_free && to_free != transformed_instructions && to_free != original_instructions) {
        free_instruction_list(to_free);
    }
    if (rel_table) free_relocation_table(rel_table);
    if (addr_map) free_address_map(addr_map);
    free_pe_info(pe);
    cs_close(&handle);
    if (config) free(config);

    return 0;
}
