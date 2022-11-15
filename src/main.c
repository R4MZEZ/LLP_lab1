#include "ui/interactive.h"
#include "interface/basic_crud.h"

void init_file(FILE *file);

int main(int argc, char **argv) {

    char* main_filename;
    char* arg_filename;
    FILE *file;
    FILE* parsable;
    char s[32];
    char flag;


    if (argc < 3 || argc > 4){
        printf("Wrong number of args!");
        return 1;
    }else if (argc == 4){
        arg_filename = argv[3];
    }
    main_filename = argv[2];
    flag = argv[1][1];



    switch (flag) {
        case 'o':
            open_exist_file(main_filename, &file);
            break;
        case 'n':
            open_empty_file(main_filename, &file);
            init_file(file);
            break;
        case 'p':
            open_exist_file(arg_filename, &parsable);
            open_empty_file(main_filename, &file);
            init_file(file);
            parse_file(file, parsable);
            break;
        default:
            printf("Unknown flag: -%c", flag);
            return 1;
    }
    interactive_mode(file);
    return 0;
}

void init_file(FILE *file) {
    printf("Initializing pattern.\nInput the number of fields in pattern: ");
    size_t count;
    scanf("%ld", &count);
    char *str;
    char **str_array = malloc(count * sizeof(char *));
    uint32_t type;
    uint32_t *types = malloc(count * sizeof(uint32_t));
    size_t *sizes = malloc(count * sizeof(size_t));
    size_t temp_size;
    for (size_t iter = 0; iter < count; iter++) {
        printf("--- Field %-3zu ---\n", iter);
        str = malloc(1024);
        printf("Enter field name: ");
        scanf("%s", str);
        str_array[iter] = str;
        temp_size = strlen(str);
        sizes[iter] = temp_size + (!(temp_size % FILE_GRANULARITY) ? 1 : 0);
        printf("%d. Boolean\n", BOOLEAN_TYPE);
        printf("%d. Integer\n", INTEGER_TYPE);
        printf("%d. Float\n", FLOAT_TYPE);
        printf("%d. String\n", STRING_TYPE);
        printf("Choose field type: ");
        scanf("%d", &type);
        types[iter] = type;
    }
    init_empty_file(file, str_array, types, count, sizes);
    for (size_t iter = 0; iter < count; iter++) free(str_array[iter]);
    free(str_array);
    free(sizes);
    free(types);

}
