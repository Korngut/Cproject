#ifndef FIRST_PASS_H
#define FIRST_PASS_H
#include <stdio.h>

/* סוגי סמלים */
typedef enum {
    SYMBOL_CODE,
    SYMBOL_DATA,
    SYMBOL_EXTERNAL,
    SYMBOL_ENTRY
} SymbolType;

/* מבנה לייצוג סמל בטבלת הסמלים */
typedef struct Symbol {
    char name[32];      /* שם הסמל */
    int address;        /* כתובת הסמל בזיכרון */
    SymbolType type;    /* סוג הסמל (code, data, entry, external) */
    struct Symbol *next;/* מצביע לאיבר הבא ברשימה המקושרת */
} Symbol;

/* פונקציית המעבר הראשון */
int runFirstPass(const char *filename);

#endif /* FIRST_PASS_H */