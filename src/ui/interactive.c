#include "interactive.h"

void interactive_mode(FILE *f) {
    size_t pattern_size;
    struct tree_header *header = malloc_test(sizeof(struct tree_header));

    read_tree_header(header, f);
    pattern_size = header->subheader->pattern_size;
    uint32_t *pattern_types = malloc_test(sizeof(uint32_t) * pattern_size);
    char **pattern_names = malloc_test(sizeof(char *) * pattern_size);

    for (int i = 0; i < pattern_size; i++) {
        pattern_types[i] = header->pattern[i]->header->type;
        pattern_names[i] = header->pattern[i]->key_value;
    }


    printf("File opened successfully!\n");
    printf("Type 'help' for available commands info.\n");


    char *input_str = malloc(INPUT_LINE_SIZE);
    size_t len = 0;
    size_t c;
    char **arr;
    getline(&input_str, &len, stdin);
    if (strcmp(input_str, "\n") == 0) {
        getline(&input_str, &len, stdin);
    }
    c = split(input_str, ' ', &arr);

    while (strcmp(arr[0], "exit") != 0) {
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

        free_test(arr);
        getline(&input_str, &len, stdin);
        c = split(input_str, ' ', &arr);
    }
    close_file(f);
}
