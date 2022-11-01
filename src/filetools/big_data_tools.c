#include "big_data_tools.h"
#include "../interface/basic_crud.h"

size_t get_real_tuple_size(uint64_t pattern_size) {
    return pattern_size * SINGLE_TUPLE_VALUE_SIZE < MINIMAL_TUPLE_SIZE
           ? MINIMAL_TUPLE_SIZE
           : pattern_size * SINGLE_TUPLE_VALUE_SIZE;
}

size_t get_real_id_array_size(uint64_t pattern_size, uint64_t cur_id){
    size_t real_tuple_size = get_real_tuple_size(pattern_size);
    if (cur_id == 0) cur_id++;
    return (cur_id * OFFSET_VALUE_SIZE / real_tuple_size) * real_tuple_size +
        cur_id * OFFSET_VALUE_SIZE % real_tuple_size ? real_tuple_size : 0;
}

static enum file_read_status read_tree_subheader(struct tree_subheader *header, FILE *file) {
    enum file_read_status code = read_from_file(header, file, sizeof(struct tree_subheader));
    return code;
}

static enum file_read_status read_key(struct key *key, FILE *file) {
    struct key_header *header = malloc(sizeof(struct key_header));
    enum file_read_status code = read_from_file(header, file, sizeof(struct key_header));
    key->header = header;

    char *key_value = (char *) malloc(header->size/FILE_GRANULARITY + (header->size % FILE_GRANULARITY ? FILE_GRANULARITY : 0));
    code |= read_from_file(key_value, file, header->size);
    key->key_value = key_value;

    return code;
}

enum file_read_status read_tree_header(struct tree_header *header, FILE *file, size_t *fpos) {
    fseek(file, 0, SEEK_SET);
    struct tree_subheader *subheader= (struct tree_subheader *) malloc(sizeof(struct tree_subheader));
    enum file_read_status code = read_tree_subheader(subheader, file);
    header->subheader = subheader;

    struct key **array_of_key = malloc(sizeof(struct key *) * subheader->pattern_size);
    header->pattern = array_of_key;
    for (size_t iter = subheader->pattern_size; iter-- > 0; array_of_key++){
        struct key *element_pattern = malloc(sizeof(struct key));
        code |= read_key(element_pattern, file);
        *array_of_key = element_pattern;
    }

    size_t real_id_array_size = get_real_id_array_size(header->subheader->pattern_size, header->subheader->cur_id);
    uint64_t *id_array = (uint64_t *) malloc(real_id_array_size * sizeof(uint64_t));
    header->id_sequence = id_array;
    code |= read_from_file(id_array, file, real_id_array_size * sizeof(uint64_t));

    *fpos = ftell(file);

    return code;
}

enum file_read_status read_basic_tuple(struct tuple **tuple, FILE *file, uint64_t pattern_size) {
    union tuple_header *header = (union tuple_header *) malloc(sizeof(union tuple_header));
    enum file_read_status code = read_from_file(header, file, sizeof(union tuple_header));
    if (header->alloc) {
        *tuple = (struct tuple *)header->alloc;
        free(header);
        return code;
    }
    fseek(file, -(sizeof(union tuple_header)), SEEK_CUR);
    struct tuple *temp_tuple = (struct tuple *) malloc(sizeof(struct tuple));
    temp_tuple->header = *header;
    free(header);
    header->alloc = (uint64_t) temp_tuple;
    write_to_file(header, file, sizeof(union tuple_header));


    uint64_t *data = (uint64_t *) malloc(get_real_tuple_size(pattern_size));
    code |= read_from_file(data, file, get_real_tuple_size(pattern_size));
    temp_tuple->data = data;

    *tuple = temp_tuple;

    return code;
}

enum file_read_status read_string_tuple(struct tuple **tuple, FILE *file, uint64_t pattern_size) {
    union tuple_header *header = malloc(sizeof(union tuple_header));
    enum file_read_status code = read_from_file(header, file, sizeof(union tuple_header));
    struct tuple *temp_tuple = (struct tuple *) malloc(sizeof(struct tuple));
    temp_tuple->header = *header;
    free(header);

    uint64_t *data = (uint64_t *) malloc(get_real_tuple_size(pattern_size));
    code |= read_from_file(data, file, get_real_tuple_size(pattern_size));
    temp_tuple->data = data;

    *tuple = temp_tuple;

    return code;
}

static size_t how_long_string_is(FILE *file, uint64_t offset){
    fseek(file, offset, SEEK_SET);
    size_t len = 1;
    union tuple_header *temp_header = malloc(sizeof(union tuple_header));
    read_from_file(temp_header, file, sizeof(union tuple_header));
    while(temp_header->next){
        fseek(file, temp_header->next, SEEK_SET);
        read_from_file(temp_header, file, sizeof(union tuple_header));
        len++;
    }
    return len;
}

