#include "defs.h"

#define TOKEN_DELIMITERS       " \t"
#define OPERAND_DELIMITERS     ","
#define BRANCH_LABEL_OPERAND   2
#define JUMP_TARGET_OPERAND    0

/* Forward declarations */
static void patch_instruction(const char *am_path, int line_num,
                               const char *mnemonic, char *operands,
                               int word_index);
static void write_ob(const char *ob_path);
static void write_entry(const char *ent_path);
static void write_extern(const char *ext_path,
                         const char *label, unsigned int addr);

// test if any extern / entry lines were written
static int s_has_entries = 0;
static int s_has_externs = 0;

static FILE *s_ext_fp = NULL;

int second_pass(const char *am_path,
                const char *ob_path,
                const char *ent_path,
                const char *ext_path)
{
    FILE *fp;
    char  raw_line[MAX_LINE_LEN];
    char  line[MAX_LINE_LEN];
    int   line_num  = 0;
    int   word_index = 0;

    fp = fopen(am_path, "r");
    if (!fp)
    {
        perror(am_path);
        return 1;
    }

    s_ext_fp = fopen(ext_path, "w");
    if (!s_ext_fp)
    {
        perror(ext_path);
        fclose(fp);
        return 1;
    }

    while (fgets(raw_line, MAX_LINE_LEN, fp))
    {
        char  buf[MAX_LINE_LEN];
        char *p;
        char *token;

        int len;
        
        line_num++;
        strncpy(line, raw_line, MAX_LINE_LEN - 1);
        line[MAX_LINE_LEN - 1] = '\0';

        // remove newline 
        len = (int)strlen(line);
        if (len > 0 && line[len - 1] == '\n')
            line[len - 1] = '\0';

        p = str_trim(line);
        if (*p == '\0' || *p == ';')
            continue;

        strncpy(buf, p, MAX_LINE_LEN - 1);
        buf[MAX_LINE_LEN - 1] = '\0';

        token = strtok(buf, TOKEN_DELIMITERS);
        if (!token)
            continue;

        // skip labels
        int tok_len = (int)strlen(token);
        if (tok_len > 0 && token[tok_len - 1] == ':')
        {
            token = strtok(NULL, TOKEN_DELIMITERS);
            if (!token)
                continue;
        }

        if (token[0] == '.')
        {
            char *operands = strtok(NULL, "");
            if (operands) operands = str_trim(operands);

            // .entry: mark symbol and note for .ent file
            if (strcmp(token, DOT_COMMAND_ENTRY) == 0)
            {
                char *sym_name = operands ? str_trim(operands) : NULL;
                if (sym_name)
                {
                    Symbol *sym = sym_find(sym_name);
                    if (!sym)
                    {
                        char err[ERR_BUF_SIZE];
                        snprintf(err, sizeof(err),
                                 ".entry label '%s' not defined", sym_name);
                        report_error(am_path, line_num, err);
                    }
                    else
                    {
                        sym->attr = (Symbol_attr)(sym->attr | ATTR_ENTRY);
                        s_has_entries = 1;
                    }
                }
            }
            continue;
        }


        {
            char *operands = strtok(NULL, "");
            if (operands) operands = str_trim(operands);
            patch_instruction(am_path, line_num, token, operands, word_index);
            word_index++;
        }
    }

    fclose(fp);
    fclose(s_ext_fp);
    s_ext_fp = NULL;

    if (error_count > 0)
        return 1;


    write_ob(ob_path);
    if (s_has_entries)
        write_entry(ent_path);

    if (!s_has_externs)
        remove(ext_path);

    return 0;
}

