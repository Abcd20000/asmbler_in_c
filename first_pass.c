#include "defs.h"

static void handle_dot_command(const char *am_path, int line_num,
                             const char *dot_command, char *operands);
static void handle_instruction(const char *am_path, int line_num,
                                const char *str_commend, char *operands);
static unsigned int encode_r(const Opcode_entry *e,
                              int rs, int rt, int rd, int shamt);
static unsigned int encode_i(const Opcode_entry *e,
                              int rs, int rt, int immed);
static unsigned int encode_j(const Opcode_entry *e, unsigned int address);

// first_pass entry point
int first_pass(const char *am_path)
{
    FILE *fp;
    char  raw_line[MAX_LINE_LEN];
    char  line[MAX_LINE_LEN];
    int   line_num = 0;

    fp = fopen(am_path, "r");
    if (!fp)
    {
        perror(am_path);
        return 1;
    }

    while (fgets(raw_line, MAX_LINE_LEN, fp))
    {
        char  buf[MAX_LINE_LEN];
        char *p;
        char *label    = NULL;
        char *token;
        int   has_label = 0;

        int len;
        
        line_num++;
        strncpy(line, raw_line, MAX_LINE_LEN - 1);
        line[MAX_LINE_LEN - 1] = '\0';

        // strip newline
        len = (int)strlen(line);
        if (len > 0 && line[len - 1] == '\n')
            line[len - 1] = '\0';

        // trim + skip blanks and comments
        p = str_trim(line);
        if (*p == '\0' || *p == ';')
            continue;

        strncpy(buf, p, MAX_LINE_LEN - 1);
        buf[MAX_LINE_LEN - 1] = '\0';

        // check for label
        token = strtok(buf, " \t");
        if (token)
        {
            int tok_len = (int)strlen(token);
            if (tok_len > 0 && token[tok_len - 1] == ':')
            {
                token[tok_len - 1] = '\0';
                if (!is_valid_label(token))
                {
                    report_error(am_path, line_num, "invalid label name");
                }
                else
                {
                    label = token;
                    has_label = 1;
                }
                token = strtok(NULL, " \t"); // next token after label
            }
        }

        if (!token)
        {
            if (has_label)
            {
                if (sym_add(label, (unsigned int)IC, ATTR_CODE) == SYM_ADD_DUPLICATE)
                    report_error(am_path, line_num, "label redefined");
            }
            continue;
        }

        // directive?
        if (token[0] == '.')
        {
            char *operands = strtok(NULL, "");
            if (operands)
                operands = str_trim(operands);

            // .entry / .extern labels do NOT get a code/data address
            if (strcmp(token, DOT_COMMAND_ENTRY) == 0 ||
                strcmp(token, DOT_COMMAND_EXTERN) == 0)
            {
                handle_dot_command(am_path, line_num, token, operands);
                continue;
            }

            if (has_label)
            {
                Symbol_attr attr = ATTR_DATA;
                if (sym_add(label, (unsigned int)DC, attr) == SYM_ADD_DUPLICATE)
                    report_error(am_path, line_num, "label redefined");
            }
            handle_dot_command(am_path, line_num, token, operands);
        }
        else
        {
            char *operands = strtok(NULL, "");
            if (operands)
                operands = str_trim(operands);

            if (has_label)
            {
                if (sym_add(label, (unsigned int)IC, ATTR_CODE) == SYM_ADD_DUPLICATE)
                    report_error(am_path, line_num, "label redefined");
            }
            handle_instruction(am_path, line_num, token, operands);
        }
    }

    fclose(fp);

    sym_update_data_labels(IC);

    return error_count > 0;
}

