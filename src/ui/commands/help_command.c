#include "help_command.h"

void print_help() {
    printf("add <parent_id> <key1>=<value1> <key2>=<value2> ...\n");
    printf("Adds the specified node to tree. Given arguments must match the full pattern.\n\n");
    printf("update <node_id> <key1>=<upd_value1> <key2>=<upd_value2> ...\n");
    printf("Updates some (one or more) fields of specified node.\n\n");
    printf("remove <node_id>\n");
    printf("Removes specified node with all descendants.\n\n");
    printf("find_by field <field name> <value>\n");
    printf("Finds node(s) with specified field.\n\n");
    printf("find_by id <id>\n");
    printf("Finds node with specified id.\n\n");
    printf("find_by parent <parent id>\n");
    printf("Finds children of specified parent.\n\n");
    printf("print header/array\n");
    printf("Prints to stdout fields of header or array instances.\n\n");
    printf("exit\n");
    printf("Exit the program.\n\n");

}