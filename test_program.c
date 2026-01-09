#include <stdio.h>
#include <windows.h>

int main() {
    printf("Hello from original program!\\n");
    MessageBox(NULL, "This is a test program for cunfyooz", "Test Program", MB_OK);
    printf("Program execution completed.\\n");
    return 0;
}