// handle_dot_command
static void handle_dot_command(const char *am_path, int line_num,
                             const char *dot_command, char *operands)
{
    char err_buf[128];

    // .extern
    if (strcmp(dot_command, DOT_COMMAND_EXTERN) == 0)
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

    if (strcmp(dot_command, DOT_COMMAND_ENTRY) == 0)
    {
        return;
    }

    // .asciz
    if (strcmp(dot_command, DOT_COMMAND_ASCIZ) == 0)
    {
        const char *s;
        if (operands == NULL || operands[0] != '"')
        {
            report_error(am_path, line_num,
                         ".asciz operand must be a quoted string");
            return;
        }
        // skip opening quote
        s = operands + 1;
        while (*s != '\0' && *s != '"')
        {
            if (DC >= MAX_DATA_IMAGE)
            {
                report_error(am_path, line_num, "data image overflow");
                return;
            }
            data_image[DC++] = (unsigned char)*s++;
        }
        if (*s != '"')
        {
            report_error(am_path, line_num, ".asciz string not closed");
            return;
        }
        // NUL terminator
        if (DC >= MAX_DATA_IMAGE)
        {
            report_error(am_path, line_num, "data image overflow");
            return;
        }
        data_image[DC++] = 0;
        return;
    }

    // .db / .dh / .dw - comma-separated integer list
    if (strcmp(dot_command, DOT_COMMAND_DB) == 0 ||
        strcmp(dot_command, DOT_COMMAND_DH) == 0 ||
        strcmp(dot_command, DOT_COMMAND_DW) == 0)
    {
        int   bytes;
        char *token;
        char  ops_buf[MAX_LINE_LEN];

        if (strcmp(dot_command, DOT_COMMAND_DB) == 0) bytes = BYTES_PER_DB;
        else if (strcmp(dot_command, DOT_COMMAND_DH) == 0) bytes = BYTES_PER_DH;
        else bytes = BYTES_PER_DW;

        if (!operands)
        {
            snprintf(err_buf, sizeof(err_buf),
                     "%s requires at least one value", dot_command);
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
                         "invalid value '%s' in %s", token, dot_command);
                report_error(am_path, line_num, err_buf);
                token = strtok(NULL, ",");
                continue;
            }

            for (b = 0; b < bytes; b++)
            {
                if (DC >= MAX_DATA_IMAGE)
                {
                    report_error(am_path, line_num, "data image overflow");
                    return;
                }
                byte_val          = (unsigned char)(val & BYTE_MASK);
                data_image[DC++] = byte_val;
                val >>= BYTE_SHIFT_BITS;
            }
            token = strtok(NULL, ",");
        }
        return;
    }

    snprintf(err_buf, sizeof(err_buf),
             "unknown dot command '%s'", dot_command);
    report_error(am_path, line_num, err_buf);
}

