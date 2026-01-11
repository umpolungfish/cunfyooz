#include <keystone/keystone.h>
#include <stdio.h>

int main() {
    ks_engine *ks;
    unsigned char *encode;
    size_t size, count;

    if (ks_open(KS_ARCH_X86, KS_MODE_64, &ks) != KS_ERR_OK) {
        fprintf(stderr, "Failed to initialize Keystone\n");
        return 1;
    }

    ks_option(ks, KS_OPT_SYNTAX, KS_OPT_SYNTAX_INTEL);

    const char *assembly = "push qword ptr [rbx + 0x10]";
    if (ks_asm(ks, assembly, 0, &encode, &size, &count) != KS_ERR_OK) {
        fprintf(stderr, "Failed to assemble '%s': %s\n", assembly, ks_strerror(ks_errno(ks)));
        ks_close(ks);
        return 1;
    }

    printf("Assembled: ");
    for (size_t i = 0; i < size; i++) {
        printf("%02x ", encode[i]);
    }
    printf("\n");

    ks_free(encode);
    ks_close(ks);
    return 0;
}
