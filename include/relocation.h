#ifndef RELOCATION_H
#define RELOCATION_H

#include <stdint.h>
#include <stddef.h>
#include <capstone/capstone.h>
#include "disassembler.h"

// Structure to track address mappings during transformation
typedef struct address_map_entry {
    uint64_t original_address;
    uint64_t new_address;
    size_t instruction_index;  // Index in instruction list
} address_map_entry;

typedef struct address_map {
    address_map_entry* entries;
    size_t count;
    size_t capacity;
} address_map;

// Structure to track jump/call targets that need relocation
typedef struct relocation_entry {
    size_t instruction_index;   // Which instruction contains the jump/call
    uint64_t target_address;    // Original target address
    int operand_index;          // Which operand contains the target
    bool is_relative;           // Is it a relative offset or absolute address?
} relocation_entry;

typedef struct relocation_table {
    relocation_entry* entries;
    size_t count;
    size_t capacity;
} relocation_table;

// Create and manage address maps
address_map* create_address_map(void);
void free_address_map(address_map* map);
int add_address_mapping(address_map* map, uint64_t original_addr, uint64_t new_addr, size_t index);
uint64_t lookup_new_address(const address_map* map, uint64_t original_addr);

// Create and manage relocation tables
relocation_table* create_relocation_table(void);
void free_relocation_table(relocation_table* table);
int add_relocation_entry(relocation_table* table, size_t insn_index, uint64_t target_addr, int op_index, bool is_relative);

// Build relocation table from instruction list
relocation_table* build_relocation_table(const instruction_list* instructions, uint64_t base_address);

// Apply relocations after transformations
int apply_relocations(instruction_list* instructions, const relocation_table* rel_table, const address_map* addr_map, csh handle);

// Update addresses and build address map
address_map* recalculate_addresses_with_map(instruction_list* instructions);

#endif // RELOCATION_H
