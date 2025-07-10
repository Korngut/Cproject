#include <stdio.h>
#include <string.h>
#include "../headers/global.h"


char *mcro_save(FILE *file, fpos_t *pos, int *line_counter) {
    char str[MAX_LENGTH_FILE];
    static char mcro_data[MAX_LENGTH_FILE];
    int mcro_length = 0;

    if (fsetpos(file, pos) != 0) {
        fprintf(stderr, "Error: Failed to set file position.\n");
        return NULL;
    }

    mcro_data[0] = '\0';

    while (fgets(str, sizeof(str), file)) {
        if (strncmp(str, "endmcro", 7) == 0 &&
            (str[7] == '\n' || str[7] == '\r' || str[7] == '\0')) {
            break;
            }

        if (strstr(str, "endmcro") && strncmp(str, "endmcro", 7) != 0) {
            fprintf(stderr, "Error: 'endmcro' found in the middle of a line.\n");
            return NULL;
        }

        (*line_counter)++;

        int len = strlen(str);
        if (mcro_length + len >= MAX_LENGTH_FILE) {
            fprintf(stderr, "Error: Macro too long.\n");
            return NULL;
        }

        strncat(mcro_data, str, MAX_LENGTH_FILE - mcro_length - 1);
        mcro_length += len;
    }

    return mcro_data;
}

char nextcINT() {

    c = fgetc(stdin);
    return c;
}