CC=gcc
CFLAGS=-Werror -Wall -g -std=gnu99
EXEC=edit
OBJECTS=editorManager.o miniEditor.o

.PHONY: default clean
default: $(EXEC)

editorManager.o: editorManager.c editorManager.h
	$(CC) -o editorManager.o -c editorManager.c $(CFLAGS)

miniEditor.o: miniEditor.c
	$(CC) -o miniEditor.o -c miniEditor.c $(CFLAGS)

$(EXEC): $(OBJECTS)
	$(CC) -o $(EXEC) $(OBJECTS)

clean:
	rm $(OBJECTS)
