#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// Define basic PE structures for analysis
#pragma pack(push, 1)
typedef struct _IMAGE_DOS_HEADER {
    uint16_t e_magic;
    uint16_t e_cblp;
    uint16_t e_cp;
    uint16_t e_crlc;
    uint16_t e_cparhdr;
    uint16_t e_minalloc;
    uint16_t e_maxalloc;
    uint16_t e_ss;
    uint16_t e_sp;
    uint16_t e_csum;
    uint16_t e_ip;
    uint16_t e_cs;
    uint16_t e_lfarlc;
    uint16_t e_ovno;
    uint16_t e_res[4];
    uint16_t e_oemid;
    uint16_t e_oeminfo;
    uint16_t e_res2[10];
    int32_t  e_lfanew;
} IMAGE_DOS_HEADER;

typedef struct _IMAGE_FILE_HEADER {
    uint16_t Machine;
    uint16_t NumberOfSections;
    uint32_t TimeDateStamp;
    uint32_t PointerToSymbolTable;
    uint32_t NumberOfSymbols;
    uint16_t SizeOfOptionalHeader;
    uint16_t Characteristics;
} IMAGE_FILE_HEADER;

typedef struct _IMAGE_OPTIONAL_HEADER64 {
    uint16_t Magic;
    uint8_t  MajorLinkerVersion;
    uint8_t  MinorLinkerVersion;
    uint32_t SizeOfCode;
    uint32_t SizeOfInitializedData;
    uint32_t SizeOfUninitializedData;
    uint32_t AddressOfEntryPoint;
    uint32_t BaseOfCode;
    uint64_t ImageBase;
    uint32_t SectionAlignment;
    uint32_t FileAlignment;
    uint16_t MajorOperatingSystemVersion;
    uint16_t MinorOperatingSystemVersion;
    uint16_t MajorImageVersion;
    uint16_t MinorImageVersion;
    uint16_t MajorSubsystemVersion;
    uint16_t MinorSubsystemVersion;
    uint32_t Win32VersionValue;
    uint32_t SizeOfImage;
    uint32_t SizeOfHeaders;
    uint32_t CheckSum;
    uint16_t Subsystem;
    uint16_t DllCharacteristics;
    uint64_t SizeOfStackReserve;
    uint64_t SizeOfStackCommit;
    uint64_t SizeOfHeapReserve;
    uint64_t SizeOfHeapCommit;
    uint32_t LoaderFlags;
    uint32_t NumberOfRvaAndSizes;
    // ... DataDirectory would follow but we don't need it for this analysis
} IMAGE_OPTIONAL_HEADER64;

typedef struct _IMAGE_NT_HEADERS64 {
    uint32_t Signature;
    IMAGE_FILE_HEADER FileHeader;
    IMAGE_OPTIONAL_HEADER64 OptionalHeader;
} IMAGE_NT_HEADERS64;

typedef struct _IMAGE_SECTION_HEADER {
    char     Name[8];
    uint32_t VirtualSize;
    uint32_t VirtualAddress;
    uint32_t SizeOfRawData;
    uint32_t PointerToRawData;
    uint32_t PointerToRelocations;
    uint32_t PointerToLinenumbers;
    uint16_t NumberOfRelocations;
    uint16_t NumberOfLinenumbers;
    uint32_t Characteristics;
} IMAGE_SECTION_HEADER;
#pragma pack(pop)

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <pe_file>\n", argv[0]);
        return 1;
    }

    FILE* file = fopen(argv[1], "rb");
    if (!file) {
        printf("Could not open file: %s\n", argv[1]);
        return 1;
    }

    // Read DOS header
    IMAGE_DOS_HEADER dos_header;
    if (fread(&dos_header, 1, sizeof(dos_header), file) != sizeof(dos_header)) {
        printf("Could not read DOS header\n");
        fclose(file);
        return 1;
    }

    if (dos_header.e_magic != 0x5A4D) { // 'MZ'
        printf("Invalid DOS signature\n");
        fclose(file);
        return 1;
    }

    printf("DOS Header:\n");
    printf("  e_magic: 0x%04X\n", dos_header.e_magic);
    printf("  e_lfanew: 0x%08X\n", dos_header.e_lfanew);

    // Go to NT headers
    if (fseek(file, dos_header.e_lfanew, SEEK_SET) != 0) {
        printf("Could not seek to NT headers\n");
        fclose(file);
        return 1;
    }

    // Read NT headers
    IMAGE_NT_HEADERS64 nt_headers;
    if (fread(&nt_headers, 1, sizeof(nt_headers), file) != sizeof(nt_headers)) {
        printf("Could not read NT headers\n");
        fclose(file);
        return 1;
    }

    if (nt_headers.Signature != 0x00004550) { // 'PE\0\0'
        printf("Invalid NT signature: 0x%08X\n", nt_headers.Signature);
        fclose(file);
        return 1;
    }

    printf("\nNT Headers:\n");
    printf("  Signature: 0x%08X\n", nt_headers.Signature);
    printf("  Machine: 0x%04X\n", nt_headers.FileHeader.Machine);
    printf("  NumberOfSections: %d\n", nt_headers.FileHeader.NumberOfSections);
    printf("  SizeOfOptionalHeader: %d\n", nt_headers.FileHeader.SizeOfOptionalHeader);
    printf("  Characteristics: 0x%04X\n", nt_headers.FileHeader.Characteristics);
    
    printf("\nOptional Header:\n");
    printf("  Magic: 0x%04X\n", nt_headers.OptionalHeader.Magic);
    printf("  AddressOfEntryPoint: 0x%08X\n", nt_headers.OptionalHeader.AddressOfEntryPoint);
    printf("  ImageBase: 0x%016lX\n", (unsigned long)nt_headers.OptionalHeader.ImageBase);
    printf("  SectionAlignment: 0x%08X\n", nt_headers.OptionalHeader.SectionAlignment);
    printf("  FileAlignment: 0x%08X\n", nt_headers.OptionalHeader.FileAlignment);
    printf("  SizeOfImage: 0x%08X\n", nt_headers.OptionalHeader.SizeOfImage);
    printf("  SizeOfHeaders: 0x%08X\n", nt_headers.OptionalHeader.SizeOfHeaders);
    printf("  CheckSum: 0x%08X\n", nt_headers.OptionalHeader.CheckSum);
    printf("  Subsystem: %d\n", nt_headers.OptionalHeader.Subsystem);
    printf("  SizeOfCode: 0x%08X\n", nt_headers.OptionalHeader.SizeOfCode);
    printf("  SizeOfInitializedData: 0x%08X\n", nt_headers.OptionalHeader.SizeOfInitializedData);
    printf("  SizeOfUninitializedData: 0x%08X\n", nt_headers.OptionalHeader.SizeOfUninitializedData);

    // Read and print section headers
    printf("\nSection Headers:\n");
    IMAGE_SECTION_HEADER section;
    for (int i = 0; i < nt_headers.FileHeader.NumberOfSections; i++) {
        if (fread(&section, 1, sizeof(section), file) != sizeof(section)) {
            printf("Could not read section header %d\n", i);
            break;
        }
        
        // Ensure section name is null-terminated for printing
        char name[9];
        strncpy(name, section.Name, 8);
        name[8] = '\0';
        
        printf("  [%d] Name: %-8s VA:0x%08X VS:0x%08X PRD:0x%08X SRD:0x%08X\n",
               i, name, section.VirtualAddress, section.VirtualSize, 
               section.PointerToRawData, section.SizeOfRawData);
    }

    fclose(file);
    return 0;
}