#include "interactive.h"

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

        } else if (strcmp(arr[0], "update") == 0) {
            size_t code = update_item(f, arr, pattern_size, pattern_types, pattern_names, c);
            if (code != 0) {
                printf("Error code: %zu\n", code);
            }
        } else if (strcmp(arr[0], "remove") == 0) {
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
