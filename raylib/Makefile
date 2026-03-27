CC=gcc
CFLAGS=-Wall -Wextra -ggdb `pkg-config --cflags raylib`
BUILD=./build
HEADERS=
LIBS=`pkg-config --libs raylib` -lm

main: main.c | $(BUILD)
	$(CC) $(CFLAGS) -o $(BUILD)/main main.c $(LIBS)

$(BUILD):
	mkdir -p $(BUILD)

clean:
	rm -f $(BUILD)/*
