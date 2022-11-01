#include "crud_interface.h"

enum crud_operation_status add_tuple(FILE *file, uint64_t *fields, uint64_t parent_id) {
    //todo мб в prev первого кортежа строк пихать ссылку на базовый кортеж, чтобы проще было менять ссылку?
    uint32_t *types;
    size_t size;
    get_types(file, &types, &size);
    struct tuple *new_tuple = malloc(sizeof(struct tuple));
    union tuple_header new_tuple_header = {.parent = parent_id, .alloc = (uint64_t) new_tuple};
    new_tuple->header = new_tuple_header;
    new_tuple->data = malloc(size);
    uint64_t *link = malloc(sizeof(uint64_t));
    for (size_t iter = 0; iter < size; iter++) {
        if (types[iter] == STRING_TYPE) {
            insert_string_tuple(file, (char *) fields[iter], get_real_tuple_size(size), link);
            new_tuple->data[iter] = *link;
        } else {
            new_tuple->data[iter] = (uint64_t) fields[iter];
        }
    }
    size_t full_tuple_size = sizeof(union tuple_header) + get_real_tuple_size(size);
    enum crud_operation_status status = insert_new_tuple(file, new_tuple, full_tuple_size, link);
    append_to_id_array(file, *link);
    free(link);
    return status;
}

enum crud_operation_status get_tuple(FILE *file, uint64_t **fields, uint64_t id) {
    uint64_t offset;
    id_to_offset(file, id, &offset);
    if (offset == NULL_VALUE) return CRUD_INVALID;
    struct tuple *cur_tuple;
    uint32_t *types;
    size_t size;
    get_types(file, &types, &size);
    fseek(file, offset, SEEK_SET);
    read_basic_tuple(&cur_tuple, file, (uint64_t) size);
    *fields = malloc(sizeof(uint64_t) * size);
    for (size_t iter = 0; iter < size; iter++) {
        if (types[iter] == STRING_TYPE) {
            char *s;
            read_string_from_tuple(file, &s, size, cur_tuple->data[iter]);
            (*fields)[iter] = (uint64_t) s;
        } else {
            (*fields)[iter] = cur_tuple->data[iter];
        }
    }
}

enum crud_operation_status remove_tuple(FILE *file, uint64_t id, uint8_t str_flag) {
    uint32_t *types;
    size_t size;
    get_types(file, &types, &size);

    if (!str_flag) {
        uint64_t offset;
        if (remove_from_id_array(file, id, &offset) == CRUD_INVALID) {
            // invalid id
            return CRUD_INVALID;
        }

        for (size_t field_num = 0; field_num < size; field_num++) {
            if (types[field_num] == STRING_TYPE) {
                uint64_t tuple_off;
                id_to_offset(file, id, &tuple_off);
                struct tuple *tpl;
                fseek(file, (long) tuple_off, SEEK_SET);
                read_basic_tuple(&tpl, file, size);
                remove_tuple(file, tpl->data[field_num], 1);
            }
        }


        fseek(file, (long) -(get_real_tuple_size(size) + sizeof(union tuple_header)), SEEK_END);
        uint64_t pos_from = ftell(file);
        uint64_t pos_to = offset;

        swap_tuple_to(file, pos_from, pos_to, get_real_tuple_size(size) + sizeof(union tuple_header));

        struct result_list_tuple *children = NULL;
        find_by_parent(file, id, &children);
        if (children != NULL) {
            void *start = children;
            do {
                remove_tuple(file, children->id, 0);
                children = children->next;
            } while (children != start);
        }
    }else{
        fseek(file, id, SEEK_SET);
        struct tuple *str_tpl;
        read_string_tuple(&str_tpl, file, size);

        while (id != NULL_VALUE){
            fseek(file, (long) -(get_real_tuple_size(size) + sizeof(union tuple_header)), SEEK_END);
            uint64_t pos_from = ftell(file);
            uint64_t pos_to = id;

            fseek(file, str_tpl->header.next, SEEK_SET);
            read_string_tuple(&str_tpl, file, size);
            id = str_tpl->header.next;

            swap_tuple_to(file, pos_from, pos_to, get_real_tuple_size(size) + sizeof(union tuple_header));
        }

    }
    return CRUD_OK;
}

