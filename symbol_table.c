/*
 * symbol_table.c  –  Symbol table management.
 *
 *  The symbol table stores every label seen in the source,
 *  together with its address and attribute flags.
 *  After the first pass completes, data-section labels must have
 *  their addresses relocated by the final IC value.
 */
#include "defs.h"


/*
 * sym_add  –  Insert a new symbol.
 *             Returns  0  on success,
 *                     -1  if table is full,
 *                     -2  if the name was already defined (error).
 */
int sym_add(const char *name, unsigned int addr, Symbol_attr attr)
{
    int i;

    if (g_sym_count >= MAX_SYMBOL_TABLE)
    {
        fprintf(stderr, "FATAL: symbol table overflow\n");
        return -1;
    }

    /* check for duplicate definition (extern/entry are allowed later) */
    for (i = 0; i < g_sym_count; i++)
    {
        if (strcmp(g_sym_table[i].name, name) == 0)
        {
            /* allow adding ATTR_ENTRY / ATTR_EXTERN on top of existing */
            if (attr == ATTR_ENTRY || attr == ATTR_EXTERN)
            {
                g_sym_table[i].attr = (Symbol_attr)(g_sym_table[i].attr | attr);
                return 0;
            }
            return -2; /* true redefinition */
        }
    }

    strncpy(g_sym_table[g_sym_count].name, name, MAX_LABEL_LEN - 1);
    g_sym_table[g_sym_count].name[MAX_LABEL_LEN - 1] = '\0';
    g_sym_table[g_sym_count].address = addr;
    g_sym_table[g_sym_count].attr    = attr;
    g_sym_count++;
    return 0;
}

/*
 * sym_find  –  Return a pointer to the symbol named 'name',
 *              or NULL if not found.
 */
Symbol *sym_find(const char *name)
{
    int i;
    for (i = 0; i < g_sym_count; i++)
    {
        if (strcmp(g_sym_table[i].name, name) == 0)
            return &g_sym_table[i];
    }
    return NULL;
}

/*
 * sym_update_data_labels  –  Called after the first pass finishes.
 *  Data-section labels were stored with addresses relative to 0;
 *  add ic_final to each so they point into the correct memory region.
 */
void sym_update_data_labels(int ic_final)
{
    int i;
    for (i = 0; i < g_sym_count; i++)
    {
        if (g_sym_table[i].attr & ATTR_DATA)
            g_sym_table[i].address += (unsigned int)ic_final;
    }
}
