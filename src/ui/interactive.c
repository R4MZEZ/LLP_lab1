#include "../interface/crud_interface.h"
#include <stdbool.h>

void print_help();

bool isNumeric(const char *str);

size_t split(char *str, char c, char ***arr);

size_t add_input_item(FILE *f, char **str, size_t pattern_size, const uint32_t *pattern_types, char **pattern_names);

void
find_by(FILE *f, char **arr, size_t pattern_size, const uint32_t *pattern_types, char **pattern_names, size_t count);

void interactive_mode(char *filename, size_t pattern_size, const uint32_t *pattern_types, char **pattern_names) {
    FILE *f = NULL;
    if (open_exist_file(filename, &f) == OPEN_FAILED) {
        printf("File doesn't exist, should we create it? (Y/n): ");
        char str[1];
        int res = scanf("%s", str);
        while (strcmp(str, "Y") != 0) {
            if (strcmp(str, "n") == 0 || res == EOF) return;
            printf("Incorrect input (Y/n): ");
            res = scanf("%s", str);
        }
        open_new_file(filename, &f);
    }

    printf("File opened successfully!\n");
    printf("Type 'help' for available commands info.\n");


    char *input_str = NULL;
    size_t len = 0;
    size_t c;
    char **arr;
    getline(&input_str, &len, stdin);
//    printf("'%s'\n", input_str);
    c = split(input_str, ' ', &arr);

    while (strcmp(arr[0], "exit") != 0) {
//        printf("'%s' '%s' '%s'\n", arr[0], arr[1], arr[2]);
        if (strcmp(arr[0], "help") == 0) print_help();
        else if (strcmp(arr[0], "find_by") == 0) {
            find_by(f, arr, pattern_size, pattern_types, pattern_names, c);
        } else if (strcmp(arr[0], "add") == 0) {
            if (c == pattern_size + 2) {
                size_t code = add_input_item(f, arr, pattern_size, pattern_types, pattern_names);
                if (code != 0) {
                    printf("Error code: %zu\n", code);
                }
            } else
                printf("Wrong number of parameters(including parent_id): %lu expected, %lu entered.\n",
                       pattern_size + 1, c - 1);

        } else if (strcmp(arr[0], "update") == 0) printf("не торопи события сынок\n");

        else if (strcmp(arr[0], "remove") == 0) {
            if (c == 2) {
                if (isNumeric(arr[1])) {
                    if (remove_tuple(f, atoi(arr[1]), 0) == CRUD_INVALID)
                        printf("Such id does not exist.\n");
                    else
                        printf("Removed successfully.\n");
                } else
                    printf("Not-numeric id.\n");
            } else if (c == 1)
                printf("Missing parameter <node_id>.\n");
            else printf("Too many parameters.\n");
        } else if (strcmp(arr[0], "print") == 0) {
            if (c == 2) {
                if (strcmp(arr[1], "header") == 0)
                    print_tree_header_from_file(f);
                else if (strcmp(arr[1], "array") == 0)
                    print_tuple_array_from_file(f);
                else
                    printf("Incorrect parameter ([print header]/[print array]).\n");
            } else if (c == 1)
                printf("Missing parameter ([print header]/[print array]).\n");
            else printf("Too many parameters ([print header]/[print array]).\n");

        } else printf("Unknown command, try using 'help'\n");

        free(arr);
        getline(&input_str, &len, stdin);
        c = split(input_str, ' ', &arr);
    }
    close_file(f);
}

void print_help() {
    printf("add <parent_id> <key1>=<value1> <key2>=<value2> ...\n");
    printf("Adds the specified node to tree. Given arguments must match the full pattern.\n\n");
    printf("update <node_id> <key1>=<upd_value1> <key2>=<upd_value2> ...\n");
    printf("Updates some (one or more) fields of specified node.\n\n");
    printf("remove <node_id>\n");
    printf("Removes specified node with all descendants.\n\n");
    printf("print header/array\n");
    printf("Prints to stdout fields of header or array instances.\n\n");
    printf("exit\n");
    printf("Exit the program.\n\n");

}

