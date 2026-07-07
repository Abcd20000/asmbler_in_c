/*
 * second_pass.c  –  Phase 2: Resolve labels, generate output files.
 *
 *  Re-reads the .am file.  For every instruction that requires a label
 *  operand (branches, jumps), looks it up in the symbol table and patches
 *  the word that was stored in g_code_image[] during the first pass.
 *  Also processes .entry declarations.
 *
 *  Output files produced:
 *    .ob   – object file: IC+DC header, then code image, then data image
 *    .ent  – one  "label  address" line per .entry symbol  (only if any)
 *    .ext  – one  "label  address" line per .extern reference (only if any)
 */
#include "defs.h"

/* ── Forward declarations ────────────────────────────────────── */
static void patch_instruction(const char *am_path, int line_num,
                               const char *mnemonic, char *operands,
                               int word_index);
static void write_ob(const char *ob_path);
static void write_entry(const char *ent_path);
static void write_extern(const char *ext_path,
                         const char *label, unsigned int addr);

/* ── Track whether any extern / entry lines were written ─────── */
static int s_has_entries = 0;
static int s_has_externs = 0;

/* ── File handles kept open during the pass ──────────────────── */
static FILE *s_ext_fp = NULL;

/* ────────────────────────────────────────────────────────────
 * second_pass  –  entry point
 * ──────────────────────────────────────────────────────────── */
