#include "big_data_tools.h"
#include "../interface/basic_crud.h"

size_t get_real_tuple_size(uint64_t pattern_size) {
    return pattern_size * SINGLE_TUPLE_VALUE_SIZE < MINIMAL_TUPLE_SIZE
           ? MINIMAL_TUPLE_SIZE
           : pattern_size * SINGLE_TUPLE_VALUE_SIZE;
}

size_t get_real_id_array_size(uint64_t pattern_size, uint64_t cur_id) {
    size_t real_tuple_size = get_real_tuple_size(pattern_size);
    if (cur_id == 0) cur_id++;
    return (cur_id * OFFSET_VALUE_SIZE / real_tuple_size) * real_tuple_size +
           cur_id * OFFSET_VALUE_SIZE % real_tuple_size ? real_tuple_size : 0;
}

static enum file_read_status read_tree_subheader(struct tree_subheader *header, FILE *file) {
    enum file_read_status code = read_from_file(file, header, sizeof(struct tree_subheader));
    return code;
}

static enum file_read_status read_key(struct key *key, FILE *file) {
    struct key_header *header = malloc(sizeof(struct key_header));
    enum file_read_status code = read_from_file(file, header, sizeof(struct key_header));
    key->header = header;

    char *key_value = (char *) malloc(
            header->size / FILE_GRANULARITY + (header->size % FILE_GRANULARITY ? FILE_GRANULARITY : 0));
    code |= read_from_file(file, key_value, header->size);
    key->key_value = key_value;

    return code;
}

enum file_read_status read_tree_header(struct tree_header *header, FILE *file) {
    fseek(file, 0, SEEK_SET);
    struct tree_subheader *subheader = malloc(sizeof(struct tree_subheader));
    enum file_read_status code = read_tree_subheader(subheader, file);
    header->subheader = subheader;

    struct key **array_of_key = malloc(sizeof(struct key *) * subheader->pattern_size);
    header->pattern = array_of_key;
    for (size_t iter = subheader->pattern_size; iter-- > 0; array_of_key++) {
        struct key *element_pattern = malloc(sizeof(struct key));
        code |= read_key(element_pattern, file);
        *array_of_key = element_pattern;
    }

    size_t real_id_array_size = get_real_id_array_size(header->subheader->pattern_size, header->subheader->cur_id);
    uint64_t *id_array = (uint64_t *) malloc(real_id_array_size * sizeof(uint64_t));
    header->id_sequence = id_array;
    code |= read_from_file(file, id_array, real_id_array_size * sizeof(uint64_t));

    return code;
}

enum file_read_status read_basic_tuple(FILE *file, struct tuple **tuple, uint64_t pattern_size) {
    union tuple_header *header = malloc(sizeof(union tuple_header));
    enum file_read_status code = read_from_file(file, header, sizeof(union tuple_header));
//    if (header->alloc) {
//        *tuple = (struct tuple *)header->alloc;
//        free(header);
//        return code;
//    }
//    fseek(file, -(sizeof(union tuple_header)), SEEK_CUR);
    struct tuple *temp_tuple = malloc(sizeof(struct tuple));
    temp_tuple->header = *header;
//    header->alloc = (uint64_t) temp_tuple;
//    write_to_file(file, header, sizeof(union tuple_header));
    free(header);


    uint64_t *data = malloc(get_real_tuple_size(pattern_size));
    code |= read_from_file(file, data, get_real_tuple_size(pattern_size));
    temp_tuple->data = data;

    *tuple = temp_tuple;

    return code;
}

enum file_read_status read_string_tuple(FILE *file, struct tuple **tuple, uint64_t pattern_size) {
    union tuple_header *header = malloc(sizeof(union tuple_header));
    enum file_read_status code = read_from_file(file, header, sizeof(union tuple_header));
    struct tuple *temp_tuple = malloc(sizeof(struct tuple));
    temp_tuple->header = *header;
    free(header);

    uint64_t *data = (uint64_t *) malloc(get_real_tuple_size(pattern_size));
    code |= read_from_file(file, data, get_real_tuple_size(pattern_size));
    temp_tuple->data = data;

    *tuple = temp_tuple;

    return code;
}

