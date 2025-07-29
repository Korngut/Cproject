#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../headers/global.h"
#include "../headers/errors.h"
#include "../headers/preproc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../headers/global.h"
#include "../headers/errors.h"
#include "../headers/preproc.h"

#define MACRO_START "mcro"
#define MACRO_END "mcroend"
#define INITIAL_MACRO_LINES_CAPACITY 10

static Macro *macroListHead = NULL;

/* אם strdup לא מוגדר, מגדירים פונקציה משלנו */
#ifndef HAVE_STRDUP
char* my_strdup(const char* src) {
    char* copy;
    size_t len;
    if (!src) return NULL;
    len = strlen(src) + 1;//hhhh
    copy = (char*)malloc(len);
    if (copy) memcpy(copy, src, len);
    return copy;
}
#define strdup my_strdup
#endif

static char* safe_strdup(const char* src, const char* context) {
    char* copy;
    if (!src) return NULL;
    copy = strdup(src);
    if (!copy) {
        fprintf(stderr, ERR_MEMORY_ALLOCATION_FAILED, context ? context : "strdup");
    }
    return copy;
}

void trimWhitespace(char *line) {
    char *start;
    char *end;

    if (!line) return;

    start = line;
    while (*start == ' ' || *start == '\t') start++;
    if (start != line) memmove(line, start, strlen(start) + 1);
    if (*line == '\0') return;

    end = line + strlen(line) - 1;
    while (end >= line && (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r')) {
        *end = '\0';
        end--;
    }
}

int startsWith(const char *line, const char *prefix) {
    size_t plen;
    if (!line || !prefix) return 0;
    while (*line == ' ' || *line == '\t') line++;
    plen = strlen(prefix);
    return strncmp(line, prefix, plen) == 0;
}

int isMacroDefinitionStart(const char *line) {
    size_t len;
    if (!line) return 0;
    while (*line == ' ' || *line == '\t') line++;
    len = strlen(MACRO_START);
    if (strncmp(line, MACRO_START, len) != 0) return 0;
    line += len;
    if (*line == '\0' || *line == ' ' || *line == '\t' || *line == '\n' || *line == '\r')
        return 1;
    return 0;
}

int isMacroDefinitionEnd(const char *line) {
    size_t len;
    if (!line) return 0;
    while (*line == ' ' || *line == '\t') line++;
    len = strlen(MACRO_END);
    if (strncmp(line, MACRO_END, len) != 0) return 0;
    line += len;
    if (*line == '\0' || *line == ' ' || *line == '\t' || *line == '\n' || *line == '\r')
        return 1;
    return 0;
}

void writeLine(FILE *output, const char *line) {
    size_t len;
    if (!output || !line || *line == '\0') return;
    fputs(line, output);
    len = strlen(line);
    if (len == 0 || line[len - 1] != '\n') fputc('\n', output);
}

Macro* findMacro(const char* name) {
    Macro *curr;
    if (!name) return NULL;
    curr = macroListHead;
    while (curr) {
        if (strcmp(curr->name, name) == 0) return curr;
        curr = curr->next;
    }
    return NULL;
}

Macro* addMacro(const char* name) {
    Macro *macro;
    if (!name || findMacro(name)) return NULL;

    macro = (Macro*)malloc(sizeof(Macro));
    if (!macro) {
        fprintf(stderr, ERR_MEMORY_ALLOCATION_FAILED, "addMacro (malloc)");
        return NULL;
    }

    macro->name = safe_strdup(name, "addMacro (strdup)");
    if (!macro->name) {
        free(macro);
        return NULL;
    }

    macro->lines = (char**)malloc(INITIAL_MACRO_LINES_CAPACITY * sizeof(char*));
    if (!macro->lines) {
        fprintf(stderr, ERR_MEMORY_ALLOCATION_FAILED, "addMacro (lines malloc)");
        free(macro->name);
        free(macro);
        return NULL;
    }

    macro->lineCount = 0;
    macro->lineCapacity = INITIAL_MACRO_LINES_CAPACITY;
    macro->next = macroListHead;
    macroListHead = macro;
    return macro;
}

void addLineToMacro(Macro* macro, const char* line) {
    int newCap;
    char **newLines;
    if (!macro || !line) return;

    if (macro->lineCount >= macro->lineCapacity) {
        newCap = macro->lineCapacity * 2;
        newLines = (char**)realloc(macro->lines, newCap * sizeof(char*));
        if (!newLines) {
            fprintf(stderr, ERR_MEMORY_ALLOCATION_FAILED, "addLineToMacro (realloc)");
            return;
        }
        macro->lines = newLines;
        macro->lineCapacity = newCap;
    }

    macro->lines[macro->lineCount] = safe_strdup(line, "addLineToMacro (strdup)");
    if (!macro->lines[macro->lineCount]) return;
    macro->lineCount++;
}

void freeMacros(void) {
    Macro *curr;
    Macro *next;
    int i;

    curr = macroListHead;
    while (curr) {
        next = curr->next;
        free(curr->name);
        for (i = 0; i < curr->lineCount; ++i) {
            free(curr->lines[i]);
        }
        free(curr->lines);
        free(curr);
        curr = next;
    }
    macroListHead = NULL;
}

void printAllMacros(void) {
    Macro *curr;
    int count = 0;
    int i;

    curr = macroListHead;
    while (curr) {
        printf("Macro: %s\n", curr->name);
        for (i = 0; i < curr->lineCount; ++i) {
            printf("  %s", curr->lines[i]);
            if (curr->lines[i][strlen(curr->lines[i]) - 1] != '\n') {
                printf("\n");
            }
        }
        curr = curr->next;
        count++;
    }
    printf("Total macros: %d\n", count);
}

void processMacroCall(FILE *output, const char *macroName, int lineNum) {
    Macro *macro;
    int i;

    macro = findMacro(macroName);
    if (!macro) {
        fprintf(stderr, ERR_MACRO_UNDEFINED, macroName, lineNum);
        return;
    }
    for (i = 0; i < macro->lineCount; ++i) {
        writeLine(output, macro->lines[i]);
    }
}

void processNormalLine(FILE *output, const char *line) {
    writeLine(output, line);
}

void parseLine(char *line, FILE *output, int *inMacro, Macro **currentMacro, int lineNum) {
    char lineCopy[MAX_LENGTH_FILE];
    Macro *macro;
    char *name;

    if (!line) return;

    strncpy(lineCopy, line, MAX_LENGTH_FILE - 1);
    lineCopy[MAX_LENGTH_FILE - 1] = '\0';
    trimWhitespace(lineCopy);

    if (!(*inMacro) && isMacroDefinitionStart(lineCopy)) {
        name = lineCopy + strlen(MACRO_START);
        while (*name == ' ' || *name == '\t') name++;
        if (*name == '\0') {
            fprintf(stderr, ERR_MACRO_NAME_MISSING, lineNum);
            return;
        }
        *currentMacro = addMacro(name);
        if (!*currentMacro) {
            fprintf(stderr, ERR_MACRO_REDEFINED, name, lineNum);
            return;
        }
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

    macro = findMacro(lineCopy);
    if (macro) {
        processMacroCall(output, lineCopy, lineNum);
    } else {
        processNormalLine(output, line);
    }
}

void runPreAssembler(const char *filename) {
    FILE *input;
    char outFilename[FILENAME_MAX];
    FILE *output;
    char line[MAX_LENGTH_FILE];
    int inMacro;
    Macro *currentMacro;
    int lineNum;

    if (!filename) return;

    input = fopen(filename, "r");
    if (!input) {
        fprintf(stderr, ERR_CANNOT_OPEN_INPUT_FILE, filename);
        return;
    }

    /* C90 אין snprintf, משתמשים ב-sprintf */
    sprintf(outFilename, "%s.am", filename);

    output = fopen(outFilename, "w");
    if (!output) {
        fprintf(stderr, ERR_CANNOT_OPEN_OUTPUT_FILE, outFilename);
        fclose(input);
        return;
    }

    inMacro = 0;
    currentMacro = NULL;
    lineNum = 0;

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
