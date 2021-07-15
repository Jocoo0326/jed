CFLAGS=-Wall -Wextra -pedantic -ggdb `pkg-config --cflags sdl2`
LIBS=`pkg-config --libs sdl2` -lm

jed: main.c la.c buffer.c
	$(CC) $(CFLAGS) -o jed $^ $(LIBS)
