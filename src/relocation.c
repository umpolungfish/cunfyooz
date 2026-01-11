#include "relocation.h"
#include "assembler.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Create a new address map
address_map* create_address_map(void) {
    address_map* map = (address_map*)malloc(sizeof(address_map));
    if (!map) return NULL;

    map->entries = NULL;
    map->count = 0;
    map->capacity = 0;

    return map;
}

// Free an address map
void free_address_map(address_map* map) {
    if (map) {
        if (map->entries) free(map->entries);
        free(map);
    }
}

// Add an address mapping entry
int add_address_mapping(address_map* map, uint64_t original_addr, uint64_t new_addr, size_t index) {
    if (!map) return 0;

    // Expand capacity if needed
    if (map->count >= map->capacity) {
        size_t new_capacity = (map->capacity == 0) ? 64 : map->capacity * 2;
        address_map_entry* new_entries = (address_map_entry*)realloc(
            map->entries, new_capacity * sizeof(address_map_entry));

        if (!new_entries) return 0;

        map->entries = new_entries;
        map->capacity = new_capacity;
    }

    // Add the mapping
    map->entries[map->count].original_address = original_addr;
    map->entries[map->count].new_address = new_addr;
    map->entries[map->count].instruction_index = index;
    map->count++;

    return 1;
}

// Lookup a new address from an original address
uint64_t lookup_new_address(const address_map* map, uint64_t original_addr) {
    if (!map) return original_addr;

    for (size_t i = 0; i < map->count; i++) {
        if (map->entries[i].original_address == original_addr) {
            return map->entries[i].new_address;
        }
    }

    // If not found, return original (may be external target)
    return original_addr;
}

// Create a new relocation table
relocation_table* create_relocation_table(void) {
    relocation_table* table = (relocation_table*)malloc(sizeof(relocation_table));
    if (!table) return NULL;

    table->entries = NULL;
    table->count = 0;
    table->capacity = 0;

    return table;
}

// Free a relocation table
void free_relocation_table(relocation_table* table) {
    if (table) {
        if (table->entries) free(table->entries);
        free(table);
    }
}

// Add a relocation entry
int add_relocation_entry(relocation_table* table, size_t insn_index, uint64_t target_addr, int op_index, bool is_relative) {
    if (!table) return 0;

    // Expand capacity if needed
    if (table->count >= table->capacity) {
        size_t new_capacity = (table->capacity == 0) ? 64 : table->capacity * 2;
        relocation_entry* new_entries = (relocation_entry*)realloc(
            table->entries, new_capacity * sizeof(relocation_entry));

        if (!new_entries) return 0;

        table->entries = new_entries;
        table->capacity = new_capacity;
    }

    // Add the entry
    table->entries[table->count].instruction_index = insn_index;
    table->entries[table->count].target_address = target_addr;
    table->entries[table->count].operand_index = op_index;
    table->entries[table->count].is_relative = is_relative;
    table->count++;

    return 1;
}

// Helper to check if an instruction is a branch/jump/call
static bool is_control_flow_instruction(x86_insn id) {
    return (id == X86_INS_JMP || id == X86_INS_CALL ||
            (id >= X86_INS_JAE && id <= X86_INS_JS) || // Conditional jumps
            id == X86_INS_LOOP || id == X86_INS_LOOPE || id == X86_INS_LOOPNE);
}

// Build relocation table from instruction list
relocation_table* build_relocation_table(const instruction_list* instructions, uint64_t base_address __attribute__((unused))) {
    if (!instructions) return NULL;

    relocation_table* table = create_relocation_table();
    if (!table) return NULL;

    for (size_t i = 0; i < instructions->count; i++) {
        const cs_insn* insn = &instructions->instructions[i];

        // Check if this is a control flow instruction
        if (!is_control_flow_instruction(insn->id)) continue;

        // Check if we have detail information
        if (!insn->detail) continue;

        cs_x86* x86 = &(insn->detail->x86);

        // Look for immediate or memory operands that contain targets
        for (uint8_t op_idx = 0; op_idx < x86->op_count; op_idx++) {
            cs_x86_op* op = &(x86->operands[op_idx]);

            if (op->type == X86_OP_IMM) {
                // Immediate operand - this is the jump target
                uint64_t target = (uint64_t)op->imm;

                // Determine if it's relative or absolute
                // Most x86-64 jumps/calls use RIP-relative addressing
                bool is_relative = true;

                // Add to relocation table
                add_relocation_entry(table, i, target, op_idx, is_relative);

            } else if (op->type == X86_OP_MEM && op->mem.base == X86_REG_RIP) {
                // RIP-relative addressing
                uint64_t target = insn->address + insn->size + op->mem.disp;
                add_relocation_entry(table, i, target, op_idx, true);
            }
        }
    }

    return table;
}

