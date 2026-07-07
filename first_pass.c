/*
 * first_pass.c  –  Phase 1: Build the symbol table and images.
 *
 *  Scans the .am file line by line and:
 *    1. Strips labels  → enters them in the symbol table.
 *    2. Handles directives (.db / .dh / .dw / .asciz / .entry / .extern).
 *    3. Handles instruction lines → encodes each word into g_code_image[].
 *    4. Advances IC / DC accordingly.
 *
 *  After the loop, data-section label addresses are fixed up by adding
 *  the final IC so that both sections live in one flat address space.
 *
 *  NOTE: Branch/jump targets referencing forward labels are left as 0 here
 *        and are resolved in the second pass.
 */
#include "defs.h"

/* ── Forward declarations ────────────────────────────────────── */
static void handle_directive(const char *am_path, int line_num,
                             const char *directive, char *operands);
static void handle_instruction(const char *am_path, int line_num,
                                const char *mnemonic, char *operands);
static unsigned int encode_r(const Opcode_entry *e,
                              int rs, int rt, int rd, int shamt);
static unsigned int encode_i(const Opcode_entry *e,
                              int rs, int rt, int immed);
static unsigned int encode_j(const Opcode_entry *e, unsigned int address);

/* ────────────────────────────────────────────────────────────
 * first_pass  –  entry point
 * ──────────────────────────────────────────────────────────── */
int first_pass(const char *am_path)
{
    FILE *fp;
    char  raw_line[MAX_LINE_LEN];
    char  line[MAX_LINE_LEN];
    int   line_num = 0;

    fp = fopen(am_path, "r");
    if (fp == NULL)
    {
        perror(am_path);
        return 1;
    }

    while (fgets(raw_line, MAX_LINE_LEN, fp) != NULL)
    {
        char  buf[MAX_LINE_LEN];
        char *p;
        char *label    = NULL;
        char *token;
        int   has_label = 0;

        line_num++;
        strncpy(line, raw_line, MAX_LINE_LEN - 1);
        line[MAX_LINE_LEN - 1] = '\0';

        /* strip newline */
        {
            int len = (int)strlen(line);
            if (len > 0 && line[len - 1] == '\n')
                line[len - 1] = '\0';
        }

        /* trim + skip blanks and comments */
        p = str_trim(line);
        if (*p == '\0' || *p == ';')
            continue;

        strncpy(buf, p, MAX_LINE_LEN - 1);
        buf[MAX_LINE_LEN - 1] = '\0';

        /* ── check for label  (token ending with ':') ─── */
        token = strtok(buf, " \t");
        if (token != NULL)
        {
            int tok_len = (int)strlen(token);
            if (tok_len > 0 && token[tok_len - 1] == ':')
            {
                token[tok_len - 1] = '\0'; /* remove ':' */
                if (!is_valid_label(token))
                {
                    report_error(am_path, line_num, "invalid label name");
                }
                else
                {
                    label     = token;
                    has_label = 1;
                }
                token = strtok(NULL, " \t"); /* next token after label */
            }
        }

        if (token == NULL)
        {
            /* label-only line is valid but unusual */
            if (has_label)
                report_error(am_path, line_num,
                             "label on empty line – ignored");
            continue;
        }

        /* ── directive? ─────────────────────────────────── */
        if (token[0] == '.')
        {
            char *operands = strtok(NULL, "");
            if (operands)
                operands = str_trim(operands);

            /* .entry / .extern labels do NOT get a code/data address */
            if (strcmp(token, DIRECTIVE_ENTRY) == 0 ||
                strcmp(token, DIRECTIVE_EXTERN) == 0)
            {
                /* label before .entry / .extern is ignored per spec */
                handle_directive(am_path, line_num, token, operands);
                continue;
            }

            /* data directive: label (if any) goes into DATA section */
            if (has_label)
            {
                Symbol_attr attr = ATTR_DATA;
                if (sym_add(label, (unsigned int)g_dc, attr) == -2)
                    report_error(am_path, line_num, "label redefined");
            }
            handle_directive(am_path, line_num, token, operands);
        }
        else
        {
            /* ── instruction ─────────────────────────────── */
            char *operands = strtok(NULL, "");
            if (operands)
                operands = str_trim(operands);

            if (has_label)
            {
                if (sym_add(label, (unsigned int)g_ic, ATTR_CODE) == -2)
                    report_error(am_path, line_num, "label redefined");
            }
            handle_instruction(am_path, line_num, token, operands);
        }
    }

    fclose(fp);

    /* ── fix up data-label addresses ─────────────────────── */
    sym_update_data_labels(g_ic);

    return (g_error_count > 0) ? 1 : 0;
}

