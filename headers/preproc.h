#ifndef PRE_ASSEMBLER_H
#define PRE_ASSEMBLER_H

#include <stdio.h> /* For FILE* */

/*
 * Macro struct: Represents a macro definition.
 * - name: Macro name.
 * - lines: Array of strings, each string is a line in the macro body.
 * - lineCount: Number of lines in the macro.
 * - lineCapacity: Allocated capacity for lines.
 * - next: Pointer to the next macro in the linked list.
 */
typedef struct Macro {
    char *name;
    char **lines;
    int lineCount;
    int lineCapacity;
    struct Macro *next;
} Macro;

/*
 * Runs the pre-assembler on the given file.
 * Handles macro definitions and expansions.
 */
void runPreAssembler(const char* filename);

/*
 * Removes leading and trailing whitespace from the given line.
 */
void trimWhitespace(char* line);

/*
 * Returns nonzero if 'line' starts with 'prefix', zero otherwise.
 */
int startsWith(const char* line, const char* prefix);

/*
 * Returns nonzero if the line marks the start of a macro definition ("mcro").
 */
int isMacroDefinitionStart(const char* line);

/*
 * Returns nonzero if the line marks the end of a macro definition ("mcroend").
 */
int isMacroDefinitionEnd(const char* line);

/*
 * Writes a line to the output file, ensuring it ends with a newline.
 */
void writeLine(FILE* output, const char* line);

/*
 * Finds and returns a pointer to the macro with the given name, or NULL if not found.
 */
Macro* findMacro(const char* name);

/*
 * Adds a new macro with the given name to the macro list.
 * Returns pointer to Macro, or NULL if the macro already exists or on failure.
 */
Macro* addMacro(const char* name);

/*
 * Adds a line to the specified macro's content.
 */
void addLineToMacro(Macro* macro, const char* line);

/*
 * Frees all memory used by the macro list.
 */
void freeMacros(void);

/*
 * Prints all macros and their contents (for debugging).
 */
void printAllMacros(void);

#endif /* PRE_ASSEMBLER_H */