#ifndef INTERACTIVE_H
#define INTERACTIVE_H

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include "../interface/crud_interface.h"
#include <stdbool.h>
#include "commands/add_command.h"
#include "commands/find_by_command.h"
#include "commands/help_command.h"
#include "commands/update_command.h"


void interactive_mode(FILE* f);


#endif
