#include <stdio.h>
#include <stdio.h>
#include <math.h>

const char* MACRO_NAME = "mcro";
const int MAX_MACRO_SIZE = 100;
const char* MACRO_TABLE[MAX_MACRO_SIZE];
const char* END_MACRO = "mcroend";

void main()
{
    int IC =100;
    int DC =0;
    int macro =0;
    int macro_table_index =0;
    char* line;
    while(1)
    {
        line =read_line();

        if(is_macro(line))
        {
            continue;
        }
        if(line == MACRO_NAME)
        {
            char* current_line =read_line();
            if (macro_table_index > MAX_MACRO_SIZE)
            {
                MACRO_TABLE[macro_table_index] = current_line;
                macro_table_index+= sizeof(current_line);
            }
            else
            {
                printf("Macro table is full. Cannot add more macros.\n");
            }
            continue;
        }
        macro = line==END_MACRO;
        if (macro)
        {
            del_line(line);
            continue;
        }
        else
        {
            break;
        }
    }
}
char* read_line()
{
    FILE *file = fopen("input.txt", "r");

    if (file == NULL) 
    {
        perror("fopen");
        return NULL;
    }

    char line[256];

    if (fgets(line, sizeof(line), file) != NULL) 
    {
        int i;
        for (i = 0; line[i] != '\n'; i++) 
        {
            if (line[i] == '\n')
            {
                break;
            }
        }
    }
    fclose(file);
    return line;
}
int is_macro(char* line)
{
    for (int i = 0; line[i] != '\0'; i++) 
    {
        if (line[i] == '#') 
        {
            write_macro(line);
            return 1; // Line is a macro
        }
    }
    return 0; // Line is not a macro
}
void write_macro(char* line)
{
    FILE *file = fopen("macro.txt", "a");
    if (file == NULL) 
    {
        perror("fopen");
        return;
    }

    fwrite(line, sizeof(char), strlen(line), file);
    //change to write in the correct format place and the correct stuff
    fclose(file);
}
void del_line(char* line)
{
    FILE *file = fopen("input.txt", "r");
    if (file == NULL) 
    {
        perror("fopen");
        return;
    }

    FILE *temp_file = fopen("temp.txt", "w");
    if (temp_file == NULL) 
    {
        perror("fopen");
        fclose(file);
        return;
    }

    char current_line[256];
    while (fgets(current_line, sizeof(current_line), file) != NULL) 
    {
        if (strcmp(current_line, line) != 0) 
        {
            fputs(current_line, temp_file);
        }
    }

    fclose(file);
    fclose(temp_file);

    // Replace the original file with the temporary file
    remove("input.txt");
    rename("temp.txt", "input.txt");
}