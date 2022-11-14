#include "find_by_command.h"

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