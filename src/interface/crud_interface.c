#include "crud_interface.h"

size_t add_tuple(FILE *file, uint64_t *fields, uint64_t parent_id) {

    uint32_t *types;
    size_t size;
    get_types(file, &types, &size);
    struct tree_header* header = malloc(sizeof(struct tree_header));
    read_tree_header(header, file);

    struct tuple* new_tuple = malloc_test(sizeof(struct tuple));
    union tuple_header new_tuple_header = {.parent = parent_id, .alloc = header->subheader->cur_id};

    new_tuple->header = new_tuple_header;
    new_tuple->data = malloc_test(get_real_tuple_size(size));
    uint64_t link;


    for (size_t iter = 0; iter < size; iter++) {
        if (types[iter] == STRING_TYPE) {
//            char* param = NULL;
//            memcpy(&param, &fields[iter], sizeof(fields[iter]));
            insert_string_tuple(file, fields[iter], get_real_tuple_size(size), &link);
            new_tuple->data[iter] = link;
        } else {
            new_tuple->data[iter] = (uint64_t) fields[iter];
        }
    }


    size_t full_tuple_size = sizeof(union tuple_header) + get_real_tuple_size(size);
    enum crud_operation_status status = insert_new_tuple(file, new_tuple, full_tuple_size, &link);



    link_strings_to_tuple(file, new_tuple, link);

    size_t id = append_to_id_array(file, link);

    free_test_tuple(new_tuple);
    free_test(types);
    free_test_tree_header(header);

    return status;
}

enum crud_operation_status get_tuple(FILE *file, uint64_t **fields, uint64_t id) {
    uint64_t offset;
    if (id_to_offset(file, id, &offset) == CRUD_INVALID) return CRUD_INVALID;
    struct tuple *cur_tuple;
    uint32_t *types;
    size_t size;
    get_types(file, &types, &size);
    fseek(file, offset, SEEK_SET);
    read_basic_tuple(file, &cur_tuple, (uint64_t) size);
    *fields = malloc_test(sizeof(uint64_t) * size);
    for (size_t iter = 0; iter < size; iter++) {
        if (types[iter] == STRING_TYPE) {
            char *s;
            read_string_from_tuple(file, &s, size, cur_tuple->data[iter]);
            memcpy(&(*fields)[iter], &s, sizeof(s));
        } else {
            (*fields)[iter] = cur_tuple->data[iter];
        }

    }
    free_test_tuple(cur_tuple);
    free_test(types);
    return CRUD_OK;
}

enum crud_operation_status remove_tuple(FILE *file, uint64_t id, uint8_t str_flag) {
    uint32_t *types;
    size_t size;
    get_types(file, &types, &size);
//    printf("%lu\n", id);

    if (!str_flag) {
        uint64_t offset;
        if (remove_from_id_array(file, id, &offset) == CRUD_INVALID) {
            // invalid id
            return CRUD_INVALID;
        }

        for (size_t field_num = 0; field_num < size; field_num++) {
            if (types[field_num] == STRING_TYPE) {
                struct tuple *tpl;
                fseek(file, (long) offset, SEEK_SET);
                read_basic_tuple(file, &tpl, size);
                remove_tuple(file, tpl->data[field_num], 1);
                free_test_tuple(tpl);
            }
        }


        swap_last_tuple_to(file, offset, get_real_tuple_size(size));

        struct result_list_tuple *children = NULL;
        find_by_parent(file, id, &children);
        while (children != NULL) {
            remove_tuple(file, children->id, 0);
            children = children->prev;
        }
    } else {
        struct tuple *str_tpl;
        while (id != NULL_VALUE) {
            fseek(file, id, SEEK_SET);
            read_string_tuple(file, &str_tpl, size);
            swap_last_tuple_to(file, id, get_real_tuple_size(size) + sizeof(union tuple_header));
            id = str_tpl->header.next;
            free_test_tuple(str_tpl);
        }

    }
    free_test(types);
    return CRUD_OK;
}

static void append_to_result_list(struct tuple **tuple_to_add, uint64_t id, struct result_list_tuple **result) {
    if ((*result) == NULL) {
        *result = malloc_test(sizeof(struct result_list_tuple));
        (*result)->prev = NULL;
    } else {
        struct result_list_tuple *new_result = malloc_test(sizeof(struct result_list_tuple));
        new_result->prev = *result;
        *result = new_result;
    }
    (*result)->value = *tuple_to_add;
    (*result)->id = id;
    *tuple_to_add = malloc_test(sizeof(struct tuple));
}

enum crud_operation_status
find_by_field(FILE *file, uint64_t field_number, uint64_t *condition, struct result_list_tuple **result) {
    uint32_t *types;
    size_t size;
    get_types(file, &types, &size);
    uint64_t type = types[field_number];
    struct tree_header *header = malloc_test(sizeof(struct tree_header));
    read_tree_header(header, file);
    struct tuple *cur_tuple = NULL;
    for (size_t i = 0; i < header->subheader->cur_id; i++) {
        if (header->id_sequence[i] == NULL_VALUE) continue;
        fseek(file, header->id_sequence[i], SEEK_SET);
        read_basic_tuple(file, &cur_tuple, size);
        if (type == STRING_TYPE) {
            char *s;
            read_string_from_tuple(file, &s, size, cur_tuple->data[field_number]);
            if (!strcmp(s, (char *) condition)) {
                append_to_result_list(&cur_tuple, i, result);
            }
            free_test(s);
        } else {
            if (cur_tuple->data[field_number] == *condition) {
                append_to_result_list(&cur_tuple, i, result);
            }
        }

    }
    free_test_tree_header(header);
    free_test(types);
    return 0;
}

