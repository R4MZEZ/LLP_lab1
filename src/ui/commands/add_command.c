#include "add_command.h"

size_t add_input_item(FILE *f, char **str, size_t pattern_size, const uint32_t *pattern_types, char **pattern_names) {
    char **key_value;
    size_t count;
    uint64_t fields[pattern_size];
    size_t par_pos = -1;

    size_t test_pos;
    uint64_t test_value;

    if (!isNumeric(str[1])) {
        printf("Not-numeric id.\n");
        return 1;
    }
    for (size_t iter = 2; iter < pattern_size + 2; iter++) {
//        printf("%s\n", str[iter]);
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
                test_pos = par_pos;
                test_value = atoi(key_value[1]);
                break;
            case STRING_TYPE:
                fields[par_pos] = (uint64_t) key_value[1];
                break;
        }




        par_pos = -1;

        free(key_value);

    }
    size_t id = add_tuple(f, fields, atoi(str[1]));
    struct result_list_tuple *result = NULL;
    uint64_t *list = NULL;

    clock_t start = clock();

    get_tuple(f, &list, id);
//    find_by_field(f, test_pos, &test_value, &result);
//    remove_tuple(f, id, 0);
    clock_t end = clock();
    printf("%f\n", (double) (end - start) / CLOCKS_PER_SEC);

    return 0;
}
