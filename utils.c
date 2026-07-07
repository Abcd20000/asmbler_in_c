/*
 * utils.c  –  Small utility helpers shared across all phases.
 */
#include "defs.h"

/*
 * str_trim  –  Remove leading and trailing whitespace in-place.
 *              Returns the pointer to the trimmed string (same buffer).
 */
char *str_trim(char *s)
{
    char *end;

    /* skip leading whitespace */
    while (isspace((unsigned char)*s))
        s++;

    if (*s == '\0')
        return s;

    /* trim trailing whitespace */
    end = s + strlen(s) - 1;
    while (end > s && isspace((unsigned char)*end))
        end--;
    *(end + 1) = '\0';

    return s;
}

/*
 * is_valid_label  –  Returns 1 if 's' is a syntactically valid label name:
 *                    starts with a letter, followed by letters/digits,
 *                    maximum MAX_LABEL_LEN-1 characters.
 */
int is_valid_label(const char *s)
{
    int i;
    if (!isalpha((unsigned char)s[0]))
        return 0;
    for (i = 1; s[i] != '\0'; i++)
    {
        if (!isalnum((unsigned char)s[i]))
            return 0;
        if (i >= MAX_LABEL_LEN - 1)
            return 0;
    }
    return 1;
}

/*
 * is_register  –  If 'token' looks like "$N" (0-31), write N into
 *                 *out_reg_num and return 1; otherwise return 0.
 */
int is_register(const char *token, int *out_reg_num)
{
    int reg_num;
    char *end_ptr;

    if (token == NULL || token[0] != '$')
        return 0;

    reg_num = (int)strtol(token + 1, &end_ptr, 10);

    if (*end_ptr != '\0')           /* trailing garbage         */
        return 0;
    if (reg_num < 0 || reg_num >= NUM_REGISTERS)
        return 0;

    *out_reg_num = reg_num;
    return 1;
}

/*
 * report_error  –  Print a formatted error to stderr.
 */
void report_error(const char *filename, int line_num, const char *msg)
{
    fprintf(stderr, "ERROR %s:%d – %s\n", filename, line_num, msg);
    g_error_count++;
}
