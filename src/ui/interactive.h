#ifndef INTERACTIVE_H
#define INTERACTIVE_H

#include <stdint.h>
#include <stddef.h>

void interactive_mode(char *filename, size_t pattern_size, uint32_t *pattern_types, char **pattern_names);

#endif
