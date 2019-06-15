
CFLAGS = -Wall -Werror -std=c99
CFLAGS += $(shell pkg-config --cflags gtk+-3.0)

LIBS = -lGL
LIBS += $(shell pkg-config --libs gtk+-3.0)

gtk-gl: gtk-gl.o
	$(CC) -o $@ $^ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $^