bool isNumeric(const char *str) {
    while (*str != '\0') {
        if (*str < '0' || *str > '9')
            return false;
        str++;
    }
    return true;
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

size_t add_input_item(FILE *f, char **str, size_t pattern_size, const uint32_t *pattern_types, char **pattern_names) {
    char **key_value;
    size_t count;
    uint64_t fields[pattern_size];
    size_t par_pos = -1;

    if (!isNumeric(str[1])) {
        printf("Not-numeric parent_id.\n");
        return 1;
    }
    for (size_t iter = 2; iter < pattern_size + 2; iter++) {
        count = split(str[iter], '=', &key_value);
        if (count != 2) {
            return 2;
        }
        for (size_t in_iter = 0; in_iter < pattern_size; in_iter++) {
            if (strcmp(key_value[0], pattern_names[in_iter]) == 0) {
                par_pos = in_iter;
                break;
            }
        }
        if (par_pos == -1) {
            printf("'%s' field does not match pattern.\n", str[iter]);
            return 3;
        }

        double val;
        switch (pattern_types[par_pos]) {
            case BOOLEAN_TYPE:
                if (strcmp(key_value[1], "True") == 0)
                    fields[par_pos] = true;
                else if (strcmp(key_value[1], "False") == 0)
                    fields[par_pos] = false;
                else {
                    printf("Not-bool '%s' parameter.\n", key_value[1]);
                    return 4;
                }
                break;
            case FLOAT_TYPE:
                val = strtod(key_value[1], NULL);
                if (val == 0.0) {
                    printf("Not-float '%s' parameter.\n", key_value[1]);
                    return 4;
                }
                memcpy(&fields[par_pos], &val, sizeof(val));
                break;
            case INTEGER_TYPE:
                if (!isNumeric(key_value[1])) {
                    printf("Not-integer '%s' parameter.\n", key_value[1]);
                    return 4;
                }
                fields[par_pos] = atoi(key_value[1]);
                break;
            case STRING_TYPE:
                fields[par_pos] = (uint64_t) key_value[1];
                break;
        }
        free(key_value);
        par_pos = -1;
    }
    add_tuple(f, fields, atoi(str[1]));
    return 0;
}

void
find_by(FILE *f, char **arr, size_t pattern_size, const uint32_t *pattern_types, char **pattern_names, size_t count) {
    struct result_list_tuple *result = NULL;
    if (strcmp(arr[1], "parent") == 0) {
        if (count == 3) {
            if (isNumeric(arr[2]))
                find_by_parent(f, atoi(arr[2]), &result);
            else
                printf("Not-integer parent id: %s\n", arr[2]);
        } else
            printf("Wrong number of arguments: 3 expected, %lu entered.\n", count - 1);
    } else if (strcmp(arr[1], "field") == 0) {
        if (count == 4) {
            int field_idx = -1;
            for (int i = 0; i < pattern_size; i++) {
                if (strcmp(pattern_names[i], arr[2]) == 0) {
                    field_idx = i;
                }
            }
            if (field_idx == -1)
                printf("Such field does not exist: %s\n", arr[2]);
            else {
                uint32_t type = pattern_types[field_idx];
                switch (type) {
                    case BOOLEAN_TYPE:
                        if (strcmp(arr[3], "True") == 0) {
                            bool cond = true;
                            find_by_field(f, field_idx, &cond, &result);
                        } else if (strcmp(arr[3], "False") == 0) {
                            bool cond = false;
                            find_by_field(f, field_idx, &cond, &result);
                        } else
                            printf("Not-bool value: %s\n", arr[3]);
                        break;
                    case INTEGER_TYPE:
                        if (isNumeric(arr[3])) {
                            uint64_t cond = atoi(arr[3]);
                            find_by_field(f, field_idx, &cond, &result);
                        } else
                            printf("Not-integer value: %s\n", arr[3]);
                        break;
                    case FLOAT_TYPE:
                        if (strtod(arr[3], NULL) != 0) {
                            double cond_tmp = strtod(arr[3], NULL);
                            uint64_t cond;
                            memcpy(&cond, &cond_tmp, sizeof(cond_tmp));
                            find_by_field(f, field_idx, &cond, &result);
                        } else
                            printf("Not-float value: %s\n", arr[3]);
                        break;
                    case STRING_TYPE:
                        find_by_field(f, field_idx, (uint64_t *) arr[3], &result);
                        break;
                    default:
                        printf("Unknown type\n");

                }
            }

        } else
            printf("Wrong number of arguments: 3 expected, %lu entered.\n", count - 1);

    } else
        printf("Incorrect parameter ([find_by parent]/[find_by field]).\n");

    if (result != NULL) {
        printf("--- FIND RESULT ---\n");
        do {
            printf("id: %lu\n", (uint64_t) result->id);
            result = result->prev;
        } while (result != NULL);

    } else
        printf("no result present\n");
}

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

void parse_file(FILE *to, FILE *from, size_t pattern_size, const uint32_t *pattern_types, char **pattern_names) {
    char *line = NULL;
    char **args = NULL;

    while (!feof(from)) {
        line = readln(from);
        if (strcmp(line, "") == 0)
            break;
        line[strlen(line) - 1] = '\0';
        char *prefix = concat("add ", line);
        split(prefix, ' ', &args);
        size_t code = add_input_item(to, args, pattern_size, pattern_types, pattern_names);
        if (code != 0) {
            printf("Error code %zu\n In line: %s\n", code, line);
        }
        free(line);
    }
    fclose(from);
    fflush(to);
}




