#include "ui/interactive.h"
#include "interface/basic_crud.h"
#include "interface/crud_interface.h"


int main(int argc, char **argv) {
    FILE *file;
    open_empty_file("simple.txt", &file);
    char *par1 = "Code";
    char *par2 = "Name";
    size_t pattern_size = 2;
    char **pattern = malloc(sizeof(char *) * pattern_size);
    pattern[0] = par1;
    pattern[1] = par2;
    size_t *sizes = malloc(sizeof(size_t) * pattern_size);
    sizes[0] = 5;
    sizes[1] = 8;
    uint32_t *types = malloc(sizeof(uint32_t) * pattern_size);
    types[0] = 1;
    types[1] = 3;
    init_empty_file(file, pattern, types, pattern_size, sizes);

    uint64_t fields[2];
    fields[0] = 1024;
    fields[1] = (uint64_t) "Рома";
    add_tuple(file, fields, 1298);
    fields[0] = 123;
    fields[1] = (uint64_t) "Чепух";
    add_tuple(file, fields, 123);
    fields[0] = 321;
    fields[1] = (uint64_t) "Биба";
    add_tuple(file, fields, 0);
    fields[0] = 2;
    fields[1] = (uint64_t) "Боба";
    add_tuple(file, fields, 1);

//    close_file(file);

    remove_tuple(file, 0, 0);

    print_tree_header_from_file(file);
    print_tuple_array_from_file(file);
//
//    get_tuple(file, &fields, 0);
//
    struct result_list_tuple *result = NULL;
    uint64_t cond = 123;
    find_by_field(file, 0, &cond, &result);
    printf("--- FIND RESULT ---\n");
    if (result != NULL) printf("id: %lu\n", (uint64_t) result->id);
    else printf("no result present\n");
//
//    result = NULL;
//    find_by_field(file, 1, (uint64_t *) str, &result);
//    printf("--- FIND RESULT ---\n");
//    if (result != NULL) printf("id: %lu\n", (uint64_t) result->id);
//    else printf("no result present\n");
//
//    result = NULL;
//    cond = 1024;
//    find_by_field(file, 0, &cond, &result);
//    printf("--- FIND RESULT ---\n");
//    if (result != NULL) printf("id: %lu\n", (uint64_t) result->id);
//    else printf("no result present\n");
//
//
    result = NULL;
    find_by_parent(file, 1, &result);
    printf("--- FIND RESULT ---\n");
    if (result != NULL) printf("id: %lu\n", (uint64_t) result->id);
    else printf("no result present\n");
//
//    result = NULL;
//    find_by_parent(file, 1298, &result);
//    printf("--- FIND RESULT ---\n");
//    if (result != NULL) printf("id: %lu\n", (uint64_t) result->id);
//    else printf("no result present\n");
//
//    cond = 10;
//    update_tuple(file, 0, &cond, 0);
//
    char *new_string = "Точно не Чепух";
    update_tuple(file, 1, (uint64_t *) new_string, 1);
//
    print_tuple_array_from_file(file);

    int flag = 0;
    char* filename = "simple.txt";
//    if (argc > 3){
//        printf("Too many args!");
//        return 0;
//    }
//    for (size_t i = 0; i < argc; i++) {
//        if (argv[i][0] == '-') {
//            if (flag > 0) {
//                printf("Too many flags!");
//                return 0;
//            } else {
//                flag = argv[i][1];
//            }
//        } else {
//            filename = argv[i];
//        }
//    }
//    if (flag == 'i'){
//        interactive_mode(filename, pattern_size, types, pattern);
//    }
    return 0;
}

