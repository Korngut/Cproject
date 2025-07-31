#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "preproc/preproc.h"     // עדכון נתיב ההכללה ל-preproc.h
#include "firstPass/firstPass.h" // הנתיב ל-firstPass.h

int main() {
    const char *inputAsm =
            "mcro testMacro\n"
            "mov r1, r2\n"
            "mcroend\n"
            "testMacro\n"
            "LABEL1: .data 5, 10, 15\n"
            ".string \"Hello\"\n"
            "extern EXT_LABEL\n"
            ".entry ENTRY_LABEL\n"
            "mov r3, r4\n";
    const char *filename = "test_input.asm";

    // יוצרים קובץ קלט עם הטקסט
    FILE *file = fopen(filename, "w");
    if (!file) {
        fprintf(stderr, "Cannot create input file\n");
        return 1;
    }
    fprintf(file, "%s", inputAsm);
    fclose(file);

    printf("Running pre-assembler on %s...\n", filename);
    runPreAssembler(filename);

    printf("Running first pass on %s.am...\n", filename);
    int success = runFirstPass(filename);

    if (success) {
        printf("First pass completed successfully!\n");
    } else {
        printf("First pass failed!\n");
    }

    // הדפסת תוכן הקובץ של המעבר הראשון
    char amFilename[FILENAME_MAX];
    snprintf(amFilename, FILENAME_MAX, "%s.am", filename);

    FILE *amFile = fopen(amFilename, "r");
    if (!amFile) {
        fprintf(stderr, "Cannot open %s for reading\n", amFilename);
        return 1;
    }

    printf("Contents of %s:\n", amFilename);
    char line[1024];
    while (fgets(line, sizeof(line), amFile)) {
        printf("%s", line);
    }
    fclose(amFile);

    return 0;
}