/* ════════════════════════════════════════════════════════════
 * handle_directive
 * ════════════════════════════════════════════════════════════ */
static void handle_directive(const char *am_path, int line_num,
                             const char *directive, char *operands)
{
    char err_buf[128];

    /* ── .extern ─────────────────────────────────────────── */
    if (strcmp(directive, DIRECTIVE_EXTERN) == 0)
    {
        char *sym_name = operands ? str_trim(operands) : NULL;
        if (sym_name == NULL || !is_valid_label(sym_name))
        {
            report_error(am_path, line_num, ".extern requires valid label");
            return;
        }
        sym_add(sym_name, 0, ATTR_EXTERN);
        return;
    }

    /* ── .entry  (note: address resolved in second pass) ─── */
    if (strcmp(directive, DIRECTIVE_ENTRY) == 0)
    {
        /* just mark for second pass – nothing to do in first pass */
        return;
    }

    /* ── .asciz ──────────────────────────────────────────── */
    if (strcmp(directive, DIRECTIVE_ASCIZ) == 0)
    {
        const char *s;
        if (operands == NULL || operands[0] != '"')
        {
            report_error(am_path, line_num,
                         ".asciz operand must be a quoted string");
            return;
        }
        /* skip opening quote */
        s = operands + 1;
        while (*s != '\0' && *s != '"')
        {
            if (g_dc >= MAX_DATA_IMAGE)
            {
                report_error(am_path, line_num, "data image overflow");
                return;
            }
            g_data_image[g_dc++] = (unsigned char)*s++;
        }
        if (*s != '"')
        {
            report_error(am_path, line_num, ".asciz string not closed");
            return;
        }
        /* NUL terminator */
        if (g_dc >= MAX_DATA_IMAGE)
        {
            report_error(am_path, line_num, "data image overflow");
            return;
        }
        g_data_image[g_dc++] = 0;
        return;
    }

    /* ── .db / .dh / .dw – comma-separated integer list ──── */
    if (strcmp(directive, DIRECTIVE_DB) == 0 ||
        strcmp(directive, DIRECTIVE_DH) == 0 ||
        strcmp(directive, DIRECTIVE_DW) == 0)
    {
        int   bytes;
        char *token;
        char  ops_buf[MAX_LINE_LEN];

        if (strcmp(directive, DIRECTIVE_DB) == 0) bytes = 1;
        else if (strcmp(directive, DIRECTIVE_DH) == 0) bytes = 2;
        else bytes = 4;

        if (operands == NULL)
        {
            snprintf(err_buf, sizeof(err_buf),
                     "%s requires at least one value", directive);
            report_error(am_path, line_num, err_buf);
            return;
        }

        strncpy(ops_buf, operands, MAX_LINE_LEN - 1);
        ops_buf[MAX_LINE_LEN - 1] = '\0';

        token = strtok(ops_buf, ",");
        while (token != NULL)
        {
            char          *end_ptr;
            long           val;
            int            b;
            unsigned char  byte_val;

            token = str_trim(token);
            val   = strtol(token, &end_ptr, 0);

            if (*end_ptr != '\0')
            {
                snprintf(err_buf, sizeof(err_buf),
                         "invalid value '%s' in %s", token, directive);
                report_error(am_path, line_num, err_buf);
                token = strtok(NULL, ",");
                continue;
            }

            /* store little-endian */
            for (b = 0; b < bytes; b++)
            {
                if (g_dc >= MAX_DATA_IMAGE)
                {
                    report_error(am_path, line_num, "data image overflow");
                    return;
                }
                byte_val          = (unsigned char)(val & 0xFF);
                g_data_image[g_dc++] = byte_val;
                val >>= 8;
            }
            token = strtok(NULL, ",");
        }
        return;
    }

    snprintf(err_buf, sizeof(err_buf),
             "unknown directive '%s'", directive);
    report_error(am_path, line_num, err_buf);
}

