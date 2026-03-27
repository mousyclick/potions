CC=gcc
CFLAGS=-Wall -Wextra -ggdb 
INCLUDES=`pkg-config --cflags glfw3 glew`
LIBS=`pkg-config --libs glfw3 glew` -lm
BUILD=./build

all: main

main: main.c | $(BUILD)
	$(CC) $(CFLAGS) -o $(BUILD)/main main.c $(INCLUDES) $(LIBS)

$(BUILD):
	mkdir -p $(BUILD)

clean:
	rm -rf $(BUILD)/*
