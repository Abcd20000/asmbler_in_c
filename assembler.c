#include "defs.h"

unsigned int  code_image[MAX_CODE_IMAGE];
unsigned char data_image[MAX_DATA_IMAGE];
int           IC          = IC_INIT;
int           DC          = 0;
int           error_count = 0;
Symbol        symbol_table[MAX_SYMBOL_TABLE];
int           symbol_count = 0;

static void reset_globals(void);
static void build_path(char *dest, size_t dest_size, const char *stem, const char *ext);
static int  get_stem(char *dest, size_t dest_size, const char *path);

int main(int argc, char *argv[])
{
    int file_idx;
    int total_errors = 0;

    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <file.as>\n", argv[0]);
        return 1;
    }

    for (file_idx = 1; file_idx < argc; file_idx++)
    {
        char stem[PATH_BUF_SIZE];
        char am_path[PATH_BUF_SIZE];
        char ob_path[PATH_BUF_SIZE];
        char ent_path[PATH_BUF_SIZE];
        char ext_path[PATH_BUF_SIZE];
        int  rc;

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

        rc = pre_process(argv[file_idx], am_path);
        if (rc != 0 || error_count > 0)
        {
            fprintf(stderr, "  Pre-processor failed (%d errors).\n", error_count);
            total_errors += error_count;
            continue;
        }

        rc = first_pass(am_path);
        if (rc != 0 || error_count > 0)
        {
            fprintf(stderr, "  First pass failed (%d errors).\n", error_count);
            total_errors += error_count;
            continue;
        }
        printf("  Phase 1 OK  IC=%d  DC=%d\n", IC, DC);

        rc = second_pass(am_path, ob_path, ent_path, ext_path);
        if (rc != 0 || error_count > 0)
        {
            fprintf(stderr, "  Second pass failed (%d errors).\n", error_count);
            total_errors += error_count;
            continue;
        }
        printf("  Phase 2 OK  -> %s\n", ob_path);
        printf("=== Done ===\n\n");
    }

    return (total_errors > 0) ? 1 : 0;
}

static void reset_globals(void)
{
    memset(code_image,   0, sizeof(code_image));
    memset(data_image,   0, sizeof(data_image));
    memset(symbol_table, 0, sizeof(symbol_table));
    IC           = IC_INIT;
    DC           = 0;
    error_count  = 0;
    symbol_count = 0;
}

static void build_path(char *dest, size_t dest_size, const char *stem, const char *ext)
{
    snprintf(dest, dest_size, "%s%s", stem, ext);
}

static int get_stem(char *dest, size_t dest_size, const char *path)
{
    size_t len = strlen(path);
    size_t stem_len = len;

    if (len + 1 > dest_size)
        return GET_STEM_FAIL;

    if (len >= SRC_EXT_LEN && path[len - SRC_EXT_LEN] == '.' &&
        path[len - SRC_EXT_LEN + 1] == 'a' && path[len - 1] == 's')
        stem_len = len - SRC_EXT_LEN;

    strncpy(dest, path, stem_len);
    dest[stem_len] = '\0';
    return 0;
}
