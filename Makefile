PKGS=sdl2 freetype2 glew
CFLAGS=-Wall -Wextra -pedantic -ggdb
LIBS=-lm

jed: main.c la.c editor.c file.c gl_extra.c sdl_extra.c free_font.c cursor.c
	$(CC) $(CFLAGS) `pkg-config --cflags ${PKGS}` -o jed $^ `pkg-config --libs ${PKGS}` $(LIBS)
