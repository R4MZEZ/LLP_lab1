CC=gcc

BUILDDIR=build
SRCDIR=src
TARGETDIR=.

SOURCES=C:\Users\User\Desktop\llp\Low_Level_Programming_lab_1\src\main.c C:\Users\User\Desktop\llp\Low_Level_Programming_lab_1\src\filetools\basic_file_manager.c C:\Users\User\Desktop\llp\Low_Level_Programming_lab_1\src\filetools\big_data_tools.c C:\Users\User\Desktop\llp\Low_Level_Programming_lab_1\src\generator\empty_generator.c C:\Users\User\Desktop\llp\Low_Level_Programming_lab_1\src\interface\basic_crud.c C:\Users\User\Desktop\llp\Low_Level_Programming_lab_1\src\interface\crud_interface.c C:\Users\User\Desktop\llp\Low_Level_Programming_lab_1\src\ui\interactive.c C:\Users\User\Desktop\llp\Low_Level_Programming_lab_1\src\ui\commands\add_command.c C:\Users\User\Desktop\llp\Low_Level_Programming_lab_1\src\ui\commands\find_by_command.c C:\Users\User\Desktop\llp\Low_Level_Programming_lab_1\src\ui\commands\help_command.c C:\Users\User\Desktop\llp\Low_Level_Programming_lab_1\src\ui\commands\update_command.c C:\Users\User\Desktop\llp\Low_Level_Programming_lab_1\src\ui\commands\tools\string_tools.c
EXECUTABLE=main

.PHONY: all
all: build run
run:
	./$(EXECUTABLE)

build: $(EXECUTABLE)

$(EXECUTABLE): $(SOURCES)
	$(CC) $(SOURCES) -o $@
clean:
	rm -rf $(BUILDDIR) $(EXECUTABLE)