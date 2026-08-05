#ifndef DEFS_H
#define DEFS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINE_LEN        82
#define MAX_LABEL_LEN       32
#define MAX_SYMBOL_TABLE    512
#define MAX_CODE_IMAGE      1024
#define MAX_DATA_IMAGE      1024
#define MAX_MACRO_LINES     256
#define MAX_MACROS          64
#define MAX_MACRO_NAME_LEN  32
#define IC_INIT             100     // instructions start at 100

#define WORD_SIZE           32
#define OPCODE_BITS         6
#define RS_BITS             5
#define RT_BITS             5
#define RD_BITS             5
#define SHAMT_BITS          5
#define FUNCT_BITS          6
#define IMMED_BITS          16
#define ADDR_BITS           26
#define NUM_REGISTERS       32

#define OPCODE_R_TYPE       0
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
#define FUNCT_NONE         -1

#define SYM_ADD_OK          0
#define SYM_ADD_OVERFLOW   -1
#define SYM_ADD_DUPLICATE  -2

#define BITMASK_6BIT        0x3F
#define BITMASK_5BIT        0x1F
#define BITMASK_16BIT       0xFFFFu
#define BITMASK_26BIT       0x03FFFFFFu
#define BYTE_MASK           0xFF
#define BYTE_SHIFT_BITS     8

#define SHIFT_OPCODE        26
#define SHIFT_RS            21
#define SHIFT_RT            16
#define SHIFT_RD            11
#define SHIFT_SHAMT         6

#define BRANCH_OPCODE_MASK  0xFFFF0000u
#define BRANCH_IMMED_MASK   0xFFFFu
#define JUMP_OPCODE_MASK    0xFC000000u
#define JUMP_ADDR_MASK      0x03FFFFFFu

#define BYTES_PER_DB        1
#define BYTES_PER_DH        2
#define BYTES_PER_DW        4

#define MAX_OPERANDS            4
#define MIN_OPERANDS_SHIFT      3
#define MIN_OPERANDS_JR         1
#define MIN_OPERANDS_R_TYPE     3
#define MIN_OPERANDS_BRANCH     3
#define MIN_OPERANDS_LOAD_STORE 2
#define MIN_OPERANDS_I_TYPE     3
#define MIN_OPERANDS_JUMP       1

#define ERR_BUF_SIZE        128
#define PATH_BUF_SIZE       256
#define SRC_EXT_LEN         3

#define TOKEN_DELIMITERS    " \t"
#define OPERAND_DELIMITERS  ","
#define BRANCH_LABEL_OPERAND 2
#define JUMP_TARGET_OPERAND  0

#define MACRO_TABLE_FULL   -1
#define NO_MACRO_INDEX     -1
#define GET_STEM_FAIL      -1

#define DOT_COMMAND_DB       ".db"
#define DOT_COMMAND_DH       ".dh"
#define DOT_COMMAND_DW       ".dw"
#define DOT_COMMAND_ASCIZ    ".asciz"
#define DOT_COMMAND_ENTRY    ".entry"
#define DOT_COMMAND_EXTERN   ".extern"

typedef enum {
    ATTR_NONE    = 0,
    ATTR_CODE    = 1,
    ATTR_DATA    = 2,
    ATTR_ENTRY   = 4,
    ATTR_EXTERN  = 8
} Symbol_attr;

typedef enum {
    INST_R,
    INST_I,
    INST_J,
    INST_UNKNOWN
} Inst_type;

typedef struct {
    char         name[MAX_LABEL_LEN];
    unsigned int address;
    Symbol_attr  attr;
} Symbol;

typedef struct {
    const char  *str_commend;
    Inst_type    type;
    int          opcode;
    int          funct;
} Opcode_entry;

extern unsigned int  code_image[MAX_CODE_IMAGE];
extern unsigned char data_image[MAX_DATA_IMAGE];
extern int           IC;
extern int           DC;
extern int           error_count;
extern Symbol        symbol_table[MAX_SYMBOL_TABLE];
extern int           symbol_count;

int    sym_add(const char *name, unsigned int addr, Symbol_attr attr);
Symbol *sym_find(const char *name);
void   sym_update_data_labels(int ic_final);

const Opcode_entry *opcode_find(const char *str_commend);

char  *str_trim(char *s);
int    is_valid_label(const char *s);
int    is_register(const char *token, int *out_reg_num);
void   report_error(const char *filename, int line_num, const char *msg);

int    pre_process(const char *src_path, const char *out_path);
int    first_pass(const char *am_path);
int    second_pass(const char *am_path, const char *ob_path,
                   const char *ent_path, const char *ext_path);

#endif
