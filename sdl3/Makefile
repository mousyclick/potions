CC=gcc
CFLAGS=-Wall -Wextra -ggdb `pkg-config --cflags sdl3 glew`
DLFLAGS=-fPIC -shared
BUILD=./build
RPATH=-Wl,-rpath,$(BUILD)/
HEADERS=-I./src/3rd/
LIBS=`pkg-config --libs sdl3 opengl glew` -ldl

PLUGINS=$(shell ls plugin_*.c | sed 's/^plugin_//;s/\.c$$//')

all: main $(addprefix $(BUILD)/, $(PLUGINS:%=plugin_%.so))

$(BUILD)/plugin_%.so: plugin_%.c plugin.h | $(BUILD)
	$(CC) $(CFLAGS) $(SANITIZE) $(DLFLAGS) -o $@ $< $(HEADERS) $(LIBS)

main: main.c plugin.h
	$(CC) $(CFLAGS) -o $(BUILD)/main main.c $(LIBS)

$(BUILD):
	mkdir -p $(BUILD)

clean:
	rm -f $(BUILD)/*
