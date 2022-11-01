#include "basic_crud.h"
#include "../filetools/big_data_tools.h"

enum crud_operation_status delete_last_tuple(FILE *file, size_t full_tuple_size) {
    fseek(file, full_tuple_size, SEEK_END);
    int fd = fileno(file);
    return ftruncate(fd, ftell(file));
}

enum crud_operation_status swap_tuple_to(FILE *file, uint64_t pos_from, uint64_t pos_to, size_t tuple_size) {
    if (pos_from != pos_to) {
        fseek(file, pos_from, SEEK_SET);
        void *buffer = malloc(tuple_size);

        read_from_file(buffer, file, tuple_size);
        fseek(file, pos_to, SEEK_SET);
        write_to_file(buffer, file, tuple_size);

        fseek(file, 0, SEEK_SET);
        struct tree_header *header = malloc(sizeof(struct tree_header));
        size_t pos;
        read_tree_header(header, file, &pos);
        uint64_t id;
        offset_to_id(file, &id, pos_from);
        header->id_sequence[id] = pos_to;
        write_tree_header(file, header);
        free(header);

    }
    ftruncate(fileno(file), pos_from);
    return CRUD_OK;
}

enum crud_operation_status insert_new_tuple(FILE *file, struct tuple *tuple, size_t full_tuple_size, uint64_t *tuple_pos) {
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

enum crud_operation_status remove_from_id_array(FILE *file, uint64_t id, uint64_t* offset) {
    fseek(file, 0, SEEK_SET);
    struct tree_header *header = malloc(sizeof(struct tree_header));
    size_t pos;
    read_tree_header(header, file, &pos);
    if (header->id_sequence[id] == 0)
        return CRUD_INVALID;
    else {
        *offset = header->id_sequence[id];
        if (header->subheader->cur_id-1 == id) {
            header->subheader->cur_id--;
        }
        header->id_sequence[id] = 0;
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
    *offset = header->id_sequence[id];
    return CRUD_OK;
}

enum crud_operation_status offset_to_id(FILE *file, uint64_t* id, uint64_t offset) {
    fseek(file, 0, SEEK_SET);
    struct tree_header *header = malloc(sizeof(struct tree_header));
    size_t pos;
    read_tree_header(header, file, &pos);
    for (size_t iter = 0; iter < header->subheader->cur_id; iter++){
        if (header->id_sequence[iter] == offset){
            *id = iter;
            free(header);
            return CRUD_OK;
        }
    }
    free(header);
    return CRUD_INVALID;
}

enum crud_operation_status change_string_tuple(FILE *file, uint64_t offset, char *new_string, uint64_t size) {
    struct tuple *cur_tuple = malloc(sizeof(struct tuple));
    int64_t len = strlen(new_string);
    uint64_t old_offset = offset;
    do {
        offset = old_offset;
        fseek(file, offset, SEEK_SET);
        read_basic_tuple(&cur_tuple, file, size);
        fseek(file, offset, SEEK_SET);
        cur_tuple->data = (uint64_t * )(new_string);
        new_string = new_string + size;
        write_tuple(file, cur_tuple, size);
        old_offset = cur_tuple->header.next;
        len -= size;
    } while (cur_tuple->header.next && len > 0);
    uint64_t fpos;
    if (len > 0) {
        insert_string_tuple(file, new_string, size, &fpos);
        cur_tuple->header.next = fpos;
        fseek(file, offset, SEEK_SET);
        write_tuple(file, cur_tuple, size);
    }
    return CRUD_OK;
}