static void append_to_result_list(struct tuple **tuple_to_add, uint64_t id, struct result_list_tuple **result) {
    if ((*result) == NULL) {
        *result = malloc(sizeof(struct result_list_tuple));
        (*result)->prev = (*result);
        (*result)->next = (*result);
    } else {
        struct result_list_tuple *new_result = malloc(sizeof(struct result_list_tuple));
        (*result)->prev->next = new_result;
        (*result)->prev = new_result;
        new_result->prev = (*result)->prev;
        new_result->next = *result;
        *result = new_result;
    }
    (*result)->value = *tuple_to_add;
    (*result)->id = id;
    *tuple_to_add = malloc(sizeof(struct tuple));
}

enum crud_operation_status
find_by_field(FILE *file, uint64_t field_number, uint64_t *condition, struct result_list_tuple **result) {
    uint32_t *types;
    size_t size;
    get_types(file, &types, &size);
    uint64_t type = types[field_number];
    struct tree_header *header = malloc(sizeof(struct tree_header));
    size_t pos;
    read_tree_header(header, file, &pos);
    struct tuple *cur_tuple = malloc(sizeof(struct tuple));
    for (size_t i = 0; i < header->subheader->cur_id; i++) {
        if (header->id_sequence[i] == NULL_VALUE) continue;
        fseek(file, header->id_sequence[i], SEEK_SET);
        read_basic_tuple(&cur_tuple, file, size);
        if (type == STRING_TYPE) {
            char *s;
            read_string_from_tuple(file, &s, size, cur_tuple->data[field_number]);
            if (!strcmp(s, (char *) condition)) {
                append_to_result_list(&cur_tuple, i, result);
            }
        } else {
            if (cur_tuple->data[field_number] == *condition) {
                append_to_result_list(&cur_tuple, i, result);
            }
        }

    }
    return 0;
}

enum crud_operation_status find_by_parent(FILE *file, uint64_t parent_id, struct result_list_tuple **result) {
    struct tree_header *header = malloc(sizeof(struct tree_header));
    size_t pos;
    read_tree_header(header, file, &pos);
    struct tuple *cur_tuple = malloc(sizeof(struct tuple));
    for (size_t i = 0; i < header->subheader->cur_id; i++) {
        if (header->id_sequence[i] == NULL_VALUE) continue;
        fseek(file, header->id_sequence[i], SEEK_SET);
        read_basic_tuple(&cur_tuple, file, header->subheader->pattern_size);
        if (cur_tuple->header.parent == parent_id) {
            append_to_result_list(&cur_tuple, i, result);
        }

    }
    return 0;
}

enum crud_operation_status update_tuple(FILE *file, uint64_t field_number, uint64_t *new_value, uint64_t id) {
    uint32_t *types;
    size_t size;
    get_types(file, &types, &size);
    uint64_t type = types[field_number];
    struct tree_header *header = malloc(sizeof(struct tree_header));
    size_t pos;
    read_tree_header(header, file, &pos);
    uint64_t offset;
    id_to_offset(file, id, &offset);
    struct tuple *cur_tuple = malloc(sizeof(struct tuple));
    fseek(file, offset, SEEK_SET);
    read_basic_tuple(&cur_tuple, file, size);
    if (type == STRING_TYPE) {
        change_string_tuple(file, cur_tuple->data[field_number], (char *) new_value, get_real_tuple_size(size));
    } else {
        cur_tuple->data[field_number] = *new_value;
        fseek(file, offset, SEEK_SET);
        write_tuple(file, cur_tuple, size);
    }
    return 0;
}