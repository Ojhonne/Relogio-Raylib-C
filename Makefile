CC = gcc
CFLAGS = -I./include
LDFLAGS = -L./lib
LIBS = -lraylib -lopengl32 -lgdi32 -lwinmm -luser32 -lshell32
SRC = src/main.c

init:
	$(CC) $(SRC) -o main.exe $(CFLAGS) $(LDFLAGS) $(LIBS)
	./main.exe
	