#include "string_tools.h"

char *readln(FILE *stream) {
    static char *str = NULL;
    static size_t i = 0;
    int ch = fgetc(stream);

    if ((ch == '\n') || (ch == EOF)) {
        str = malloc(i + 1);
        str[i] = 0;
    } else {
        i++;
        readln(stream);
        str[--i] = ch;
    }
    return str;
}

char *concat(const char *s1, const char *s2) {
    char *result = malloc(strlen(s1) + strlen(s2) + 1);
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}

size_t split(char *str, const char c, char ***arr) {
    int count = 1;
    int i = 0;
    char *p;

    p = str;
    while (*p != '\n' && *p != '\0') {
        if (*p == c)
            count++;
        p++;
    }

    *arr = (char **) malloc(sizeof(char *) * count);
    if (*arr == NULL)
        exit(1);


    for (char *pch = strtok(str, &c); pch != NULL; pch = strtok(NULL, &c)) {
        if (i == count - 1 && pch[strlen(pch) - 1] == '\n')
            pch[strlen(pch) - 1] = '\0';
        (*arr)[i++] = pch;
    }

    return count;
}

bool isNumeric(const char *str) {
    while (*str != '\0' && *str != 13) {
        if (*str < '0' || *str > '9')
            return false;
        str++;
    }
    return true;
}

void parse_file(FILE *to, FILE *from) {
    char line[1024];
    char **args = NULL;
    size_t pattern_size;
    struct tree_header *header = malloc(sizeof(struct tree_header));

    read_tree_header(header, to);
    pattern_size = header->subheader->pattern_size;
    uint32_t *pattern_types = malloc(sizeof(uint32_t) * pattern_size);
    char **pattern_names = malloc(sizeof(char *) * pattern_size);

    for (int i = 0; i < pattern_size; i++) {
        pattern_types[i] = header->pattern[i]->header->type;
        pattern_names[i] = header->pattern[i]->key_value;
    }


    while (!feof(from)) {
//        line = readln(from);
        char* res = fgets(line,1024,from);
        printf("%ld\n", (uint64_t) res);
        if (strcmp(line, "") == 0)
            break;
        line[strlen(line) - 1] = '\0';
        char *prefix = concat("add ", line);
        split(prefix, ' ', &args);
        size_t code = add_input_item(to, args, pattern_size, pattern_types, pattern_names);
        if (code != 0) {
            printf("Error code %zu\n In line: %s\n", code, line);
        }
//        free(line);
    }



    fclose(from);
    fflush(to);
    free_tree_header(header);
    free(pattern_types);
    free(pattern_names);


}