// Recalculate addresses and build address map
address_map* recalculate_addresses_with_map(instruction_list* instructions) {
    if (!instructions || !instructions->instructions) {
        return NULL;
    }

    address_map* map = create_address_map();
    if (!map) return NULL;

    uint64_t original_address = instructions->instructions[0].address;
    uint64_t current_address = original_address;

    for (size_t i = 0; i < instructions->count; i++) {
        uint64_t old_addr = instructions->instructions[i].address;

        // Update instruction address
        instructions->instructions[i].address = current_address;

        // Add mapping
        add_address_mapping(map, old_addr, current_address, i);

        // Advance to next instruction
        current_address += instructions->instructions[i].size;
    }

    return map;
}

// Apply relocations after transformations
int apply_relocations(instruction_list* instructions, const relocation_table* rel_table, const address_map* addr_map, csh handle) {
    if (!instructions || !rel_table || !addr_map || !handle) {
        return -1;
    }

    for (size_t i = 0; i < rel_table->count; i++) {
        relocation_entry* rel = &rel_table->entries[i];

        // Find the instruction in the transformed list
        // We need to search by original address
        size_t insn_idx = (size_t)-1;

        // The relocation entry stores the index from the original list
        // We need to find the corresponding instruction in the transformed list
        // by matching the instruction ID and approximate position

        // For now, use a simpler approach: find by sequential search
        // This assumes instruction order is mostly preserved
        if (rel->instruction_index < instructions->count) {
            insn_idx = rel->instruction_index;
        } else {
            continue; // Skip if index is out of bounds
        }

        cs_insn* insn = &instructions->instructions[insn_idx];

        // Check if this is still a control flow instruction
        if (!is_control_flow_instruction(insn->id)) continue;

        // Lookup the new target address
        uint64_t new_target = lookup_new_address(addr_map, rel->target_address);

        // Reassemble the instruction with the new target
        // Build the assembly string
        char asm_str[256];

        // Get the mnemonic
        const char* mnemonic = insn->mnemonic;

        // Build operand string with new target address
        // For x86-64, most jumps/calls are RIP-relative, but we can specify absolute addresses
        // and the assembler will handle the encoding
        if (insn->id == X86_INS_CALL || insn->id == X86_INS_JMP ||
            (insn->id >= X86_INS_JAE && insn->id <= X86_INS_JS)) {
            // For jumps/calls, specify the target address
            // The assembler (Keystone) will automatically encode it correctly (RIP-relative or absolute)
            snprintf(asm_str, sizeof(asm_str), "%s 0x%lx", mnemonic, (unsigned long)new_target);
        } else {
            // For other instructions, preserve original operand format
            snprintf(asm_str, sizeof(asm_str), "%s %s", mnemonic, insn->op_str);
        }

        // Assemble the instruction
        size_t new_size;
        unsigned char* new_bytes = assemble_instruction(asm_str, insn->address, &new_size);

        if (new_bytes && new_size > 0 && new_size <= sizeof(insn->bytes)) {
            // Update the instruction bytes and size
            memcpy(insn->bytes, new_bytes, new_size);
            insn->size = new_size;
            free(new_bytes);
        } else {
            if (new_bytes) free(new_bytes);
            fprintf(stderr, "Warning: Failed to reassemble instruction at index %zu with relocated target\n", insn_idx);
            // Continue anyway - may not be critical
        }
    }

    return 0;
}
