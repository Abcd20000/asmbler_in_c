#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "assembler.h"

static char *xstrdup(const char *s)
{
    size_t len = strlen(s) + 1;
    char *copy = malloc(len);
    if (copy)
        memcpy(copy, s, len);
    return copy;
}

#define NOTE                '#'
#define DATA_STRING         ".asciz"
#define ENTRY               ".entry"
#define EXTERN              ".extern"
#define DATA_BYTE           ".db"
#define DATA_WORD           ".dw"
#define DATA_HALF           ".dh"
#define MAX_DATA_SIZE       100
#define MAX_COMMANDS_SIZE   300

#define IS_DATA(a) (\
                strstr((a), DATA_STRING)  ||\
                strstr((a), DATA_BYTE)    ||\
                strstr((a), DATA_WORD)    ||\
                strstr((a), DATA_HALF)      \
            )

const char *data_arr[MAX_DATA_SIZE];
static int commend_arr[MAX_COMMANDS_SIZE];

static int is_string_in_array(const char **arr, const char *line, int arr_size);

int main(void)
{
    int DC = 0;
    int IC = 100;
    int last_data = 0;
    int last_command = 0;
    char *line;

    while ((line = read_line()) != NULL)
    {
        if (line[0] == '\0' || line[0] == NOTE)
        {
            free(line);
            continue;
        }

        if (strcmp(line, EXTERN) == 0)
        {
            if (last_data < MAX_DATA_SIZE)
                data_arr[last_data++] = NULL;
            free(line);
            continue;
        }

        if (strcmp(line, ENTRY) == 0)
        {
            free(line);
            continue;
        }

        if (IS_DATA(line))
        {
            if (last_data >= MAX_DATA_SIZE)
            {
                fprintf(stderr, "Data table full\n");
                free(line);
                return 1;
            }

            if (!is_string_in_array(data_arr, line, last_data))
            {
                data_arr[last_data++] = xstrdup(line);
                DC += 1;
            }

            free(line);
            continue;
        }

        if (last_command < MAX_COMMANDS_SIZE)
            commend_arr[last_command++] = commend_parse_value(line);

        IC += 4;
        free(line);
    }

    printf("First pass complete: IC=%d, DC=%d\n", IC, DC);
    return 0;
}

static int is_string_in_array(const char **arr, const char *line, int arr_size)
{
    for (int i = 0; i < arr_size; i++)
    {
        if (arr[i] != NULL && strcmp(arr[i], line) == 0)
            return 1;
    }
    return 0;
}