enum file_read_status read_string_from_tuple(FILE *file, char **string, uint64_t pattern_size, uint64_t offset){
    size_t str_len = how_long_string_is(file, offset);
    size_t rts = get_real_tuple_size(pattern_size);
    *string = malloc(str_len * rts);
    struct tuple *temp_tuple;
    for(size_t iter = 0; iter < str_len; iter++) {
        fseek(file, offset, SEEK_SET);
        read_string_tuple(&temp_tuple, file, pattern_size);
        offset = temp_tuple->header.next;
        strncpy((*string) + rts * iter, (char *) temp_tuple->data, rts);
    }
    return 0;
}



static enum file_write_status write_tree_subheader(FILE *file, struct tree_subheader *subheader){
    enum file_write_status code = write_to_file(subheader, file, sizeof(struct tree_subheader));
    return code;
}

static enum file_write_status write_pattern(FILE *file, struct key **pattern, size_t pattern_size){
    enum file_write_status code = NULL_VALUE;
    for (; pattern_size-- > 0; pattern++){
        code |= write_to_file((*pattern)->header, file, sizeof(struct key_header));
        code |= write_to_file((*pattern)->key_value, file, (*pattern)->header->size);
    }
    return code;
}

static enum file_write_status write_id_sequence(FILE *file, uint64_t *id_sequence, size_t size){
    enum file_write_status code = write_to_file(id_sequence, file, size);
    return code;
}

enum file_write_status write_tree_header(FILE *file, struct tree_header *header){
    fseek(file, 0, SEEK_SET);
    size_t pattern_size = header->subheader->pattern_size;
    enum file_write_status code = write_tree_subheader(file, header->subheader);
    code |= write_pattern(file, header->pattern, pattern_size);
    size_t real_id_array_size = get_real_id_array_size(header->subheader->pattern_size, header->subheader->cur_id);
    code |= write_id_sequence(file, header->id_sequence, real_id_array_size * sizeof(uint64_t));
    return code;
}

enum file_write_status init_empty_file(FILE *file, char **pattern, uint32_t *types, size_t pattern_size, size_t *key_sizes){
    fseek(file, 0, SEEK_SET);
    struct tree_header *header = (struct tree_header *) malloc(sizeof(struct tree_header));
    generate_empty_tree_header(pattern, types, pattern_size, key_sizes, header);
    return write_tree_header(file, header);
}

enum file_open_status open_file_anyway(FILE **file, char *filename){
    enum file_open_status code = open_exist_file(filename, file);
    if (code) {
        code = open_new_file(filename, file);
    }
    return code;
}

enum file_write_status write_tuple(FILE *file, struct tuple *tuple, size_t tuple_size){
    union tuple_header *tuple_header = malloc(sizeof(union  tuple_header));
    *tuple_header = tuple->header;
    enum file_write_status code = write_to_file(tuple_header, file, sizeof(union  tuple_header));
    free(tuple_header);
    code |= write_to_file(tuple->data, file, tuple_size);
    return code;
}

void print_tree_header_from_file(FILE *file) {
    struct tree_header *header = malloc(sizeof(struct tree_header));
    size_t *pos = malloc(sizeof(size_t));
    read_tree_header(header, file, pos);
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

    size_t real_id_array_size = get_real_id_array_size(header->subheader->pattern_size, header->subheader->cur_id);
    for (size_t iter = 0; iter < (real_id_array_size / PRINT_ID_ARRAY_LEN); iter++) {
        for (size_t inner_iter = 0; inner_iter < PRINT_ID_ARRAY_LEN; inner_iter++) {
            //printf("%ld", iter * PRINT_ID_ARRAY_LEN + inner_iter);
            printf("%16lx ", header->id_sequence[iter * PRINT_ID_ARRAY_LEN + inner_iter]);
        }
        printf("\n");
    }
}

    void print_tuple_array_from_file(FILE *file){
        struct tree_header *header = malloc(sizeof(struct tree_header));
        size_t pos;
        read_tree_header(header, file, &pos);
        uint32_t* fields;
        size_t size;
        get_types(file, &fields, &size);
        struct tuple* cur_tuple = malloc(sizeof(struct tuple));

        for(size_t i = 0; i < header->subheader->cur_id; i++){
            if (header->id_sequence[i] == NULL_VALUE) continue;
            fseek(file, header->id_sequence[i], SEEK_SET);
            read_basic_tuple(&cur_tuple, file, header->subheader->pattern_size);
            printf("--- TUPLE %3zu ---\n", i);
            for(size_t iter = 0; iter < size; iter++){
                if (header->pattern[iter]->header->type == STRING_TYPE){
                    char *s;
                    read_string_from_tuple(file, &s, header->subheader->pattern_size, cur_tuple->data[iter]);
                    printf("%-20s %s\n", header->pattern[iter]->key_value, s);
                } else {
                    printf("%-20s %lu\n", header->pattern[iter]->key_value, cur_tuple->data[iter]);
                }
            }

        }
    }
