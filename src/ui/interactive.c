#include "../interface/crud_interface.h"
#include <stdbool.h>

void print_help();

bool isNumeric(const char *str);

int split(char *str, char c, char ***arr, char end);

void interactive_mode(char *filename, size_t pattern_size, uint32_t *pattern_types, char **pattern_names) {
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
    char **arr = malloc(sizeof(char[30]) * 30);
    getline(&input_str, &len, stdin);
    c = split(input_str, ' ', &arr, '\n');

    while (strcmp(arr[0], "exit") != 0) {
        printf("'%s' '%s' '%s'\n", arr[0], arr[1], arr[2]);
        if (strcmp(arr[0], "help") == 0) print_help();

        else if (strcmp(arr[0], "add") == 0) {
            if (c == pattern_size + 2) {
                char **key_value;
                size_t count;
                uint64_t fields[pattern_size];
                size_t par_pos = -1;
                bool error = false;

                if (!isNumeric(arr[1])) {
                    printf("Not-numeric parent_id.\n");
                    break;
                }
                for (size_t iter = 2; iter < pattern_size + 2; iter++) {
                    count = split(arr[iter], '=', &key_value, '\0');
                    if (count != 2) {
                        error = true;
                        break;
                        //#TODO error code
                    }
                    for (size_t in_iter = 0; in_iter < pattern_size; in_iter++) {
                        if (strcmp(key_value[0], pattern_names[in_iter]) == 0) {
                            par_pos = in_iter;
                            break;
                        }
                    }
                    if (par_pos == -1) {
                        printf("'%s' field does not match pattern.\n", arr[iter]);
                        break;
                    }


                    switch (pattern_types[par_pos]) {
                        case BOOLEAN_TYPE:
                            //#TODO
                            break;
                        case FLOAT_TYPE:
                            //#TODO
                            break;
                        case INTEGER_TYPE:
                            if (!isNumeric(key_value[1])) {
                                printf("Not-numeric '%s' parameter.\n", key_value[1]);
                                break;
                            }
                            fields[par_pos] = atoi(key_value[1]);
                            break;
                        case STRING_TYPE:
                            fields[par_pos] = key_value[1];
                            break;
                    }
                    par_pos = -1;
                }
                add_tuple(f, fields, atoi(arr[1]));
            } else printf("Wrong number of parameters(including parent_id): %lu expected, %lu entered.\n", pattern_size+1, c-1);

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

        getline(&input_str, &len, stdin);
        c = split(input_str, ' ', &arr, '\n');
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

int split(char *str, char c, char ***arr, const char end) {
    int count = 1;
    int token_len = 1;
    int i = 0;
    char *p;
    char *t;

    p = str;
    while (*p != end) {
        if (*p == c)
            count++;
        p++;
    }

    *arr = (char **) malloc(sizeof(char *) * count);
    if (*arr == NULL)
        exit(1);

    p = str;
    while (*p != end) {
        if (*p == c) {
            (*arr)[i] = (char *) malloc(sizeof(char) * token_len);
            if ((*arr)[i] == NULL)
                exit(1);

            token_len = 0;
            i++;
        }
        p++;
        token_len++;
    }
    (*arr)[i] = (char *) malloc(sizeof(char) * token_len);
    if ((*arr)[i] == NULL)
        exit(1);

    i = 0;
    p = str;
    t = ((*arr)[i]);
    while (*p != end) {
        if (*p != c && *p != end) {
            *t = *p;
            t++;
        } else {
            *t = '\0';
            i++;
            t = ((*arr)[i]);
        }
        p++;
    }
    printf("%d ", *((*arr)[1]+2));
    printf("%s ", (*arr)[1]);


    return count;
}
