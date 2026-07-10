/* Phase 0: Macro pre-processing. */
#include "defs.h"

#define MCRO_START  "mcro"
#define MCRO_END    "mcroend"

// Macro storage
typedef struct {
    char  name[MAX_MACRO_NAME_LEN];
    char  lines[MAX_MACRO_LINES][MAX_LINE_LEN];
    int   line_count;
} Macro;

static Macro   s_macros[MAX_MACROS];
static int     s_macro_count = 0;

// Forward declarations
static Macro  *find_macro(const char *name);
static int     add_macro(const char *name);

/* pre_process: expands macros and writes to out_path */
int pre_process(const char *src_path, const char *out_path)
{
    FILE *src_fp;
    FILE *out_fp;
    char  raw_line[MAX_LINE_LEN];
    char  line[MAX_LINE_LEN];
    int   line_num          = 0;
    int   inside_macro_def  = 0;
    int   current_macro_idx = NO_MACRO_INDEX;
    Macro *macro_ptr;
    char  *first_token;
    char  *rest;
    char   buf[MAX_LINE_LEN];

    src_fp = fopen(src_path, "r");
    if (src_fp == NULL)
    {
        perror(src_path);
        return 1;
    }

    out_fp = fopen(out_path, "w");
    if (out_fp == NULL)
    {
        perror(out_path);
        fclose(src_fp);
        return 1;
    }

    while (fgets(raw_line, MAX_LINE_LEN, src_fp) != NULL)
    {
        line_num++;
        strncpy(line, raw_line, MAX_LINE_LEN - 1);
        line[MAX_LINE_LEN - 1] = '\0';

        /* remove trailing newline */
        {
            int len = (int)strlen(line);
            if (len > 0 && line[len - 1] == '\n')
                line[len - 1] = '\0';
        }

        //first token
        strncpy(buf, line, MAX_LINE_LEN - 1);
        buf[MAX_LINE_LEN - 1] = '\0';
        first_token = strtok(buf, " \t");
        rest        = strtok(NULL, "");   // everything after first token

        // empty or comment line
        if (first_token == NULL || first_token[0] == ';')
        {
            if (!inside_macro_def)
                fprintf(out_fp, "%s\n", line);
            continue;
        }

        // macsos mcroend 
        if (strcmp(first_token, MCRO_END) == 0)
        {
            if (!inside_macro_def)
            {
                fprintf(stderr, "ERROR %s:%d – mcroend without mcro\n",
                        src_path, line_num);
                error_count++;
            }
            inside_macro_def  = 0;
            current_macro_idx = NO_MACRO_INDEX;
            continue;
        }

        // mcro <name> 
        if (strcmp(first_token, MCRO_START) == 0)
        {
            char *macro_name = rest ? str_trim(rest) : NULL;

            if (inside_macro_def)
            {
                fprintf(stderr, "ERROR %s:%d – nested mcro not allowed\n",
                        src_path, line_num);
                error_count++;
                continue;
            }
            if (macro_name == NULL || !is_valid_label(macro_name))
            {
                fprintf(stderr, "ERROR %s:%d – invalid macro name\n",
                        src_path, line_num);
                error_count++;
                continue;
            }
            if (find_macro(macro_name) != NULL)
            {
                fprintf(stderr, "ERROR %s:%d – macro '%s' redefined\n",
                        src_path, line_num, macro_name);
                error_count++;
                continue;
            }
            current_macro_idx = add_macro(macro_name);
            if (current_macro_idx == MACRO_TABLE_FULL)
            {
                fprintf(stderr, "FATAL %s:%d – macro table full\n",
                        src_path, line_num);
                fclose(src_fp);
                fclose(out_fp);
                return 1;
            }
            inside_macro_def = 1;
            continue;
        }

        // inside a macro definition: store the body line 
        if (inside_macro_def)
        {
            Macro *m = &s_macros[current_macro_idx];
            if (m->line_count < MAX_MACRO_LINES)
            {
                strncpy(m->lines[m->line_count], line, MAX_LINE_LEN - 1);
                m->lines[m->line_count][MAX_LINE_LEN - 1] = '\0';
                m->line_count++;
            }
            else
            {
                fprintf(stderr, "ERROR %s:%d – macro body too long\n",
                        src_path, line_num);
                error_count++;
            }
            continue;
        }

        // macro call
        {
            char *macro_check = first_token;
            char *label_to_print = NULL;
            int tok_len = strlen(first_token);

            if (tok_len > 0 && first_token[tok_len - 1] == ':')
            {
                char *next_tok = strtok(rest, " \t");
                if (next_tok != NULL)
                {
                    label_to_print = first_token;
                    macro_check = next_tok;
                }
            }

            macro_ptr = find_macro(macro_check);
            if (macro_ptr != NULL)
            {
                int k;
                if (label_to_print != NULL)
                {
                    fprintf(out_fp, "%s\n", label_to_print);
                }
                for (k = 0; k < macro_ptr->line_count; k++)
                    fprintf(out_fp, "%s\n", macro_ptr->lines[k]);
                continue;
            }
        }

        fprintf(out_fp, "%s\n", line);
    }

    if (inside_macro_def)
    {
        fprintf(stderr, "ERROR %s – mcro not closed at end of file\n",
                src_path);
        error_count++;
    }

    fclose(src_fp);
    fclose(out_fp);
    return 0;
}


static Macro *find_macro(const char *name)
{
    int i;
    for (i = 0; i < s_macro_count; i++)
    {
        if (strcmp(s_macros[i].name, name) == 0)
            return &s_macros[i];
    }
    return NULL;
}

static int add_macro(const char *name)
{
    if (s_macro_count >= MAX_MACROS)
        return MACRO_TABLE_FULL;
    strncpy(s_macros[s_macro_count].name, name, MAX_MACRO_NAME_LEN - 1);
    s_macros[s_macro_count].name[MAX_MACRO_NAME_LEN - 1] = '\0';
    s_macros[s_macro_count].line_count = 0;
    return s_macro_count++;
}
