#ifndef ERRORS_H
#define ERRORS_H

/* Error message when input file cannot be opened */
#define ERR_CANNOT_OPEN_INPUT_FILE "Error: Cannot open input file %s\n"

/* Error message when output file cannot be opened */
#define ERR_CANNOT_OPEN_OUTPUT_FILE "Error: Cannot open output file %s\n"

/* Error message when a macro is redefined */
#define ERR_MACRO_REDEFINED "Error: Macro '%s' redefined at line %d\n"

/* Error message when a macro name is missing */
#define ERR_MACRO_NAME_MISSING "Error: Macro name missing at line %d\n"

/* Error message when a macro definition is not closed */
#define ERR_MACRO_NOT_CLOSED "Error: Macro definition not closed at end of file\n"

/* Error message when memory allocation fails */
#define ERR_MEMORY_ALLOCATION_FAILED "Error: Memory allocation failed in %s\n"

/* Error message when a macro is called but not defined */
#define ERR_MACRO_UNDEFINED "Error: Macro '%s' called but not defined at line %d\n"

/* Optional integer error codes */
#define ERR_CODE_FILE_OPEN_FAILED 1
#define ERR_CODE_MACRO_REDEFINED 2
#define ERR_CODE_MEMORY_ALLOCATION 3

#endif /* ERRORS_H */