/* ════════════════════════════════════════════════════════════
 * handle_instruction
 * ════════════════════════════════════════════════════════════ */
static void handle_instruction(const char *am_path, int line_num,
                                const char *mnemonic, char *operands)
{
    const Opcode_entry *e;
    char  ops_buf[MAX_LINE_LEN];
    char *tok[4];
    int   tok_count = 0;
    char *p;
    char  err_buf[128];

    e = opcode_find(mnemonic);
    if (e == NULL)
    {
        snprintf(err_buf, sizeof(err_buf),
                 "unknown instruction '%s'", mnemonic);
        report_error(am_path, line_num, err_buf);
        return;
    }

    if (g_ic - IC_INIT >= MAX_CODE_IMAGE)
    {
        report_error(am_path, line_num, "code image overflow");
        return;
    }

    /* tokenise operands on ',' */
    ops_buf[0] = '\0';
    if (operands)
        strncpy(ops_buf, operands, MAX_LINE_LEN - 1);
    ops_buf[MAX_LINE_LEN - 1] = '\0';

    p = strtok(ops_buf, ",");
    while (p != NULL && tok_count < 4)
    {
        tok[tok_count++] = str_trim(p);
        p = strtok(NULL, ",");
    }

    /* ── R-type ──────────────────────────────────────────── */
    if (e->type == INST_R)
    {
        int rs = 0, rt = 0, rd = 0, shamt = 0;

        /* sll / srl: rd, rt, shamt */
        if (e->funct == FUNCT_SLL || e->funct == FUNCT_SRL)
        {
            if (tok_count < 3 ||
                !is_register(tok[0], &rd) ||
                !is_register(tok[1], &rt))
            {
                report_error(am_path, line_num,
                             "shift expects: rd, rt, shamt");
                return;
            }
            shamt = (int)strtol(tok[2], NULL, 0);
        }
        /* jr: rs */
        else if (e->funct == FUNCT_JR)
        {
            if (tok_count < 1 || !is_register(tok[0], &rs))
            {
                report_error(am_path, line_num, "jr expects: rs");
                return;
            }
        }
        /* normal R: rd, rs, rt */
        else
        {
            if (tok_count < 3 ||
                !is_register(tok[0], &rd) ||
                !is_register(tok[1], &rs) ||
                !is_register(tok[2], &rt))
            {
                report_error(am_path, line_num,
                             "R-type expects: rd, rs, rt");
                return;
            }
        }
        g_code_image[g_ic - IC_INIT] = encode_r(e, rs, rt, rd, shamt);
        g_ic++;
        return;
    }

    /* ── I-type ──────────────────────────────────────────── */
    if (e->type == INST_I)
    {
        int  rs = 0, rt = 0, immed = 0;
        char *end_ptr;

        /* beq / bne: rs, rt, label  (immed patched in second pass) */
        if (e->opcode == OPCODE_BEQ || e->opcode == OPCODE_BNE)
        {
            if (tok_count < 3 ||
                !is_register(tok[0], &rs) ||
                !is_register(tok[1], &rt))
            {
                report_error(am_path, line_num,
                             "branch expects: rs, rt, label");
                return;
            }
            /* immed = 0 for now; second pass will fill it */
        }
        /* lw / sw: rt, immed(rs) */
        else if (e->opcode == OPCODE_LW || e->opcode == OPCODE_SW)
        {
            char inner[MAX_LINE_LEN];
            char *paren;

            if (tok_count < 2)
            {
                report_error(am_path, line_num,
                             "load/store expects: rt, immed(rs)");
                return;
            }
            if (!is_register(tok[0], &rt))
            {
                report_error(am_path, line_num, "expected register for rt");
                return;
            }
            /* parse  immed(rs) */
            strncpy(inner, tok[1], MAX_LINE_LEN - 1);
            inner[MAX_LINE_LEN - 1] = '\0';
            paren = strchr(inner, '(');
            if (paren == NULL)
            {
                report_error(am_path, line_num,
                             "load/store: expected immed(rs)");
                return;
            }
            *paren = '\0';
            immed  = (int)strtol(inner, NULL, 0);
            paren++;
            {
                char *close = strchr(paren, ')');
                if (close) *close = '\0';
            }
            if (!is_register(paren, &rs))
            {
                report_error(am_path, line_num,
                             "load/store: invalid base register");
                return;
            }
        }
        /* normal I: rt, rs, immed */
        else
        {
            if (tok_count < 3 ||
                !is_register(tok[0], &rt) ||
                !is_register(tok[1], &rs))
            {
                report_error(am_path, line_num,
                             "I-type expects: rt, rs, immed");
                return;
            }
            immed = (int)strtol(tok[2], &end_ptr, 0);
            if (*end_ptr != '\0')
            {
                report_error(am_path, line_num, "invalid immediate value");
                return;
            }
        }
        g_code_image[g_ic - IC_INIT] = encode_i(e, rs, rt, immed);
        g_ic++;
        return;
    }

    /* ── J-type ──────────────────────────────────────────── */
    if (e->type == INST_J)
    {
        /* address is a label – store 0 now, second pass resolves it */
        g_code_image[g_ic - IC_INIT] = encode_j(e, 0);
        g_ic++;
        return;
    }

    report_error(am_path, line_num, "internal: unknown instruction type");
}

