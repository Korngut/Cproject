#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../headers/global.h"
#include "../headers/errors.h"
#include "../headers/preproc.h"

/* Keyword marking the start of a macro definition */
#define MACRO_START "mcro"
/* Keyword marking the end of a macro definition */
#define MACRO_END "mcroend"

static Macro *macroListHead = NULL;

/* Helper: Trim leading and trailing whitespace from a line */
void trimWhitespace(char *line) {
    char *start = line;
    char *end;
    while (*start == ' ' || *start == '\t') start++;
    if (start != line) memmove(line, start, strlen(start) + 1);
    end = line + strlen(line) - 1;
    while (end >= line && (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r')) {
        *end = '\0';
        end--;
    }
}

/* Helper: Write a line to output file, ensuring newline */
void writeLine(FILE *output, const char *line) {
    fputs(line, output);
    if (line[strlen(line) - 1] != '\n') fputc('\n', output);
}

/* Helper: Handles writing a macro's content to the output file.
 * Params:
 *   output - output file pointer
 *   macroName - name of the macro to expand
 *   lineNum - current line number (for error reporting)
 */
void processMacroCall(FILE *output, const char *macroName, int lineNum) {
    Macro *macro = findMacro(macroName);
    if (!macro) {
        fprintf(stderr, ERR_MACRO_UNDEFINED, macroName, lineNum);
        return;
    }
    for (int i = 0; i < macro->lineCount; ++i) {
        writeLine(output, macro->lines[i]);
    }
}

/* Helper: Writes a regular (non-macro) line to the output file.
 * Params:
 *   output - output file pointer
 *   line - the line to write
 */
void processNormalLine(FILE *output, const char *line) {
    writeLine(output, line);
}

/* Helper: Parses a single line, updating macro state and handling macro definitions/expansions.
 * Params:
 *   line - the current line (may be modified)
 *   output - output file pointer
 *   inMacro - pointer to macro state flag (0/1)
 *   currentMacro - pointer to current macro being defined
 *   lineNum - current line number
 */
void parseLine(char *line, FILE *output, int *inMacro, Macro **currentMacro, int lineNum) {
    char lineCopy[MAX_LENGTH_FILE];
    strncpy(lineCopy, line, MAX_LENGTH_FILE - 1);
    lineCopy[MAX_LENGTH_FILE - 1] = '\0';
    trimWhitespace(lineCopy);

    if (!(*inMacro) && isMacroDefinitionStart(lineCopy)) {
        /* Macro definition start */
        char *name = lineCopy + strlen(MACRO_START);
        while (*name == ' ' || *name == '\t') name++;
        if (*name == '\0') {
            fprintf(stderr, ERR_MACRO_NAME_MISSING, lineNum);
            return;
        }
        if (!addMacro(name)) {
            fprintf(stderr, ERR_MACRO_REDEFINED, name, lineNum);
            return;
        }
        *currentMacro = macroListHead;
        *inMacro = 1;
        return;
    }

    if (*inMacro) {
        if (isMacroDefinitionEnd(lineCopy)) {
            *inMacro = 0;
            *currentMacro = NULL;
        } else {
            addLineToMacro(*currentMacro, line);
        }
        return;
    }

    /* Not in macro definition: check for macro call */
    Macro *macro = findMacro(lineCopy);
    if (macro) {
        processMacroCall(output, lineCopy, lineNum);
    } else {
        processNormalLine(output, line);
    }
}

/* Main pre-assembler function: reads each line and delegates to helpers */
void runPreAssembler(const char *filename) {
    FILE *input = fopen(filename, "r");
    if (!input) {
        fprintf(stderr, ERR_CANNOT_OPEN_INPUT_FILE, filename);
        return;
    }

    char outFilename[FILENAME_MAX];
    snprintf(outFilename, sizeof(outFilename), "%s.am", filename);
    FILE *output = fopen(outFilename, "w");
    if (!output) {
        fprintf(stderr, ERR_CANNOT_OPEN_OUTPUT_FILE, outFilename);
        fclose(input);
        return;
    }

    char line[MAX_LENGTH_FILE];
    int inMacro = 0;
    Macro *currentMacro = NULL;
    int lineNum = 0;

    while (fgets(line, sizeof(line), input)) {
        lineNum++;
        parseLine(line, output, &inMacro, &currentMacro, lineNum);
    }

    if (inMacro) {
        fprintf(stderr, ERR_MACRO_NOT_CLOSED);
    }

    fclose(input);
    fclose(output);
    freeMacros();
}