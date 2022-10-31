#ifndef BASIC_CRUD_H
#define BASIC_CRUD_H
#include "../filetools/big_data_tools.h"
#include <unistd.h>
#include <string.h>

/**
 * Удаляет последний кортеж их файла
 * @param file файл
 * @param full_tuple_size размер одного кортежа вместе с заголовком
 * @return статус операции
 */
enum crud_operation_status delete_last_tuple(FILE *file, size_t full_tuple_size);

/**
 * Вставляет на позицию кортеж
 * @param file файл
 * @param tuple_to_swap кортеж
 * @param pos позиция вставки
 * @return статус операции
 */
enum crud_operation_status swap_tuple_to(FILE *file, struct tuple *tuple_to_swap, uint64_t pos, size_t tuple_size);

/**
 * Вставляет в конец файла новый кортеж
 * @param file файл
 * @param tuple кортеж
 * @param full_tuple_size размер одного кортежа вместе с заголовком
 * @return статус операции
 */
enum crud_operation_status insert_string_tuple(FILE *file, char *string, size_t tuple_size, uint64_t *str_pos);
enum crud_operation_status insert_new_tuple(FILE *file, struct tuple *tuple, size_t full_tuple_size, uint64_t *tuple_pos);
void get_types(FILE *file, uint32_t **types, size_t *size);
enum crud_operation_status change_parameter(FILE *file, enum tree_subheader_parameter parameter, uint64_t value);
enum crud_operation_status append_to_id_array(FILE *file, uint64_t offset);
/**
 * Статус операции
 */
enum crud_operation_status {
    CRUD_OK = 0,
    CRUD_END_OF_FILE,
    CRUD_INVALID
};

#endif