static size_t how_long_string_is(FILE *file, uint64_t offset) {
    fseek(file, offset, SEEK_SET);
    size_t len = 1;
    union tuple_header *temp_header = malloc(sizeof(union tuple_header));
    read_from_file(file, temp_header, sizeof(union tuple_header));
//    printf("'%lu %lu %lu'\n", offset, temp_header->next, temp_header->prev);
    while (temp_header->next) {
        fseek(file, temp_header->next, SEEK_SET);
        read_from_file(file, temp_header, sizeof(union tuple_header));
        len++;
    }
    free(temp_header);
    return len;
}

enum file_read_status read_string_from_tuple(FILE *file, char **string, uint64_t pattern_size, uint64_t offset) {
    size_t str_len = how_long_string_is(file, offset);
    size_t rts = get_real_tuple_size(pattern_size);
    *string = malloc(str_len * rts);
    struct tuple *temp_tuple;
    for (size_t iter = 0; iter < str_len; iter++) {
        fseek(file, offset, SEEK_SET);
        read_string_tuple(file, &temp_tuple, pattern_size);
        offset = temp_tuple->header.next;
        strncpy((*string) + rts * iter, (char *) temp_tuple->data, rts);
        free_tuple(temp_tuple);
    }
    return 0;
}


static enum file_write_status write_tree_subheader(FILE *file, struct tree_subheader *subheader) {
    enum file_write_status code = write_to_file(file, subheader, sizeof(struct tree_subheader));
    return code;
}

static enum file_write_status write_pattern(FILE *file, struct key **pattern, size_t pattern_size) {
    enum file_write_status code = NULL_VALUE;
    for (; pattern_size-- > 0; pattern++) {
        code |= write_to_file(file, (*pattern)->header, sizeof(struct key_header));
        code |= write_to_file(file, (*pattern)->key_value, (*pattern)->header->size);
    }
    return code;
}

static enum file_write_status write_id_sequence(FILE *file, uint64_t *id_sequence, size_t size) {
    enum file_write_status code = write_to_file(file, id_sequence, size);
    return code;
}

enum file_write_status write_tree_header(FILE *file, struct tree_header *header) {
    fseek(file, 0, SEEK_SET);
    size_t pattern_size = header->subheader->pattern_size;
    enum file_write_status code = write_tree_subheader(file, header->subheader);
    code |= write_pattern(file, header->pattern, pattern_size);
    size_t real_id_array_size = get_real_id_array_size(header->subheader->pattern_size, header->subheader->cur_id);
    code |= write_id_sequence(file, header->id_sequence, real_id_array_size * sizeof(uint64_t));
    return code;
}

enum file_write_status
init_empty_file(FILE *file, char **pattern, uint32_t *types, size_t pattern_size, size_t *key_sizes) {
    fseek(file, 0, SEEK_SET);
    struct tree_header *header = malloc(sizeof(struct tree_header));
    generate_empty_tree_header(pattern, types, pattern_size, key_sizes, header);
    enum file_write_status code = write_tree_header(file, header);
    free_tree_header(header);
    return code;
}

enum file_open_status open_file_anyway(FILE **file, char *filename) {
    enum file_open_status code = open_exist_file(filename, file);
    if (code) {
        code = open_new_file(filename, file);
    }
    return code;
}

enum file_write_status write_tuple(FILE *file, struct tuple *tuple, size_t tuple_size) {
    union tuple_header *tuple_header = malloc(sizeof(union tuple_header));
    *tuple_header = tuple->header;
    enum file_write_status code = write_to_file(file, tuple_header, sizeof(union tuple_header));
    free(tuple_header);

    code |= write_to_file(file, tuple->data, tuple_size);
    return code;
}

void free_tree_header(struct tree_header* header){
    for (size_t iter = 0; iter < header->subheader->pattern_size; iter++){
        free(header->pattern[iter]->key_value);
        free(header->pattern[iter]->header);
        free(header->pattern[iter]);
    }
    free(header->pattern);
    free(header->id_sequence);
    free(header->subheader);
    free(header);
}

void free_tuple(struct tuple* tuple){
    free(tuple->data);
    free(tuple);
}


void free_result_list(struct result_list_tuple* rlt){
//#TODO
}




