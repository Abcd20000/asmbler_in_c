#include "defs.h"

int sym_add(const char *name, unsigned int addr, Symbol_attr attr)
{
    int i;

    if (symbol_count >= MAX_SYMBOL_TABLE)
    {
        fprintf(stderr, "FATAL: symbol table overflow\n");
        return SYM_ADD_OVERFLOW;
    }

    for (i = 0; i < symbol_count; i++)
    {
        if (strcmp(symbol_table[i].name, name) == 0)
        {
            if (attr == ATTR_ENTRY || attr == ATTR_EXTERN)
            {
                symbol_table[i].attr = (Symbol_attr)(symbol_table[i].attr | attr);
                return SYM_ADD_OK;
            }
            return SYM_ADD_DUPLICATE;
        }
    }

    strncpy(symbol_table[symbol_count].name, name, MAX_LABEL_LEN - 1);
    symbol_table[symbol_count].name[MAX_LABEL_LEN - 1] = '\0';
    symbol_table[symbol_count].address = addr;
    symbol_table[symbol_count].attr    = attr;
    symbol_count++;
    return SYM_ADD_OK;
}

Symbol *sym_find(const char *name)
{
    int i;
    for (i = 0; i < symbol_count; i++)
    {
        if (strcmp(symbol_table[i].name, name) == 0)
            return &symbol_table[i];
    }
    return NULL;
}

// shift data label addresses by the final IC value
void sym_update_data_labels(int ic_final)
{
    int i;
    for (i = 0; i < symbol_count; i++)
    {
        if (symbol_table[i].attr & ATTR_DATA)
            symbol_table[i].address += (unsigned int)ic_final;
    }
}
