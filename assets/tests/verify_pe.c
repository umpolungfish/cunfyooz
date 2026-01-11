#include <stdio.h>
#include <stdint.h>
#include "include/pe_parser.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <pe_file>\n", argv[0]);
        return 1;
    }

    pe_info* pe = parse_pe(argv[1]);
    if (!pe) {
        printf("Failed to parse PE file: %s\n", argv[1]);
        return 1;
    }

    printf("PE File: %s\n", argv[1]);
    printf("DOS Header: MZ=%c%c (0x%04x)\n", 
           ((char*)&pe->dos_header.e_magic)[0], 
           ((char*)&pe->dos_header.e_magic)[1], 
           pe->dos_header.e_magic);
    printf("NT Signature: %c%c%c%c (0x%08x)\n",
           ((char*)&pe->nt_headers.Signature)[0],
           ((char*)&pe->nt_headers.Signature)[1],
           ((char*)&pe->nt_headers.Signature)[2],
           ((char*)&pe->nt_headers.Signature)[3],
           pe->nt_headers.Signature);
    printf("Machine: 0x%04x\n", pe->nt_headers.FileHeader.Machine);
    printf("Number of Sections: %d\n", pe->nt_headers.FileHeader.NumberOfSections);
    printf("Time Date Stamp: 0x%08x\n", pe->nt_headers.FileHeader.TimeDateStamp);
    printf("Optional Header Magic: 0x%04x\n", pe->nt_headers.OptionalHeader.Magic);
    printf("Address of Entry Point: 0x%08x\n", pe->nt_headers.OptionalHeader.AddressOfEntryPoint);
    printf("Image Base: 0x%016llx\n", (unsigned long long)pe->nt_headers.OptionalHeader.ImageBase);
    printf("Section Alignment: 0x%08x\n", pe->nt_headers.OptionalHeader.SectionAlignment);
    printf("File Alignment: 0x%08x\n", pe->nt_headers.OptionalHeader.FileAlignment);
    printf("Size of Image: 0x%08x\n", pe->nt_headers.OptionalHeader.SizeOfImage);
    printf("Size of Headers: 0x%08x\n", pe->nt_headers.OptionalHeader.SizeOfHeaders);
    printf("CheckSum: 0x%08x\n", pe->nt_headers.OptionalHeader.CheckSum);
    printf("SubSystem: %d\n", pe->nt_headers.OptionalHeader.Subsystem);
    
    printf("\nSections:\n");
    for (int i = 0; i < pe->num_sections; i++) {
        printf("  [%d] Name: %s, Virtual Address: 0x%08x, Virtual Size: %d, Raw Data Size: %d\n", 
               i, 
               pe->sections[i].name, 
               pe->sections[i].virtual_address, 
               pe->sections[i].size, 
               pe->sections[i].size);
    }

    free_pe_info(pe);
    return 0;
}