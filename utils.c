#include "defs.h"

char *str_trim(char *s)
{
    char *end;
    while (isspace((unsigned char)*s))
        s++;
    if (*s == '\0')
        return s;
    end = s + strlen(s) - 1;
    while (end > s && isspace((unsigned char)*end))
        end--;
    *(end + 1) = '\0';
    return s;
}

int is_valid_label(const char *s)
{
    int i;
    if (!isalpha((unsigned char)s[0]))
        return 0;
    for (i = 1; s[i] != '\0'; i++)
    {
        if (!isalnum((unsigned char)s[i]) && s[i] != '_')
            return 0;
        if (i >= MAX_LABEL_LEN - 1)
            return 0;
    }
    return 1;
}

int is_register(const char *token, int *out_reg_num)
{
    int reg_num;
    char *end_ptr;

    if (token == NULL || token[0] != '$')
        return 0;

    reg_num = (int)strtol(token + 1, &end_ptr, 10);
    if (*end_ptr != '\0' || reg_num < 0 || reg_num >= NUM_REGISTERS)
        return 0;

    *out_reg_num = reg_num;
    return 1;
}

void report_error(const char *filename, int line_num, const char *msg)
{
    fprintf(stderr, "ERROR %s:%d - %s\n", filename, line_num, msg);
    error_count++;
}
