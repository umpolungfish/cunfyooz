#include "pe_parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

pe_info* parse_pe(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        perror("Failed to open file");
        return NULL;
    }

    pe_info* pe = (pe_info*)malloc(sizeof(pe_info));
    if (!pe) {
        fprintf(stderr, "Failed to allocate memory for pe_info.\n");
        fclose(file);
        return NULL;
    }
    memset(pe, 0, sizeof(pe_info));

    // Get file size
    fseek(file, 0, SEEK_END);
    pe->file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Read entire file into buffer
    pe->file_buffer = (uint8_t*)malloc(pe->file_size);
    if (!pe->file_buffer) {
        fprintf(stderr, "Failed to allocate memory for file buffer.\n");
        free(pe);
        fclose(file);
        return NULL;
    }
    if (fread(pe->file_buffer, 1, pe->file_size, file) != pe->file_size) {
        fprintf(stderr, "Failed to read entire file.\n");
        free(pe->file_buffer);
        free(pe);
        fclose(file);
        return NULL;
    }
    fclose(file);

    // Parse DOS header
    memcpy(&pe->dos_header, pe->file_buffer, sizeof(IMAGE_DOS_HEADER));
    if (pe->dos_header.e_magic != IMAGE_DOS_SIGNATURE) {
        fprintf(stderr, "Not a valid PE file (Invalid DOS header).\n");
        free(pe->file_buffer);
        free(pe);
        return NULL;
    }

    // Parse NT headers
    uint32_t nt_headers_offset = pe->dos_header.e_lfanew;
    if ((size_t)nt_headers_offset + sizeof(IMAGE_NT_HEADERS64) > pe->file_size) {
        fprintf(stderr, "Invalid NT headers offset.\n");
        free(pe->file_buffer);
        free(pe);
        return NULL;
    }
    memcpy(&pe->nt_headers, pe->file_buffer + nt_headers_offset, sizeof(IMAGE_NT_HEADERS64));

    if (pe->nt_headers.Signature != IMAGE_NT_SIGNATURE) {
        fprintf(stderr, "Not a valid PE file (Invalid NT signature).\n");
        free(pe->file_buffer);
        free(pe);
        return NULL;
    }

    // Check for 64-bit PE
    if (pe->nt_headers.OptionalHeader.Magic != 0x20b) { // PE32+ magic
        fprintf(stderr, "Only 64-bit PE files are fully supported.\n");
        free(pe->file_buffer);
        free(pe);
        return NULL;
    }

    pe->num_sections = pe->nt_headers.FileHeader.NumberOfSections;
    pe->sections = (pe_section*)malloc(pe->num_sections * sizeof(pe_section));
    if (!pe->sections) {
        fprintf(stderr, "Failed to allocate memory for sections.\n");
        free(pe->file_buffer);
        free(pe);
        return NULL;
    }
    memset(pe->sections, 0, pe->num_sections * sizeof(pe_section));

    // Parse section headers
    uint32_t section_header_offset = nt_headers_offset + sizeof(IMAGE_NT_HEADERS64);
    for (int i = 0; i < pe->num_sections; i++) {
        IMAGE_SECTION_HEADER section_header;
        if ((size_t)section_header_offset + (i + 1) * sizeof(IMAGE_SECTION_HEADER) > pe->file_size) {
            fprintf(stderr, "Invalid section header offset.\n");
            // Free already allocated sections
            for (int j = 0; j < i; j++) {
                // No need to free pe->sections[j].data as it points to file_buffer
            }
            free(pe->sections);
            free(pe->file_buffer);
            free(pe);
            return NULL;
        }
        memcpy(&section_header, pe->file_buffer + section_header_offset + i * sizeof(IMAGE_SECTION_HEADER), sizeof(IMAGE_SECTION_HEADER));

        memcpy(pe->sections[i].name, section_header.Name, 8);
        pe->sections[i].name[8] = '\0';
        pe->sections[i].virtual_address = section_header.VirtualAddress;
        pe->sections[i].size = section_header.SizeOfRawData;
        // Store the complete section header to preserve characteristics
        pe->sections[i].header = section_header;
        // Point to data within the main file buffer
        pe->sections[i].data = pe->file_buffer + section_header.PointerToRawData;
    }

    return pe;
}

