#include "defs.h"

static const Opcode_entry OPCODE_TABLE[] = {
    { "add", INST_R, OPCODE_R_TYPE, FUNCT_ADD },
    { "addu", INST_R, OPCODE_R_TYPE, FUNCT_ADDU },
    { "sub", INST_R, OPCODE_R_TYPE, FUNCT_SUB },
    { "subu", INST_R, OPCODE_R_TYPE, FUNCT_SUBU },
    { "and", INST_R, OPCODE_R_TYPE, FUNCT_AND },
    { "or", INST_R, OPCODE_R_TYPE, FUNCT_OR },
    { "nor", INST_R, OPCODE_R_TYPE, FUNCT_NOR },
    { "slt", INST_R, OPCODE_R_TYPE, FUNCT_SLT },
    { "sltu", INST_R, OPCODE_R_TYPE, FUNCT_SLTU },
    { "sll", INST_R, OPCODE_R_TYPE, FUNCT_SLL },
    { "srl", INST_R, OPCODE_R_TYPE, FUNCT_SRL },
    { "jr", INST_R, OPCODE_R_TYPE, FUNCT_JR },
    { "addi", INST_I, OPCODE_ADDI, FUNCT_NONE },
    { "addiu", INST_I, OPCODE_ADDIU, FUNCT_NONE },
    { "andi", INST_I, OPCODE_ANDI, FUNCT_NONE },
    { "ori", INST_I, OPCODE_ORI, FUNCT_NONE },
    { "xori", INST_I, OPCODE_XORI, FUNCT_NONE },
    { "slti", INST_I, OPCODE_SLTI, FUNCT_NONE },
    { "sltiu", INST_I, OPCODE_SLTIU, FUNCT_NONE },
    { "beq", INST_I, OPCODE_BEQ, FUNCT_NONE },
    { "bne", INST_I, OPCODE_BNE, FUNCT_NONE },
    { "lw", INST_I, OPCODE_LW, FUNCT_NONE },
    { "sw", INST_I, OPCODE_SW, FUNCT_NONE },
    { "j", INST_J, OPCODE_J, FUNCT_NONE },
    { "jal", INST_J, OPCODE_JAL, FUNCT_NONE },
    { NULL, INST_UNKNOWN, 0, 0 }
};

const Opcode_entry *opcode_find(const char *str_commend)
{
    const Opcode_entry *e;
    for (e = OPCODE_TABLE; e->str_commend != NULL; e++)
    {
        if (strcmp(e->str_commend, str_commend) == 0)
            return e;
    }
    return NULL;
}
