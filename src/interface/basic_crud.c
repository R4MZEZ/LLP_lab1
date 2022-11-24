#include <time.h>
#include "basic_crud.h"
#include "../filetools/big_data_tools.h"
#include "crud_interface.h"

enum crud_operation_status delete_last_tuple(FILE *file, size_t full_tuple_size) {
    fseek(file, full_tuple_size, SEEK_END);
    int fd = fileno(file);
    return ftruncate(fd, ftell(file));
}



enum crud_operation_status swap_tuple_to(FILE *file, uint64_t pos_to, uint64_t pos_from, size_t tuple_size) {
    uint32_t *types;
    size_t size;
    get_types(file, &types, &size);

    if (pos_from != pos_to) {
        fseek(file, pos_from, SEEK_SET);
        void *buffer = malloc_test(tuple_size);
        read_from_file(file, buffer, tuple_size);


        fseek(file, pos_to, SEEK_SET);
        write_to_file(file, buffer, tuple_size);
        free_test(buffer);

        fseek(file, 0, SEEK_SET);
        struct tree_header *header = malloc_test(sizeof(struct tree_header));
        read_tree_header(header, file);
        uint64_t id;
        struct tuple *tpl;

        enum crud_operation_status st = offset_to_id(file, &id, pos_from);

        if (st == CRUD_INVALID) {

            fseek(file, pos_from, SEEK_SET);
            read_string_tuple(file, &tpl, size);
            union tuple_header *temp_header = malloc_test(sizeof(union tuple_header));

            if (tpl->header.next != 0) {
                fseek(file, tpl->header.next, SEEK_SET);
                read_from_file(file, temp_header, sizeof(union tuple_header));
                temp_header->prev = pos_to;
                fseek(file, tpl->header.next, SEEK_SET);
                write_to_file(file, temp_header, sizeof(union tuple_header));
            }

            fseek(file, tpl->header.prev, SEEK_SET);
            read_from_file(file, temp_header, sizeof(union tuple_header));
            if (temp_header->next == pos_from) {
                temp_header->next = pos_to;
                fseek(file, tpl->header.prev, SEEK_SET);
                write_to_file(file, temp_header, sizeof(union tuple_header));
            } else {


                struct tuple *parent;
                fseek(file, tpl->header.prev, SEEK_SET);
                read_basic_tuple(file, &parent, size);
                for (size_t iter = 0; iter < size; iter++) {
                    if (types[iter] == STRING_TYPE && parent->data[iter] == pos_from) {
                        parent->data[iter] = pos_to;
                        break;
                    }
                }

//                printf("PARENT\n data[1]: %lu data[0]: %lu\n", parent->data[1], parent->data[0]);
                fseek(file, (tpl->header.prev), SEEK_SET);
                write_tuple(file, parent, get_real_tuple_size(size));
                free_test_tuple(parent);
            }
            free_test(temp_header);
//            printf("STRING\n next: %lu, prev: %lu, pos_from: %lu, pos_to: %lu\n", tpl->header.next, tpl->header.prev, pos_from, pos_to);
        } else {

            fseek(file, pos_from, SEEK_SET);
            read_basic_tuple(file, &tpl, size);
            link_strings_to_tuple(file, tpl, pos_to);
            header->id_sequence[id] = pos_to;
            write_tree_header(file, header);

//            printf("TUPLE\nid: %lu, tpl1->data[1]: %lu, pos_from: %lu, pos_to: %lu\n", id, tpl->data[1], pos_from, pos_to);
        }


        free_test_tuple(tpl);
        free_test_tree_header(header);

    }
    free_test(types);
    return CRUD_OK;
}

enum crud_operation_status swap_last_tuple_to(FILE *file, uint64_t pos_to, size_t tuple_size){
    uint32_t *types;
    size_t size;
    get_types(file, &types, &size);

    fseek(file, (long) -(get_real_tuple_size(size) + sizeof(union tuple_header)), SEEK_END);
    uint64_t pos_from = ftell(file);

    free_test(types);
    enum crud_operation_status status = swap_tuple_to(file, pos_to, pos_from, tuple_size);
    ftruncate(fileno(file), (long) pos_from);
    return status;
}

enum crud_operation_status
insert_new_tuple(FILE *file, struct tuple *tuple, size_t full_tuple_size, uint64_t *tuple_pos) {
    fseek(file, 0, SEEK_END);
    *tuple_pos = ftell(file);
    int fd = fileno(file);
    ftruncate(fd, ftell(file) + full_tuple_size);
    enum file_write_status status = write_tuple(file, tuple, full_tuple_size - sizeof(union tuple_header));
    return status == WRITE_OK ? CRUD_OK : CRUD_INVALID;
}


enum crud_operation_status insert_string_tuple(FILE *file, char *string, size_t tuple_size, uint64_t *str_pos) {
    size_t len = strlen(string);
    size_t count = len / tuple_size + (len % tuple_size ? 1 : 0);
    struct tuple *temp_tuple = malloc_test(sizeof(struct tuple));
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
    free_test(temp_tuple);
    return 0;
}

enum crud_operation_status link_strings_to_tuple(FILE *file, struct tuple *tpl, uint64_t tpl_offset) {
    uint32_t *types;
    size_t size;
    get_types(file, &types, &size);
    struct tuple *str;