void free_pe_info(pe_info* pe) {
    if (pe) {
        if (pe->file_buffer) {
            free(pe->file_buffer);
        }
        if (pe->sections) {
            free(pe->sections);
        }
        free(pe);
    }
}

// Function to update the PE file with a new code section and write to a new file
int write_transformed_pe(const pe_info* original_pe, const unsigned char* new_code, size_t new_code_size, const char* output_filename) {
    if (!original_pe || !new_code || !output_filename) {
        fprintf(stderr, "Invalid parameters passed to write_transformed_pe\n");
        return -1;
    }
    
    FILE* output_file = fopen(output_filename, "wb"); 
    if (!output_file) {
        perror("Failed to open output file");
        return -1;
    }
    
    // Find the .text section to replace
    int text_section_idx = -1;
    for (int i = 0; i < original_pe->num_sections; i++) {
        if (strcmp((char*)original_pe->sections[i].name, ".text") == 0) {
            text_section_idx = i;
            break;
        }
    }
    
    if (text_section_idx == -1) {
        fprintf(stderr, "Could not find .text section to update.\n");
        fclose(output_file);
        return -1;
    }
    
    // Calculate alignment values
    uint32_t file_alignment = original_pe->nt_headers.OptionalHeader.FileAlignment;
    uint32_t section_alignment = original_pe->nt_headers.OptionalHeader.SectionAlignment;
    
    // Calculate new code size with proper alignment
    uint32_t aligned_new_code_size = ((new_code_size + file_alignment - 1) / file_alignment) * file_alignment;
    
    // Calculate the new file size, preserving the original layout as much as possible
    // Find the last section's end to determine base file size
    uint32_t last_section_end_offset = 0;
    for (int i = 0; i < original_pe->num_sections; i++) {
        uint32_t section_start_offset = original_pe->sections[i].data - original_pe->file_buffer;
        uint32_t section_end_offset;
        
        if (i == text_section_idx) {
            // For the text section, use the new aligned size
            section_end_offset = section_start_offset + aligned_new_code_size;
        } else {
            // For other sections, use original size
            section_end_offset = section_start_offset + original_pe->sections[i].size;
        }
        
        if (section_end_offset > last_section_end_offset) {
            last_section_end_offset = section_end_offset;
        }
    }
    
    // Ensure we have at least as many bytes as the original file
    size_t new_file_size = (last_section_end_offset > original_pe->file_size) ? 
                           last_section_end_offset : original_pe->file_size;
    
    // Create a new file buffer
    uint8_t* new_file_buffer = (uint8_t*)malloc(new_file_size);
    if (!new_file_buffer) {
        fprintf(stderr, "Failed to allocate memory for new file buffer.\n");
        fclose(output_file);
        return -1;
    }
    
    // Copy the original file content (copy as much as possible)
    size_t copy_size = (original_pe->file_size < new_file_size) ? original_pe->file_size : new_file_size;
    memcpy(new_file_buffer, original_pe->file_buffer, copy_size);
    
    // If we expanded the file, zero out the new area
    if (new_file_size > copy_size) {
        memset(new_file_buffer + copy_size, 0, new_file_size - copy_size);
    }
    
    // Update the .text section data in the new file buffer
    uint32_t text_section_file_offset = original_pe->sections[text_section_idx].data - original_pe->file_buffer;
    
    // Copy the new transformed code to the .text section location
    memcpy(new_file_buffer + text_section_file_offset, new_code, new_code_size);
    
    // If the new code is smaller than the aligned size, zero-pad it
    if (new_code_size < aligned_new_code_size) {
        memset(new_file_buffer + text_section_file_offset + new_code_size, 0, 
               aligned_new_code_size - new_code_size);
    }
    
    // Update the section header in the new file
    uint32_t nt_headers_offset = original_pe->dos_header.e_lfanew;
    uint32_t section_header_offset = nt_headers_offset + sizeof(IMAGE_NT_HEADERS64);
    IMAGE_SECTION_HEADER* new_section_header = (IMAGE_SECTION_HEADER*)(new_file_buffer + section_header_offset + text_section_idx * sizeof(IMAGE_SECTION_HEADER));
    
    // Calculate aligned virtual size for the section
    uint32_t aligned_virtual_size = ((new_code_size + section_alignment - 1) / section_alignment) * section_alignment;
    
    // Update section header fields for .text section
    new_section_header->Misc.VirtualSize = aligned_virtual_size;  // Virtual size (aligned)
    new_section_header->VirtualAddress = original_pe->sections[text_section_idx].virtual_address;  // Keep original VA
    new_section_header->SizeOfRawData = aligned_new_code_size;  // File size (aligned)
    new_section_header->PointerToRawData = text_section_file_offset;  // Keep original file offset
    // Preserve original section characteristics to maintain executable permissions
    new_section_header->Characteristics = original_pe->sections[text_section_idx].header.Characteristics;

    // Update VAs of sections that come AFTER .text if .text changed size
    int64_t text_size_delta = (int64_t)aligned_virtual_size - (int64_t)original_pe->sections[text_section_idx].header.Misc.VirtualSize;
    if (text_size_delta != 0) {
        printf("Updating VAs of sections after .text (delta: %ld bytes)\n", (long)text_size_delta);
        for (int i = 0; i < original_pe->num_sections; i++) {
            if (i == text_section_idx) continue; // Skip .text itself

            IMAGE_SECTION_HEADER* section_hdr = (IMAGE_SECTION_HEADER*)(
                new_file_buffer + section_header_offset + i * sizeof(IMAGE_SECTION_HEADER));

            // If this section's VA comes after .text's VA, update it
            if (original_pe->sections[i].virtual_address > original_pe->sections[text_section_idx].virtual_address) {
                uint32_t old_va = section_hdr->VirtualAddress;
                section_hdr->VirtualAddress = (uint32_t)(old_va + text_size_delta);
                printf("  Section %d (%s): VA 0x%x -> 0x%x\n",
                       i, original_pe->sections[i].name, old_va, section_hdr->VirtualAddress);
            }
        }
    }
    
    // Update the NT headers
    IMAGE_NT_HEADERS64* new_nt_headers = (IMAGE_NT_HEADERS64*)(new_file_buffer + nt_headers_offset);
    
    // Preserve critical fields that affect execution environment
    uint64_t original_image_base = new_nt_headers->OptionalHeader.ImageBase;
    uint64_t original_size_of_stack_reserve = new_nt_headers->OptionalHeader.SizeOfStackReserve;
    uint64_t original_size_of_stack_commit = new_nt_headers->OptionalHeader.SizeOfStackCommit;
    uint64_t original_size_of_heap_reserve = new_nt_headers->OptionalHeader.SizeOfHeapReserve;
    uint64_t original_size_of_heap_commit = new_nt_headers->OptionalHeader.SizeOfHeapCommit;
    uint32_t original_subsystem = new_nt_headers->OptionalHeader.Subsystem;
    uint32_t original_dll_characteristics = new_nt_headers->OptionalHeader.DllCharacteristics;
    uint32_t original_entry_point = new_nt_headers->OptionalHeader.AddressOfEntryPoint;
    
    // Update SizeOfCode to reflect the new code size
    new_nt_headers->OptionalHeader.SizeOfCode = aligned_new_code_size;
    
    // Restore critical fields that affect execution environment
    new_nt_headers->OptionalHeader.ImageBase = original_image_base;
    new_nt_headers->OptionalHeader.SizeOfStackReserve = original_size_of_stack_reserve;
    new_nt_headers->OptionalHeader.SizeOfStackCommit = original_size_of_stack_commit;
    new_nt_headers->OptionalHeader.SizeOfHeapReserve = original_size_of_heap_reserve;
    new_nt_headers->OptionalHeader.SizeOfHeapCommit = original_size_of_heap_commit;
    new_nt_headers->OptionalHeader.Subsystem = original_subsystem;
    new_nt_headers->OptionalHeader.DllCharacteristics = original_dll_characteristics;
    new_nt_headers->OptionalHeader.AddressOfEntryPoint = original_entry_point;
    
    // Update CheckSum to 0 to avoid verification issues
    new_nt_headers->OptionalHeader.CheckSum = 0;
    
    // Calculate SizeOfImage based on all sections
    uint64_t max_virtual_end = 0;
    for (int i = 0; i < original_pe->num_sections; i++) {
        uint64_t section_virtual_end;
        if (i == text_section_idx) {
            // For the modified text section, use aligned virtual size
            section_virtual_end = original_pe->sections[i].virtual_address + aligned_virtual_size;
        } else {
            // For other sections, use their VirtualSize from header (not SizeOfRawData)
            section_virtual_end = original_pe->sections[i].virtual_address + original_pe->sections[i].header.Misc.VirtualSize;
        }
        
        if (section_virtual_end > max_virtual_end) {
            max_virtual_end = section_virtual_end;
        }
    }
    
    // Align total image size to section alignment
    uint64_t new_image_size = ((max_virtual_end + section_alignment - 1) / section_alignment) * section_alignment;
    new_nt_headers->OptionalHeader.SizeOfImage = new_image_size;
    
    // Preserve other important fields
    new_nt_headers->OptionalHeader.SizeOfHeaders = original_pe->nt_headers.OptionalHeader.SizeOfHeaders;
    new_nt_headers->OptionalHeader.NumberOfRvaAndSizes = original_pe->nt_headers.OptionalHeader.NumberOfRvaAndSizes;
    
    // Update DataDirectory entries to account for section size changes
    // text_size_delta was already calculated above

    printf("Text section size change: %ld bytes (%zu -> %u)\n",
           (long)text_size_delta,
           original_pe->sections[text_section_idx].size,
           aligned_new_code_size);

    // Get the .text section's virtual address range
    uint32_t text_va_start = original_pe->sections[text_section_idx].virtual_address;
    uint32_t text_va_end = text_va_start + aligned_virtual_size;

    for (uint32_t i = 0; i < original_pe->nt_headers.OptionalHeader.NumberOfRvaAndSizes; i++) {
        IMAGE_DATA_DIRECTORY* dir = &new_nt_headers->OptionalHeader.DataDirectory[i];
        uint32_t original_rva = original_pe->nt_headers.OptionalHeader.DataDirectory[i].VirtualAddress;
        uint32_t original_size = original_pe->nt_headers.OptionalHeader.DataDirectory[i].Size;

        // Copy original values first
        dir->VirtualAddress = original_rva;
        dir->Size = original_size;

        if (original_rva == 0 || original_size == 0) {
            continue; // Empty entry, nothing to update
        }

        // If this RVA points AFTER the .text section, we need to adjust it
        // But only if the .text section changed size
        if (text_size_delta != 0 && original_rva > text_va_end) {
            // This RVA points to a location after .text, so it needs adjustment
            printf("Updating DataDirectory[%u] RVA: 0x%x -> 0x%x\n",
                   i, original_rva, (uint32_t)(original_rva + text_size_delta));
            dir->VirtualAddress = (uint32_t)(original_rva + text_size_delta);
        }
        // If the RVA points WITHIN the .text section, we generally shouldn't update it
        // unless we've relocated specific structures, which we haven't in this case
        // The exception would be if we're updating things like the export table which
        // might be in .text, but that's unlikely for most binaries
    }
    
    // Write the modified PE file
    if (fwrite(new_file_buffer, 1, new_file_size, output_file) != new_file_size) {
        fprintf(stderr, "Failed to write complete file.\n");
        free(new_file_buffer);
        fclose(output_file);
        return -1;
    }
    
    free(new_file_buffer);
    fclose(output_file);
    return 0;
}