enum crud_operation_status find_by_parent(FILE *file, uint64_t parent_id, struct result_list_tuple **result) {
    struct tree_header *header = malloc_test(sizeof(struct tree_header));
    read_tree_header(header, file);
    struct tuple *cur_tuple = malloc_test(sizeof(struct tuple));
    for (size_t i = 0; i < header->subheader->cur_id; i++) {
        if (header->id_sequence[i] == NULL_VALUE) continue;
        fseek(file, header->id_sequence[i], SEEK_SET);
        read_basic_tuple(file, &cur_tuple, header->subheader->pattern_size);
        if (cur_tuple->header.parent == parent_id) {
            append_to_result_list(&cur_tuple, i, result);
        }

    }
    free_test_tree_header(header);
    return 0;
}

enum crud_operation_status update_tuple(FILE *file, uint64_t field_number, uint64_t *new_value, uint64_t id) {
    uint32_t *types;
    size_t size;
    get_types(file, &types, &size);
    uint64_t type = types[field_number];
    uint64_t offset;
    id_to_offset(file, id, &offset);
    struct tuple *cur_tuple;
    fseek(file, offset, SEEK_SET);
    read_basic_tuple(file, &cur_tuple, size);
    if (type == STRING_TYPE) {
        change_string_tuple(file, cur_tuple->data[field_number], (char *) new_value, get_real_tuple_size(size));
    } else {

        memcpy(&(cur_tuple->data[field_number]), new_value, sizeof(*new_value));
//        cur_tuple->data[field_number] = *new_value;
        fseek(file, offset, SEEK_SET);

        write_tuple(file, cur_tuple, get_real_tuple_size(size));
    }
    free_test_tuple(cur_tuple);
    free_test(types);
    return 0;
}

void print_tuple_array_from_file(FILE *file) {
    struct tree_header header;
    read_tree_header(&header, file);
    uint32_t *types;
    size_t size;
    get_types(file, &types, &size);
    struct tuple *cur_tuple;

    for (size_t i = 0; i < header.subheader->cur_id; i++) {
        if (header.id_sequence[i] == NULL_VALUE) continue;
        fseek(file, header.id_sequence[i], SEEK_SET);
        read_basic_tuple(file, &cur_tuple, size);
        printf("--- TUPLE %3zu ---\n", i);
        for (size_t iter = 0; iter < size; iter++) {
            if (types[iter] == STRING_TYPE) {
                char *s;
                read_string_from_tuple(file, &s, header.subheader->pattern_size, cur_tuple->data[iter]);
                printf("%-20s %s\n", header.pattern[iter]->key_value, s);
                free_test(s);
            } else if (types[iter] == INTEGER_TYPE || types[iter] == BOOLEAN_TYPE) {
                printf("%-20s %lu\n", header.pattern[iter]->key_value, cur_tuple->data[iter]);
            } else if (types[iter] == FLOAT_TYPE) {
                double res;
                memcpy(&res, &(cur_tuple->data[iter]), sizeof(cur_tuple->data[iter]));
                printf("%-20s %.6f\n", header.pattern[iter]->key_value, res);
            }
        }
        free_test_tuple(cur_tuple);
    }
    free_test(types);

}

void print_tree_header_from_file(FILE *file) {
    struct tree_header* header = malloc_test(sizeof(struct tree_header));
    read_tree_header(header, file);
    printf("--- SUBHEADER ---\n");
    printf("%-20s%ld\n", "ASCII Signature: ", header->subheader->ASCII_signature);
    printf("%-20s%ld\n", "Root Offset: ", header->subheader->root_offset);
    printf("%-20s%ld\n", "First Sequence: ", header->subheader->first_seq);
    printf("%-20s%ld\n", "Second Sequence: ", header->subheader->second_seq);
    printf("%-20s%ld\n", "Current ID: ", header->subheader->cur_id);
    printf("%-20s%ld\n", "Pattern Size: ", header->subheader->pattern_size);
    printf("--- PATTERN ---\n");
    for (size_t iter = 0; iter < header->subheader->pattern_size; iter++) {
        printf("Key %3d [Type %3d]: %s\n",
               header->pattern[iter]->header->size, header->pattern[iter]->header->type,
               header->pattern[iter]->key_value);
    }
    printf("--- ID ARRAY ---\n");

    size_t real_id_array_size = get_id_array_size(header->subheader->pattern_size, header->subheader->cur_id);
    for (size_t iter = 0; iter < (header->subheader->cur_id / PRINT_ID_ARRAY_LEN); iter++) {
        for (size_t inner_iter = 0; inner_iter < PRINT_ID_ARRAY_LEN; inner_iter++) {
            //printf("%ld", iter * PRINT_ID_ARRAY_LEN + inner_iter);
            printf("%16lx ", header->id_sequence[iter * PRINT_ID_ARRAY_LEN + inner_iter]);
        }
        printf("\n");
    }

    free_test_tree_header(header);
}