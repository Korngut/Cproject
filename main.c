#include <stdio.h>
#include <stdlib.h>

void runPreAssembler(const char *filename);

const char *testContent =
"mcro MyCodeBlock\n"
"    load R1\n"
"    add R2\n"
"    store R3\n"
"mcroend\n"
".data\n"
"var1: .data 10\n"
"var2: .data 20\n"
".entry processData\n"
"processData:\n"
"    MyCodeBlock\n"
"    stop\n";

int main() {
    const char *tempFileName = "temp_test.asm";
    FILE *tempFile = fopen(tempFileName, "w");
    if (!tempFile) {
        fprintf(stderr, "Cannot create temp file\n");
        return 1;
    }

    fputs(testContent, tempFile);
    fclose(tempFile);

    printf("Starting preassembler...\n");
    runPreAssembler(tempFileName);
    printf("Preassembler finished.\n");

    // הדפסת הקובץ שנוצר
    FILE *outputFile = fopen("temp_test.asm.am", "r");
    if (outputFile) {
        char ch;
        printf("\n---- Preassembler output ----\n");
        while ((ch = fgetc(outputFile)) != EOF) {
            putchar(ch);
        }
        fclose(outputFile);
        printf("\n---- End of output ----\n");
    } else {
        fprintf(stderr, "Cannot open output file for reading\n");
    }

    return 0;
}
