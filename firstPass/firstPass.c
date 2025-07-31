#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "firstPass.h"
#include "errorsInFirstPass.h"
#include "../headers/global.h"

/* קבועים עבור מוני הזיכרון */
#define IC_START_ADDRESS 100

/* ראש טבלת הסמלים */
static Symbol *symbolTableHead = NULL;

/* הפונקציה מוסיפה סמל לטבלת הסמלים */
static int addSymbol(const char *name, int address, SymbolType type) {
    Symbol *newSymbol, *curr;

    curr = symbolTableHead;
    while (curr) {
        if (strcmp(curr->name, name) == 0) return 0; /* הסמל כבר קיים */
        curr = curr->next;
    }

    newSymbol = (Symbol *)malloc(sizeof(Symbol));
    if (!newSymbol) {
        fprintf(stderr, ERR_MEMORY_ALLOCATION_FAILED);
        return -1;
    }

    strcpy(newSymbol->name, name);
    newSymbol->address = address;
    newSymbol->type = type;
    newSymbol->next = symbolTableHead;
    symbolTableHead = newSymbol;
    return 1;
}

/* משחררת את טבלת הסמלים מהזיכרון */
static void freeSymbolTable() {
    Symbol *curr = symbolTableHead, *temp;
    while (curr) {
        temp = curr;
        curr = curr->next;
        free(temp);
    }
    symbolTableHead = NULL;
}

/* בודקת האם השורה מכילה הגדרת תווית ומחזירה את שם התווית אם יש */
static char* getLabel(char *line) {
    char *colon = strchr(line, ':');
    if (colon) {
        *colon = '\0';
        return line;
    }
    return NULL;
}

/* מנתחת שורה, מעדכנת מוני זיכרון וטבלת סמלים */
static int processLine(char *line, int *IC, int *DC, int lineNum) {
    char *label = getLabel(line);
    char *trimmedLine = line;
    char firstToken[32];
    int tokenCount, operandSize;

    if (label) {
        trimmedLine += strlen(label) + 1;
        while (isspace(*trimmedLine)) trimmedLine++;
    }

    tokenCount = sscanf(trimmedLine, "%s", firstToken);
    if (tokenCount <= 0) return 1; /* שורה ריקה */

    if (label) {
        if (strlen(label) > 31 || !isalpha(label[0])) {
            fprintf(stderr, ERR_INVALID_LABEL, lineNum, label);
            return 0;
        }
        if (addSymbol(label, *IC, SYMBOL_CODE) == 0) {
            fprintf(stderr, ERR_SYMBOL_ALREADY_DEFINED, lineNum, label);
            return 0;
        }
    }

    if (firstToken[0] == '.') {
        if (strcmp(firstToken, ".data") == 0) {
            if (label) {
                Symbol *s = symbolTableHead;
                while (s && strcmp(s->name, label) != 0) s = s->next;
                if(s) {
                    s->type = SYMBOL_DATA;
                    s->address = *DC;
                }
            }
            trimmedLine += strlen(".data");
            operandSize = 0;
            while (sscanf(trimmedLine, "%*s%n", &operandSize) == 0 && operandSize > 0) {
                *DC += 1;
                trimmedLine += operandSize;
            }
        } else if (strcmp(firstToken, ".string") == 0) {
            if (label) {
                Symbol *s = symbolTableHead;
                while (s && strcmp(s->name, label) != 0) s = s->next;
                if(s) {
                    s->type = SYMBOL_DATA;
                    s->address = *DC;
                }
            }
            trimmedLine += strlen(".string");
            while (isspace(*trimmedLine)) trimmedLine++;
            if (*trimmedLine == '"') {
                trimmedLine++;
                while (*trimmedLine && *trimmedLine != '"') {
                    *DC += 1;
                    trimmedLine++;
                }
                *DC += 1; /* עבור '\0' */
            }
        } else if (strcmp(firstToken, ".extern") == 0) {
            trimmedLine += strlen(".extern");
            while (isspace(*trimmedLine)) trimmedLine++;
            sscanf(trimmedLine, "%s", firstToken);
            addSymbol(firstToken, 0, SYMBOL_EXTERNAL);
            if (label) {
                fprintf(stderr, ERR_INVALID_SYNTAX, lineNum);
                return 0;
            }
        } else if (strcmp(firstToken, ".entry") == 0) {
            if (label) {
                fprintf(stderr, ERR_INVALID_SYNTAX, lineNum);
                return 0;
            }
            /* הטיפול ב-.entry מתבצע בשלב מאוחר יותר */
        }
    } else { /* פקודת אסמבלי רגילה */
        if (label && symbolTableHead->type != SYMBOL_CODE) {
            fprintf(stderr, ERR_INVALID_SYNTAX, lineNum);
            return 0;
        }

        *IC += 1; /* מילת פקודה */
        char operand1[MAX_LENGTH_FILE], operand2[MAX_LENGTH_FILE];
        int scanCount = sscanf(trimmedLine, "%*s %[^,],%s", operand1, operand2);
        if (scanCount > 0) {
            *IC += 1;
            if (scanCount > 1) *IC += 1;
        }
    }

    return 1;
}

/* הפונקציה הראשית של המעבר הראשון */
int runFirstPass(const char *filename) {
    FILE *inputFile;
    char line[MAX_LENGTH_FILE];
    int IC = IC_START_ADDRESS, DC = 0, lineNum = 0, passSuccess = 1;
    char amFilename[FILENAME_MAX];

    snprintf(amFilename, FILENAME_MAX, "%s.am", filename);

    inputFile = fopen(amFilename, "r");
    if (!inputFile) {
        fprintf(stderr, ERR_CANNOT_OPEN_INPUT_FILE, amFilename);
        return 0;
    }

    freeSymbolTable();

    while (fgets(line, sizeof(line), inputFile)) {
        lineNum++;
        if (!processLine(line, &IC, &DC, lineNum)) {
            passSuccess = 0;
        }
    }

    fclose(inputFile);

    Symbol *curr = symbolTableHead;
    while (curr) {
        if (curr->type == SYMBOL_DATA) {
            curr->address += IC;
        }
        curr = curr->next;
    }

    if (!passSuccess) {
        freeSymbolTable();
        return 0;
    }

    return 1;
}
