#include <stdio.h>
#include <string.h>

#include "../headers/global.h"

char *mcro_save(FILE *file, fpos_t *pos, int *line_counter) {
    char str[MAX_LENGTH_FILE];
    static char mcro_data[MAX_LENGTH_FILE];
    int mcro_length = 0;

    if (fsetpos(file, pos) != 0) {
        return NULL;
    }

    mcro_data[0] = '\0';

    while (fgets(str, MAX_LENGTH_FILE, file)) {
        if (strcmp(str, "endmcro\n") == 0 || strcmp(str, "endmcro\r\n") == 0) {
            break;
        }

        if (strstr(str, "endmcro") && strncmp(str, "endmcro", 7) == 0) {
            // error
            return NULL;
        }

        (*line_counter)++;

        int len = strlen(str);
        if (mcro_length + len >= MAX_LENGTH_FILE) {
            //error
            return NULL;
        }

        strcpy(mcro_data + mcro_length, str);
        mcro_length += len;
    }

    return mcro_data;
}