int second_pass(const char *am_path,
                const char *ob_path,
                const char *ent_path,
                const char *ext_path)
{
    FILE *fp;
    char  raw_line[MAX_LINE_LEN];
    char  line[MAX_LINE_LEN];
    int   line_num  = 0;
    int   word_index = 0; /* index into g_code_image[] */

    fp = fopen(am_path, "r");
    if (fp == NULL)
    {
        perror(am_path);
        return 1;
    }

    /* open .ext eagerly; we'll delete it later if empty */
    s_ext_fp = fopen(ext_path, "w");
    if (s_ext_fp == NULL)
    {
        perror(ext_path);
        fclose(fp);
        return 1;
    }

    while (fgets(raw_line, MAX_LINE_LEN, fp) != NULL)
    {
        char  buf[MAX_LINE_LEN];
        char *p;
        char *token;

        line_num++;
        strncpy(line, raw_line, MAX_LINE_LEN - 1);
        line[MAX_LINE_LEN - 1] = '\0';

        /* strip newline */
        {
            int len = (int)strlen(line);
            if (len > 0 && line[len - 1] == '\n')
                line[len - 1] = '\0';
        }

        p = str_trim(line);
        if (*p == '\0' || *p == ';')
            continue;

        strncpy(buf, p, MAX_LINE_LEN - 1);
        buf[MAX_LINE_LEN - 1] = '\0';

        token = strtok(buf, " \t");
        if (token == NULL)
            continue;

        /* ── skip labels ─────────────────────────────────── */
        {
            int tok_len = (int)strlen(token);
            if (tok_len > 0 && token[tok_len - 1] == ':')
            {
                token = strtok(NULL, " \t");
                if (token == NULL)
                    continue;
            }
        }

        /* ── directives ──────────────────────────────────── */
        if (token[0] == '.')
        {
            char *operands = strtok(NULL, "");
            if (operands) operands = str_trim(operands);

            /* .entry: mark symbol and note for .ent file */
            if (strcmp(token, DIRECTIVE_ENTRY) == 0)
            {
                char *sym_name = operands ? str_trim(operands) : NULL;
                if (sym_name != NULL)
                {
                    Symbol *sym = sym_find(sym_name);
                    if (sym == NULL)
                    {
                        char err[128];
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
            /* .extern / data directives: no patching needed */
            continue;
        }

        /* ── instruction: patch labels if needed ─────────── */
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

    if (g_error_count > 0)
        return 1;

    /* ── write output files ───────────────────────────────── */
    write_ob(ob_path);
    if (s_has_entries)
        write_entry(ent_path);

    /* if no externals were referenced, remove the empty .ext file */
    if (!s_has_externs)
        remove(ext_path);

    return 0;
}

/* ════════════════════════════════════════════════════════════
 * patch_instruction
 *   Looks at instructions that reference labels and fixes the
 *   already-encoded word in g_code_image[word_index].
 * ════════════════════════════════════════════════════════════ */
static void patch_instruction(const char *am_path, int line_num,
                               const char *mnemonic, char *operands,
                               int word_index)
{
    const Opcode_entry *e;
    char  ops_buf[MAX_LINE_LEN];
    char *tok[4];
    int   tok_count = 0;
    char *p;
    char  err_buf[128];

    e = opcode_find(mnemonic);
    if (e == NULL)
        return; /* already reported in first pass */

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

    /* ── Branch instructions: patch 16-bit immed field ───── */
    if (e->type == INST_I &&
        (e->opcode == OPCODE_BEQ || e->opcode == OPCODE_BNE))
    {
        Symbol       *sym;
        int           offset;
        unsigned int  word;

        if (tok_count < 3)
            return;

        sym = sym_find(tok[2]);
        if (sym == NULL)
        {
            snprintf(err_buf, sizeof(err_buf),
                     "undefined label '%s'", tok[2]);
            report_error(am_path, line_num, err_buf);
            return;
        }
        if (sym->attr & ATTR_EXTERN)
        {
            /* branch to extern: offset = 0 per convention,
               record in .ext as word address */
            unsigned int inst_addr = (unsigned int)(IC_INIT + word_index);
            write_extern(NULL, sym->name, inst_addr);
            return;
        }
        /* offset = (target_addr - current_addr) in words */
        offset = (int)sym->address - (IC_INIT + word_index);
        word   = g_code_image[word_index];
        word   = (word & 0xFFFF0000u) | ((unsigned int)(offset) & 0xFFFFu);
        g_code_image[word_index] = word;
        return;
    }

    /* ── Jump instructions: patch 26-bit address field ────── */
    if (e->type == INST_J)
    {
        Symbol       *sym;
        unsigned int  target;
        unsigned int  word;

        if (tok_count < 1)
        {
            report_error(am_path, line_num, "j/jal missing target label");
            return;
        }

        sym = sym_find(tok[0]);
        if (sym == NULL)
        {
            snprintf(err_buf, sizeof(err_buf),
                     "undefined label '%s'", tok[0]);
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

        word = g_code_image[word_index];
        word = (word & 0xFC000000u) | (target & 0x03FFFFFFu);
        g_code_image[word_index] = word;
        return;
    }

    /* All other instructions need no patching */
}

/* ════════════════════════════════════════════════════════════
 * write_extern  –  Append one line to the .ext file.
 *   (am_path is unused but kept for a consistent signature;
 *    s_ext_fp is the already-open file.)
 * ════════════════════════════════════════════════════════════ */
static void write_extern(const char *am_path,
                         const char *label, unsigned int addr)
{
    (void)am_path;
    if (s_ext_fp != NULL)
    {
        fprintf(s_ext_fp, "%s\t%04u\n", label, addr);
        s_has_externs = 1;
    }
}

/* ════════════════════════════════════════════════════════════
 * write_ob  –  Write the object file.
 *
 *  Format (text):
 *   Line 1:  IC-IC_INIT  DC
 *   Then each word of the code image as a 32-bit hex value,
 *   then each byte of the data image as a 2-digit hex value.
 * ════════════════════════════════════════════════════════════ */
static void write_ob(const char *ob_path)
{
    FILE        *fp;
    int          code_words = g_ic - IC_INIT;
    int          i;

    fp = fopen(ob_path, "w");
    if (fp == NULL)
    {
        perror(ob_path);
        g_error_count++;
        return;
    }

    /* header */
    fprintf(fp, "%d %d\n", code_words, g_dc);

    /* code image: one word per line, address + value */
    for (i = 0; i < code_words; i++)
        fprintf(fp, "%04u %08X\n",
                (unsigned int)(IC_INIT + i), g_code_image[i]);

    /* data image: one byte per line */
    for (i = 0; i < g_dc; i++)
        fprintf(fp, "%04u %02X\n",
                (unsigned int)(g_ic + i), g_data_image[i]);

    fclose(fp);
}

/* ════════════════════════════════════════════════════════════
 * write_entry  –  Write the .ent file.
 * ════════════════════════════════════════════════════════════ */
static void write_entry(const char *ent_path)
{
    FILE *fp;
    int   i;

    fp = fopen(ent_path, "w");
    if (fp == NULL)
    {
        perror(ent_path);
        g_error_count++;
        return;
    }

    for (i = 0; i < g_sym_count; i++)
    {
        if (g_sym_table[i].attr & ATTR_ENTRY)
            fprintf(fp, "%s\t%04u\n",
                    g_sym_table[i].name,
                    g_sym_table[i].address);
    }
    fclose(fp);
}
