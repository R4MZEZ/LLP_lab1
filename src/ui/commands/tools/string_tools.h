#ifndef LOW_LEVEL_PROGRAMMING_LAB_1_STRING_TOOLS_H
#define LOW_LEVEL_PROGRAMMING_LAB_1_STRING_TOOLS_H
#include "../../../interface/crud_interface.h"
#include <stdbool.h>
#include "../add_command.h"

char *concat(const char *s1, const char *s2);
bool isNumeric(const char *str);
size_t split(char *str, char c, char ***arr);
void parse_file(FILE *to, FILE *from);

#endif
