#include "basic_crud.h"

enum crud_operation_status delete_last_tuple(FILE *file, size_t full_tuple_size) {
    fseek(file, full_tuple_size, SEEK_END);
    int fd = fileno(file);
    return ftruncate(fd, ftell(file));
}

enum crud_operation_status swap_tuple_to(FILE *file, struct tuple *tuple_to_swap, uint64_t pos, size_t tuple_size) {
    fseek(file, (long) pos, SEEK_SET);
    return (enum crud_operation_status) write_tuple(file, tuple_to_swap, tuple_size);
}

enum crud_operation_status
insert_new_tuple(FILE *file, struct tuple *tuple, size_t full_tuple_size, uint64_t *tuple_pos) {
    fseek(file, 0, SEEK_END);
    *tuple_pos = ftell(file);
    int fd = fileno(file);
    ftruncate(fd, ftell(file) + full_tuple_size);
    return (enum crud_operation_status) write_tuple(file, tuple, full_tuple_size - sizeof(union tuple_header));
}


enum crud_operation_status insert_string_tuple(FILE *file, char *string, size_t tuple_size, uint64_t *str_pos) {
    size_t len = strlen(string);
    size_t count = len / tuple_size + (len % tuple_size ? 1 : 0);
    struct tuple *temp_tuple = malloc(sizeof(struct tuple));
    char *temp_tuple_content = string;
    size_t pos = (size_t) ftell(file);
    uint64_t fake_pos;
    fseek(file, 0, SEEK_END);
    *str_pos = ftell(file);
    for (size_t iter = 0; count > iter; iter++) {
        if (count - 1 == iter) {
            temp_tuple->header.next = 0;
        } else {
            temp_tuple->header.next = pos + (tuple_size + sizeof(union tuple_header)) * (iter + 1);
        }
        if (0 == iter) {
            temp_tuple->header.prev = 0;
        } else {
            temp_tuple->header.prev = pos + (tuple_size + sizeof(union tuple_header)) * (iter - 1);
        }
        temp_tuple->data = (uint64_t *) (temp_tuple_content + tuple_size * iter);
        insert_new_tuple(file, temp_tuple, tuple_size + sizeof(union tuple_header), &fake_pos);
    }
    return 0;
}

void get_types(FILE *file, uint32_t **types, size_t *size) {
    fseek(file, 0, SEEK_SET);
    struct tree_header *header = malloc(sizeof(struct tree_header));
    size_t pos;
    read_tree_header(header, file, &pos);
    uint32_t *temp_types = malloc(header->subheader->pattern_size * sizeof(uint32_t));
    for (size_t iter = 0; iter < header->subheader->pattern_size; iter++) {
        temp_types[iter] = header->pattern[iter]->header->type;
    }
    *types = temp_types;
    *size = header->subheader->pattern_size;
    free(header);
}

enum crud_operation_status change_parameter(FILE *file, enum tree_subheader_parameter parameter, uint64_t value) {
    fseek(file, 0, SEEK_SET);
    struct tree_header *header = malloc(sizeof(struct tree_header));
    size_t pos;
    read_tree_header(header, file, &pos);
    switch (parameter) {
        case PAR_CURRENT_ID:
            header->subheader->cur_id = value;
            break;
        case PAR_FIRST_SEQ:
            header->subheader->first_seq = value;
            break;
        case PAR_SECOND_SEQ:
            header->subheader->second_seq = value;
            break;
        case PAR_ROOT_OFFSET:
            header->subheader->root_offset = value;
            break;
        default:
            break;
    }
    write_tree_header(file, header);
    free(header);
    return 0;
}

enum crud_operation_status append_to_id_array(FILE *file, uint64_t offset) {
    fseek(file, 0, SEEK_SET);
    struct tree_header *header = malloc(sizeof(struct tree_header));
    size_t pos;
    read_tree_header(header, file, &pos);
    header->id_sequence[header->subheader->cur_id] = offset;
    header->subheader->cur_id++;
    write_tree_header(file, header);
    free(header);
    return 0;
}

enum crud_operation_status remove_from_id_array(FILE *file, uint64_t id) {
    fseek(file, 0, SEEK_SET);
    struct tree_header *header = malloc(sizeof(struct tree_header));
    size_t pos;
    read_tree_header(header, file, &pos);
    if (header->id_sequence[id] == 0)
        return CRUD_INVALID;
    else {
        header->id_sequence[id] = 0;
        if (header->subheader->cur_id == id) {
            header->subheader->cur_id--;
        }
        write_tree_header(file, header);
        free(header);
        return CRUD_OK;
    }
}

enum crud_operation_status id_to_offset(FILE *file, uint64_t id, uint64_t* offset) {
    fseek(file, 0, SEEK_SET);
    struct tree_header *header = malloc(sizeof(struct tree_header));
    size_t pos;
    read_tree_header(header, file, &pos);
    return header->id_sequence[id];
}