// handle_instruction
static void handle_instruction(const char *am_path, int line_num,
                                const char *str_commend, char *operands)
{
    const Opcode_entry *e;
    char  ops_buf[MAX_LINE_LEN];
    char *tok[MAX_OPERANDS];
    int   tok_count = 0;
    char *p;
    char  err_buf[ERR_BUF_SIZE];

    e = opcode_find(str_commend);
    if (!e)
    {
        snprintf(err_buf, sizeof(err_buf),
                 "unknown instruction '%s'", str_commend);
        report_error(am_path, line_num, err_buf);
        return;
    }

    if (IC - IC_INIT >= MAX_CODE_IMAGE)
    {
        report_error(am_path, line_num, "code image overflow");
        return;
    }

    // tokenise operands on ','
    ops_buf[0] = '\0';
    if (operands)
        strncpy(ops_buf, operands, MAX_LINE_LEN - 1);
    ops_buf[MAX_LINE_LEN - 1] = '\0';

    p = strtok(ops_buf, ",");
    while (p != NULL && tok_count < MAX_OPERANDS)
    {
        tok[tok_count++] = str_trim(p);
        p = strtok(NULL, ",");
    }

    // R-type
    if (e->type == INST_R)
    {
        int rs = 0, rt = 0, rd = 0, shamt = 0;

        // sll / srl: rd, rt, shamt
        if (e->funct == FUNCT_SLL || e->funct == FUNCT_SRL)
        {
            if (tok_count < MIN_OPERANDS_SHIFT ||
                !is_register(tok[0], &rd) ||
                !is_register(tok[1], &rt))
            {
                report_error(am_path, line_num,
                             "shift expects: rd, rt, shamt");
                return;
            }
            shamt = (int)strtol(tok[2], NULL, 0);
        }
        // jr: rs
        else if (e->funct == FUNCT_JR)
        {
            if (tok_count < MIN_OPERANDS_JR || !is_register(tok[0], &rs))
            {
                report_error(am_path, line_num, "jr expects: rs");
                return;
            }
        }
        // normal R: rd, rs, rt
        else
        {
            if (tok_count < MIN_OPERANDS_R_TYPE ||
                !is_register(tok[0], &rd) ||
                !is_register(tok[1], &rs) ||
                !is_register(tok[2], &rt))
            {
                report_error(am_path, line_num,
                             "R-type expects: rd, rs, rt");
                return;
            }
        }
        code_image[IC - IC_INIT] = encode_r(e, rs, rt, rd, shamt);
        IC++;
        return;
    }

    // I-type
    if (e->type == INST_I)
    {
        int  rs = 0, rt = 0, immed = 0;
        char *end_ptr;

        // beq / bne: rs, rt, label  (immed patched in second pass)
        if (e->opcode == OPCODE_BEQ || e->opcode == OPCODE_BNE)
        {
            if (tok_count < MIN_OPERANDS_BRANCH ||
                !is_register(tok[0], &rs) ||
                !is_register(tok[1], &rt))
            {
                report_error(am_path, line_num,
                             "branch expects: rs, rt, label");
                return;
            }
            // immed = 0 for now; second pass will fill it
        }
        // lw / sw: rt, immed(rs)
        else if (e->opcode == OPCODE_LW || e->opcode == OPCODE_SW)
        {
            char inner[MAX_LINE_LEN];
            char *paren;

            if (tok_count < MIN_OPERANDS_LOAD_STORE)
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
            // parse  immed(rs)
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
        // normal I: rt, rs, immed
        else
        {
            if (tok_count < MIN_OPERANDS_I_TYPE ||
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
        code_image[IC - IC_INIT] = encode_i(e, rs, rt, immed);
        IC++;
        return;
    }

    // J-type
    if (e->type == INST_J)
    {
        // address is a label – store 0 now, second pass resolves it
        code_image[IC - IC_INIT] = encode_j(e, 0);
        IC++;
        return;
    }

    report_error(am_path, line_num, "internal: unknown instruction type");
}

// Helpers

/*
 *  R format:
 *   [31..26] opcode(6) | [25..21] rs(5) | [20..16] rt(5)
 *   | [15..11] rd(5) | [10..6] shamt(5) | [5..0] funct(6)
 */
static unsigned int encode_r(const Opcode_entry *e,
                              int rs, int rt, int rd, int shamt)
{
    unsigned int word = 0;
    word |= ((unsigned int)(e->opcode & BITMASK_6BIT)) << SHIFT_OPCODE;
    word |= ((unsigned int)(rs        & BITMASK_5BIT)) << SHIFT_RS;
    word |= ((unsigned int)(rt        & BITMASK_5BIT)) << SHIFT_RT;
    word |= ((unsigned int)(rd        & BITMASK_5BIT)) << SHIFT_RD;
    word |= ((unsigned int)(shamt     & BITMASK_5BIT)) << SHIFT_SHAMT;
    word |= ((unsigned int)(e->funct  & BITMASK_6BIT));
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
    word |= ((unsigned int)(e->opcode & BITMASK_6BIT))  << SHIFT_OPCODE;
    word |= ((unsigned int)(rs        & BITMASK_5BIT))  << SHIFT_RS;
    word |= ((unsigned int)(rt        & BITMASK_5BIT))  << SHIFT_RT;
    word |= ((unsigned int)(immed     & BITMASK_16BIT));
    return word;
}

/*
 *  J format:
 *   [31..26] opcode(6) | [25..0] address(26)
 */
static unsigned int encode_j(const Opcode_entry *e, unsigned int address)
{
    unsigned int word = 0;
    word |= ((unsigned int)(e->opcode & BITMASK_6BIT)) << SHIFT_OPCODE;
    word |= (address & BITMASK_26BIT);
    return word;
}
