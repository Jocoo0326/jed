CFLAGS=-Wall -Wextra -pedantic -ggdb `pkg-config --cflags sdl2`
LIBS=`pkg-config --libs sdl2 glew` -lm

jed: main.c la.c editor.c file.c gl_extra.c
	$(CC) $(CFLAGS) -o jed $^ $(LIBS)
