/*
 * assembler.c  –  Main driver: ties all phases together.
 *
 *  Usage:  assembler <file1.as> [file2.as …]
 *
 *  For each source file  <stem>.as  the driver:
 *    Phase 0  pre_process()   →  <stem>.am
 *    Phase 1  first_pass()    reads <stem>.am, fills images + symbol table
 *    Phase 2  second_pass()   re-reads <stem>.am, patches labels,
 *                              writes <stem>.ob / <stem>.ent / <stem>.ext
 *
 *  Global images and counters are reset between files.
 */
#include "defs.h"

/* ── Globals defined here (declared extern in defs.h) ───────── */
unsigned int  g_code_image[MAX_CODE_IMAGE];
unsigned char g_data_image[MAX_DATA_IMAGE];
int           g_ic          = IC_INIT;
int           g_dc          = 0;
int           g_error_count = 0;
Symbol        g_sym_table[MAX_SYMBOL_TABLE];
int           g_sym_count   = 0;

/* ── Forward declaration ─────────────────────────────────────── */
static void reset_globals(void);
static void build_path(char *dest, size_t dest_size,
                       const char *stem, const char *ext);
static int  get_stem(char *dest, size_t dest_size, const char *path);

/* ────────────────────────────────────────────────────────────
 * main
 * ──────────────────────────────────────────────────────────── */
int main(int argc, char *argv[])
{
    int file_idx;
    int total_errors = 0;

    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <file.as> [file2.as …]\n", argv[0]);
        return 1;
    }

    for (file_idx = 1; file_idx < argc; file_idx++)
    {
        char stem[256];
        char am_path[256];
        char ob_path[256];
        char ent_path[256];
        char ext_path[256];
        int  rc;

        /* ── extract stem (strip .as suffix if present) ─── */
        if (get_stem(stem, sizeof(stem), argv[file_idx]) != 0)
        {
            fprintf(stderr, "ERROR: path too long: %s\n", argv[file_idx]);
            total_errors++;
            continue;
        }

        build_path(am_path,  sizeof(am_path),  stem, ".am");
        build_path(ob_path,  sizeof(ob_path),  stem, ".ob");
        build_path(ent_path, sizeof(ent_path), stem, ".ent");
        build_path(ext_path, sizeof(ext_path), stem, ".ext");

        reset_globals();

        printf("=== Assembling: %s ===\n", argv[file_idx]);

        /* ── Phase 0: pre-processing ────────────────────── */
        rc = pre_process(argv[file_idx], am_path);
        if (rc != 0 || g_error_count > 0)
        {
            fprintf(stderr, "  Pre-processor failed (%d errors).\n",
                    g_error_count);
            total_errors += g_error_count;
            continue;
        }
        printf("  Phase 0 OK  →  %s\n", am_path);

        /* ── Phase 1: first pass ────────────────────────── */
        rc = first_pass(am_path);
        if (rc != 0 || g_error_count > 0)
        {
            fprintf(stderr, "  First pass failed (%d errors).\n",
                    g_error_count);
            total_errors += g_error_count;
            continue;
        }
        printf("  Phase 1 OK  IC=%d  DC=%d\n", g_ic, g_dc);

        /* ── Phase 2: second pass ───────────────────────── */
        rc = second_pass(am_path, ob_path, ent_path, ext_path);
        if (rc != 0 || g_error_count > 0)
        {
            fprintf(stderr, "  Second pass failed (%d errors).\n",
                    g_error_count);
            total_errors += g_error_count;
            continue;
        }
        printf("  Phase 2 OK  →  %s\n", ob_path);
        printf("=== Done ===\n\n");
    }

    return (total_errors > 0) ? 1 : 0;
}

/* ── Helpers ─────────────────────────────────────────────────── */

static void reset_globals(void)
{
    memset(g_code_image, 0, sizeof(g_code_image));
    memset(g_data_image, 0, sizeof(g_data_image));
    memset(g_sym_table,  0, sizeof(g_sym_table));
    g_ic          = IC_INIT;
    g_dc          = 0;
    g_error_count = 0;
    g_sym_count   = 0;
}

static void build_path(char *dest, size_t dest_size,
                       const char *stem, const char *ext)
{
    snprintf(dest, dest_size, "%s%s", stem, ext);
}

/*
 * get_stem  –  Copy 'path' into 'dest', stripping a trailing ".as"
 *              if present.  Returns 0 on success, -1 if path is too long.
 */
static int get_stem(char *dest, size_t dest_size, const char *path)
{
    size_t len = strlen(path);
    size_t stem_len;

    if (len + 1 > dest_size)
        return -1;

    stem_len = len;
    if (len >= 3 &&
        path[len - 3] == '.' &&
        path[len - 2] == 'a' &&
        path[len - 1] == 's')
    {
        stem_len = len - 3;
    }

    strncpy(dest, path, stem_len);
    dest[stem_len] = '\0';
    return 0;
}
