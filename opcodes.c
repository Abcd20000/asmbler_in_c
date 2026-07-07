/*
 * opcodes.c  –  Opcode / funct lookup table.
 *
 *  The table maps every mnemonic to:
 *    - instruction type  (R / I / J)
 *    - opcode field value
 *    - funct field value  (-1 when not applicable)
 */
#include "defs.h"

static const Opcode_entry OPCODE_TABLE[] = {
    /* ── R-type ───────────────────────────────────────── */
    { "add",   INST_R, OPCODE_R_TYPE, FUNCT_ADD  },
    { "addu",  INST_R, OPCODE_R_TYPE, FUNCT_ADDU },
    { "sub",   INST_R, OPCODE_R_TYPE, FUNCT_SUB  },
    { "subu",  INST_R, OPCODE_R_TYPE, FUNCT_SUBU },
    { "and",   INST_R, OPCODE_R_TYPE, FUNCT_AND  },
    { "or",    INST_R, OPCODE_R_TYPE, FUNCT_OR   },
    { "nor",   INST_R, OPCODE_R_TYPE, FUNCT_NOR  },
    { "slt",   INST_R, OPCODE_R_TYPE, FUNCT_SLT  },
    { "sltu",  INST_R, OPCODE_R_TYPE, FUNCT_SLTU },
    { "sll",   INST_R, OPCODE_R_TYPE, FUNCT_SLL  },
    { "srl",   INST_R, OPCODE_R_TYPE, FUNCT_SRL  },
    { "jr",    INST_R, OPCODE_R_TYPE, FUNCT_JR   },
    /* ── I-type ───────────────────────────────────────── */
    { "addi",  INST_I, OPCODE_ADDI,  -1 },
    { "addiu", INST_I, OPCODE_ADDIU, -1 },
    { "andi",  INST_I, OPCODE_ANDI,  -1 },
    { "ori",   INST_I, OPCODE_ORI,   -1 },
    { "xori",  INST_I, OPCODE_XORI,  -1 },
    { "slti",  INST_I, OPCODE_SLTI,  -1 },
    { "sltiu", INST_I, OPCODE_SLTIU, -1 },
    { "beq",   INST_I, OPCODE_BEQ,   -1 },
    { "bne",   INST_I, OPCODE_BNE,   -1 },
    { "lw",    INST_I, OPCODE_LW,    -1 },
    { "sw",    INST_I, OPCODE_SW,    -1 },
    /* ── J-type ───────────────────────────────────────── */
    { "j",     INST_J, OPCODE_J,     -1 },
    { "jal",   INST_J, OPCODE_JAL,   -1 },
    /* sentinel */
    { NULL,    INST_UNKNOWN, 0, 0 }
};

/*
 * opcode_find  –  Return a pointer to the entry for 'mnemonic',
 *                 or NULL if not found.
 */
const Opcode_entry *opcode_find(const char *mnemonic)
{
    const Opcode_entry *e;
    for (e = OPCODE_TABLE; e->mnemonic != NULL; e++)
    {
        if (strcmp(e->mnemonic, mnemonic) == 0)
            return e;
    }
    return NULL;
}
