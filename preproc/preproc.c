#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../headers/global.h"
#include "../headers/errors.h"

/* קבועים עבור זיהוי מאקרו */
#define MACRO_START "mcro"
#define MACRO_END "mcroend"

/* קיבולת התחלתית של מערך השורות במאקרו */
#define INITIAL_MACRO_LINES_CAPACITY 10

/* ראש הרשימה המקושרת של המאקרואים */
static Macro *macroListHead = NULL;

/* אם strdup לא מוגדר, מגדירים פונקציה משלנו */
#ifndef HAVE_STRDUP
char* my_strdup(const char* src) {
    char* copy;
    size_t len;
    if (!src) return NULL;
    len = strlen(src) + 1;
    copy = (char*)malloc(len);
    if (copy) memcpy(copy, src, len);
    return copy;
}
#define strdup my_strdup
#endif

/* עותק בטוח של מחרוזת עם בדיקת שגיאות */
static char* safe_strdup(const char* src, const char* context) {
    char* copy;
    if (!src) return NULL;
    copy = strdup(src);
    if (!copy) {
        fprintf(stderr, ERR_MEMORY_ALLOCATION_FAILED, context ? context : "strdup");
    }
    return copy;
}

/* מסיר רווחים וטאבים מתחילת וסוף שורה */
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

/* בודק האם שורה מתחילה בקידומת מסוימת, תוך התעלמות מרווחים בהתחלה */
int startsWith(const char *line, const char *prefix) {
    size_t plen;
    if (!line || !prefix) return 0;
    while (*line == ' ' || *line == '\t') line++;
    plen = strlen(prefix);
    return strncmp(line, prefix, plen) == 0;
}

/* בודק האם שורה היא תחילת הגדרת מאקרו */
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

/* בודק האם שורה היא סוף הגדרת מאקרו */
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

/* כותב שורה לקובץ פלט, מוסיף תו מעבר שורה אם אין כזה */
void writeLine(FILE *output, const char *line) {
    size_t len;
    if (!output || !line || *line == '\0') return;
    fputs(line, output);
    len = strlen(line);
    if (len == 0 || line[len - 1] != '\n') fputc('\n', output);
}

/* מחפש מאקרו לפי שם ברשימה המקושרת */
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

/* מוסיף מאקרו חדש לרשימה, מחזיר מצביע למאקרו או NULL במקרה של כשל */
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

/* מוסיף שורה למאקרו קיים, מטפל בהקצאת זיכרון מחדש במידת הצורך */
void addLineToMacro(Macro* macro, const char* line) {
    int newCap;
    char **newLines;
    if (!macro || !line) return;

    if (macro->lineCount >= macro->lineCapacity) {
        newCap = macro->lineCapacity * 2;
        newLines = (char**)realloc(macro->lines, newCap * sizeof(char*));
        if (!newLines) {
            fprintf(stderr, ERR_MEMORY_ALLOCATION_FAILED, "addLineToMacro (realloc)");
            /* אם נכשל, נמשיך עם הקיבולת הקודמת - לא מושלם אבל מונע דליפת זיכרון */
            return;
        }
        macro->lines = newLines;
        macro->lineCapacity = newCap;
    }

    macro->lines[macro->lineCount] = safe_strdup(line, "addLineToMacro (strdup)");
    if (!macro->lines[macro->lineCount]) {
        /* אם strdup נכשל, נחזיר ונשאיר את המאקרו במצבו הנוכחי */
        return;
    }
    macro->lineCount++;
}

/* משחרר את כל הזיכרון שהוקצה למאקרואים */
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

/* מדפיס את כל המאקרואים שהוגדרו (לצרכי בדיקה) */
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

/* מעבד קריאה למאקרו ומדפיס את תוכנו לקובץ הפלט */
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

/* מעבד שורה רגילה ומדפיס אותה לקובץ הפלט */
void processNormalLine(FILE *output, const char *line) {
    writeLine(output, line);
}

/* מנתח שורה אחת, בודק האם זו הגדרת מאקרו, קריאה למאקרו או שורה רגילה */
void parseLine(char *line, FILE *output, int *inMacro, Macro **currentMacro, int lineNum) {
    char lineCopy[MAX_LENGTH_FILE];
    Macro *macro;
    char *name;

    if (!line) return;

    /* שימוש בטוח יותר ב-snprintf במקום strncpy */
    snprintf(lineCopy, MAX_LENGTH_FILE, "%s", line);
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

/* הפונקציה הראשית שמריצה את הקדם-מעבד על קובץ נתון */
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

    snprintf(outFilename, FILENAME_MAX, "%s.am", filename);

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