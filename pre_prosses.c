#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "assembler.h"

#define MAX_MACRO_SIZE 100
#define WRITE_COMMENT '#'
#define MACRO_FILE "macro.txt"
const char *MACRO_NAME = "mcro";
const char *MACRO_TABLE[MAX_MACRO_SIZE];
const char *END_MACRO = "mcroend";

int is_macro(const char *line);
void write_macro(const char *line);

int main(void)
{
    int macro_table_index = 0;
    char *line;

    while (1)
    {
        line = read_line();
        if (line == NULL)
            break;

        if (is_macro(line))
        {
            free(line);
            continue;
        }

        if (strcmp(line, MACRO_NAME) == 0)
        {
            char *current_line = read_line();
            if (current_line == NULL)
            {
                free(line);
                break;
            }

            if (macro_table_index < MAX_MACRO_SIZE)
                MACRO_TABLE[macro_table_index++] = current_line;
            else
            {
                fprintf(stderr, "Macro table is full. Cannot add more macros.\n");
                free(current_line);
            }

            free(line);
            continue;
        }

        if (strcmp(line, END_MACRO) == 0)
        {
            del_line(line);
            free(line);
            continue;
        }

        free(line);
        break;
    }

    return 0;
}

int is_macro(const char *line)
{
    for (int i = 0; line[i] != '\0'; i++)
    {
        if (line[i] == WRITE_COMMENT)
        {
            write_macro(line);
            return 1;
        }
    }
    return 0;
}

void write_macro(const char *line)
{
    FILE *file = fopen(MACRO_FILE, "a");
    if (!file)
    {
        perror("fopen");
        exit(1);
    }

    fwrite(line, sizeof(char), strlen(line), file);
    fclose(file);
}
