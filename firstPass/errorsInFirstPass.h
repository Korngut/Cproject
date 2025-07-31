#ifndef ERRORS_FIRST_PASS_H
#define ERRORS_FIRST_PASS_H

#define ERR_SYMBOL_ALREADY_DEFINED "שגיאה (שורה %d): הסמל '%s' כבר הוגדר.\n"
#define ERR_INVALID_LABEL "שגיאה (שורה %d): שם הסמל '%s' אינו תקין.\n"
#define ERR_INVALID_SYNTAX "שגיאה (שורה %d): שגיאה תחבירית.\n"
#define ERR_LABEL_TOO_LONG "שגיאה (שורה %d): שם הסמל '%s' ארוך מדי (מעל 31 תווים).\n"
#define ERR_ENTRY_NOT_FOUND "שגיאה (שורה %d): סמל 'entry' '%s' לא הוגדר בקובץ.\n"
#define ERR_INVALID_OPERAND_COUNT "שגיאה (שורה %d): מספר האופרנדים אינו תקין עבור הפקודה.\n"
#define ERR_INVALID_ADDRESSING_MODE "שגיאה (שורה %d): שיטת מיעון לא תקינה עבור האופרנד.\n"
#define ERR_MEMORY_ALLOCATION_FAILED "שגיאה: כשל בהקצאת זיכרון.\n"
#define ERR_CANNOT_OPEN_INPUT_FILE "Error: Cannot open input file %s\n"


#endif /* ERRORS_FIRST_PASS_H */