static void patch_instruction(const char *am_path, int line_num,
                               const char *str_commend, char *operands,
                               int word_index)
{
    const Opcode_entry *e;
    char  ops_buf[MAX_LINE_LEN];
    char *tok[MAX_OPERANDS];
    int   tok_count = 0;
    char *p;
    char  err_buf[ERR_BUF_SIZE];

    e = opcode_find(str_commend);
    if (!e)
        return;

    ops_buf[0] = '\0';
    if (operands)
        strncpy(ops_buf, operands, MAX_LINE_LEN - 1);
    ops_buf[MAX_LINE_LEN - 1] = '\0';

    p = strtok(ops_buf, OPERAND_DELIMITERS);
    while (p && tok_count < MAX_OPERANDS)
    {
        tok[tok_count++] = str_trim(p);
        p = strtok(NULL, OPERAND_DELIMITERS);
    }

    if (e->type == INST_I &&
        (e->opcode == OPCODE_BEQ || e->opcode == OPCODE_BNE))
    {
        Symbol       *sym;
        int           offset;
        unsigned int  word;

        if (tok_count < MIN_OPERANDS_BRANCH)
            return;

        sym = sym_find(tok[BRANCH_LABEL_OPERAND]);
        if (!sym)
        {
            snprintf(err_buf, sizeof(err_buf),
                     "undefined label '%s'", tok[BRANCH_LABEL_OPERAND]);
            report_error(am_path, line_num, err_buf);
            return;
        }
        if (sym->attr & ATTR_EXTERN)
        {
            unsigned int inst_addr = (unsigned int)(IC_INIT + word_index);
            write_extern(NULL, sym->name, inst_addr);
            return;
        }
        offset = (int)sym->address - (IC_INIT + word_index);
        word   = code_image[word_index];
        word   = (word & BRANCH_OPCODE_MASK) | ((unsigned int)(offset) & BRANCH_IMMED_MASK);
        code_image[word_index] = word;
        return;
    }

    /* Jump instructions: patch 26-bit address field */
    if (e->type == INST_J)
    {
        Symbol       *sym;
        unsigned int  target;
        unsigned int  word;

        if (tok_count < MIN_OPERANDS_JUMP)
        {
            report_error(am_path, line_num, "j/jal missing target label");
            return;
        }

        sym = sym_find(tok[JUMP_TARGET_OPERAND]);
        if (!sym)
        {
            snprintf(err_buf, sizeof(err_buf),
                     "undefined label '%s'", tok[JUMP_TARGET_OPERAND]);
            report_error(am_path, line_num, err_buf);
            return;
        }

        target = sym->address;
        if (sym->attr & ATTR_EXTERN)
        {
            unsigned int inst_addr = (unsigned int)(IC_INIT + word_index);
            write_extern(NULL, sym->name, inst_addr);
            target = 0;
        }

        word = code_image[word_index];
        word = (word & JUMP_OPCODE_MASK) | (target & JUMP_ADDR_MASK);
        code_image[word_index] = word;
        return;
    }


}

static void write_extern(const char *am_path,
                         const char *label, unsigned int addr)
{
    (void)am_path;
    if (s_ext_fp)
    {
        fprintf(s_ext_fp, "%s\t%04u\n", label, addr);
        s_has_externs = 1;
    }
}

static void write_ob(const char *ob_path)
{
    FILE        *fp;
    int          code_words = IC - IC_INIT;
    int          i;

    fp = fopen(ob_path, "w");
    if (!fp)
    {
        perror(ob_path);
        error_count++;
        return;
    }

    fprintf(fp, "%d %d\n", code_words, DC);

    for (i = 0; i < code_words; i++)
        fprintf(fp, "%04u %08X\n",
                (unsigned int)(IC_INIT + i), code_image[i]);

    for (i = 0; i < DC; i++)
        fprintf(fp, "%04u %02X\n",
                (unsigned int)(IC + i), data_image[i]);

    fclose(fp);
}

static void write_entry(const char *ent_path)
{
    FILE *fp;
    int   i;

    fp = fopen(ent_path, "w");
    if (!fp)
    {
        perror(ent_path);
        error_count++;
        return;
    }

    for (i = 0; i < symbol_count; i++)
    {
        if (symbol_table[i].attr & ATTR_ENTRY)
            fprintf(fp, "%s\t%04u\n",
                    symbol_table[i].name,
                    symbol_table[i].address);
    }
    fclose(fp);
}
