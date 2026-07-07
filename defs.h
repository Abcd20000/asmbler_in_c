/*
 * defs.h  –  Global constants, enums, and shared data types
 *            for the two-pass MIPS-like assembler (Maman 14).
 */
#ifndef DEFS_H
#define DEFS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* ── Sizing limits ─────────────────────────────────────────── */
#define MAX_LINE_LEN        82      /* 80 chars + newline + NUL  */
#define MAX_LABEL_LEN       32
#define MAX_SYMBOL_TABLE    512
#define MAX_CODE_IMAGE      1024    /* instruction words         */
#define MAX_DATA_IMAGE      1024    /* data bytes                */
#define MAX_MACRO_LINES     256
#define MAX_MACROS          64
#define MAX_MACRO_NAME_LEN  32
#define IC_INIT             100     /* instructions start at 100 */

/* ── Architecture constants ────────────────────────────────── */
#define WORD_SIZE           32      /* bits per instruction      */
#define OPCODE_BITS         6
#define RS_BITS             5
#define RT_BITS             5
#define RD_BITS             5
#define SHAMT_BITS          5
#define FUNCT_BITS          6
#define IMMED_BITS          16
#define ADDR_BITS           26
#define NUM_REGISTERS       32

/* ── Instruction type opcodes ──────────────────────────────── */
#define OPCODE_R_TYPE       0   /* all R-type share opcode 0    */
#define OPCODE_J           2
#define OPCODE_JAL         3
#define OPCODE_BEQ         4
#define OPCODE_BNE         5
#define OPCODE_ADDI        8
#define OPCODE_ADDIU       9
#define OPCODE_SLTI        10
#define OPCODE_SLTIU       11
#define OPCODE_ANDI        12
#define OPCODE_ORI         13
#define OPCODE_XORI        14
#define OPCODE_LW          35
#define OPCODE_SW          43

/* ── R-type funct codes ────────────────────────────────────── */
#define FUNCT_SLL          0
#define FUNCT_SRL          2
#define FUNCT_JR           8
#define FUNCT_ADD          32
#define FUNCT_ADDU         33
#define FUNCT_SUB          34
#define FUNCT_SUBU         35
#define FUNCT_AND          36
#define FUNCT_OR           37
#define FUNCT_NOR          39
#define FUNCT_SLT          42
#define FUNCT_SLTU         43

/* ── Assembly directives ───────────────────────────────────── */
#define DIRECTIVE_DB       ".db"
#define DIRECTIVE_DH       ".dh"
#define DIRECTIVE_DW       ".dw"
#define DIRECTIVE_ASCIZ    ".asciz"
#define DIRECTIVE_ENTRY    ".entry"
#define DIRECTIVE_EXTERN   ".extern"

/* ── Symbol attribute flags ────────────────────────────────── */
typedef enum {
    ATTR_NONE    = 0,
    ATTR_CODE    = 1,   /* label in code section    */
    ATTR_DATA    = 2,   /* label in data section    */
    ATTR_ENTRY   = 4,   /* declared .entry          */
    ATTR_EXTERN  = 8    /* declared .extern         */
} Symbol_attr;

/* ── Instruction type ──────────────────────────────────────── */
typedef enum {
    INST_R,
    INST_I,
    INST_J,
    INST_UNKNOWN
} Inst_type;

/* ── One row in the symbol table ───────────────────────────── */
typedef struct {
    char         name[MAX_LABEL_LEN];
    unsigned int address;
    Symbol_attr  attr;
} Symbol;

/* ── Opcode table entry ────────────────────────────────────── */
typedef struct {
    const char  *mnemonic;
    Inst_type    type;
    int          opcode;
    int          funct;     /* -1 for I/J types */
} Opcode_entry;

/* ── Global images and counters (defined in assembler.c) ───── */
extern unsigned int  g_code_image[MAX_CODE_IMAGE];
extern unsigned char g_data_image[MAX_DATA_IMAGE];
extern int           g_ic;          /* instruction counter       */
extern int           g_dc;          /* data counter              */
extern int           g_error_count; /* errors found during pass  */
extern Symbol        g_sym_table[MAX_SYMBOL_TABLE];
extern int           g_sym_count;

/* ── Shared function prototypes ────────────────────────────── */
/* symbol_table.c */
int    sym_add(const char *name, unsigned int addr, Symbol_attr attr);
Symbol *sym_find(const char *name);
void   sym_update_data_labels(int ic_final);

/* opcodes.c */
const Opcode_entry *opcode_find(const char *mnemonic);

/* utils.c */
char  *str_trim(char *s);
int    is_valid_label(const char *s);
int    is_register(const char *token, int *out_reg_num);
void   report_error(const char *filename, int line_num, const char *msg);

/* pre_processor.c */
int    pre_process(const char *src_path, const char *out_path);

/* first_pass.c */
int    first_pass(const char *am_path);

/* second_pass.c */
int    second_pass(const char *am_path,
                   const char *ob_path,
                   const char *ent_path,
                   const char *ext_path);

#endif /* DEFS_H */