    for (uint64_t iter = 0; iter < size; iter++) {
        if (types[iter] == STRING_TYPE) {
            fseek(file, tpl->data[iter], SEEK_SET);
            read_string_tuple(file, &str, size);
            str->header.prev = tpl_offset;
            fseek(file, tpl->data[iter], SEEK_SET);
            write_tuple(file, str, size);
            free_test_tuple(str);
        }
    }
    free_test(types);
    return CRUD_OK;
}

void get_types(FILE *file, uint32_t **types, size_t *size) {
    fseek(file, 0, SEEK_SET);
    struct tree_header *header = malloc_test(sizeof(struct tree_header));
    read_tree_header(header, file);
    uint32_t *temp_types = malloc_test(header->subheader->pattern_size * sizeof(uint32_t));
    for (size_t iter = 0; iter < header->subheader->pattern_size; iter++) {
        temp_types[iter] = header->pattern[iter]->header->type;
    }
    *types = temp_types;
    *size = header->subheader->pattern_size;
    free_test_tree_header(header);
}


size_t append_to_id_array(FILE *file, uint64_t offset) {

    size_t id;
    struct tree_header *header = malloc_test(sizeof(struct tree_header));
    read_tree_header(header, file);
    uint64_t from = ftell(file);
//    printf("%lu %zu\n", header->subheader->cur_id, get_id_array_size(header->subheader->pattern_size, header->subheader->cur_id));
//    printf("%lu %zu\n", header->subheader->cur_id, get_id_array_size(header->subheader->pattern_size, header->subheader->cur_id));
    uint64_t real_tuple_size = get_id_array_size(header->subheader->pattern_size, header->subheader->cur_id);

    if (!((header->subheader->cur_id + 1) % real_tuple_size)){
        fseek(file, 0, SEEK_END);
        uint64_t cur_end = ftell(file);
        ftruncate(fileno(file), cur_end + get_real_tuple_size(header->subheader->pattern_size) + sizeof(union tuple_header));

        swap_tuple_to(file, cur_end, from, get_real_tuple_size(header->subheader->pattern_size));



        free_test_tree_header(header);
        header = malloc_test(sizeof(struct tree_header));
        read_tree_header(header, file);
    }


    header->id_sequence[header->subheader->cur_id] = offset;
    header->subheader->cur_id++;
    id = header->subheader->cur_id - 1;

    fseek(file, 0, SEEK_SET);
    if (write_tree_header(file, header) != WRITE_OK){
        printf("WRITE ERROR\n");
    }

    free_test_tree_header(header);

    return id;
}


enum crud_operation_status remove_from_id_array(FILE *file, uint64_t id, uint64_t *offset) {
    fseek(file, 0, SEEK_SET);
    struct tree_header *header = malloc_test(sizeof(struct tree_header));
    read_tree_header(header, file);
    if (header->id_sequence[id] == 0) {
        free_test_tree_header(header);
        return CRUD_INVALID;
    }
    else {
        *offset = header->id_sequence[id];
        if (header->subheader->cur_id - 1 == id) {
            header->subheader->cur_id--;
        }
        header->id_sequence[id] = 0;
        write_tree_header(file, header);
        free_test_tree_header(header);
        return CRUD_OK;
    }
}

enum crud_operation_status id_to_offset(FILE *file, uint64_t id, uint64_t *offset) {
    fseek(file, 0, SEEK_SET);
    struct tree_header *header = malloc_test(sizeof(struct tree_header));
    read_tree_header(header, file);
    if (header->id_sequence[id] == NULL_VALUE){
        free_test_tree_header(header);
        return CRUD_INVALID;
    }else{
        *offset = header->id_sequence[id];
        free_test_tree_header(header);
        return CRUD_OK;
    }

}

enum crud_operation_status offset_to_id(FILE *file, uint64_t *id, uint64_t offset) {

    struct tree_header *header = malloc_test(sizeof(struct tree_header));
    read_tree_header(header, file);

    struct tuple *tpl;
    fseek(file, offset, SEEK_SET);
    read_basic_tuple(file, &tpl, header->subheader->pattern_size);



    if (header->id_sequence[tpl->header.alloc] == offset) {
        *id = tpl->header.alloc;
        free_test_tree_header(header);
        free_test_tuple(tpl);
        return CRUD_OK;
    } else{
        free_test_tree_header(header);
        free_test_tuple(tpl);
        return CRUD_INVALID;
    }


//    for (size_t iter = 0; iter < header->subheader->cur_id; iter++) {
//        if (header->id_sequence[iter] == offset) {
//            *id = iter;
//            free_test(header);
//            return CRUD_OK;
//        }
//    }
//    free_test_tree_header(header);
//    return CRUD_INVALID;
}

enum crud_operation_status change_string_tuple(FILE *file, uint64_t offset, char *new_string, uint64_t size) {
    struct tuple *cur_tuple = NULL;
    uint64_t len = strlen(new_string);
    uint64_t old_offset = offset;
    do {
        offset = old_offset;
        fseek(file, offset, SEEK_SET);
        read_string_tuple(file, &cur_tuple, size);
        fseek(file, offset, SEEK_SET);
        cur_tuple->data = (uint64_t *) (new_string);
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