/* ════════════════════════════════════════════════════════════
 * Encoding helpers
 * ════════════════════════════════════════════════════════════ */

/*
 *  R format:
 *   [31..26] opcode(6) | [25..21] rs(5) | [20..16] rt(5)
 *   | [15..11] rd(5) | [10..6] shamt(5) | [5..0] funct(6)
 */
static unsigned int encode_r(const Opcode_entry *e,
                              int rs, int rt, int rd, int shamt)
{
    unsigned int word = 0;
    word |= ((unsigned int)(e->opcode & 0x3F)) << 26;
    word |= ((unsigned int)(rs        & 0x1F)) << 21;
    word |= ((unsigned int)(rt        & 0x1F)) << 16;
    word |= ((unsigned int)(rd        & 0x1F)) << 11;
    word |= ((unsigned int)(shamt     & 0x1F)) << 6;
    word |= ((unsigned int)(e->funct  & 0x3F));
    return word;
}

/*
 *  I format:
 *   [31..26] opcode(6) | [25..21] rs(5) | [20..16] rt(5)
 *   | [15..0] immed(16)
 */
static unsigned int encode_i(const Opcode_entry *e,
                              int rs, int rt, int immed)
{
    unsigned int word = 0;
    word |= ((unsigned int)(e->opcode & 0x3F))    << 26;
    word |= ((unsigned int)(rs        & 0x1F))    << 21;
    word |= ((unsigned int)(rt        & 0x1F))    << 16;
    word |= ((unsigned int)(immed     & 0xFFFF));
    return word;
}

/*
 *  J format:
 *   [31..26] opcode(6) | [25..0] address(26)
 */
static unsigned int encode_j(const Opcode_entry *e, unsigned int address)
{
    unsigned int word = 0;
    word |= ((unsigned int)(e->opcode & 0x3F)) << 26;
    word |= (address & 0x03FFFFFF);
    return word;
}
