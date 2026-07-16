#ifndef ASSEMBLER_H
#define ASSEMBLER_H

char *read_line(void);
void del_line(const char *line);
int commend_parse_value(const char *commend);
void commend_parse(int commend);
void I_parse(int commend);
void R_parse(int commend);
void J_parse(int commend);

#endif // ASSEMBLER_H
