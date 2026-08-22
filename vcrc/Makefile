CC = gcc
CFLAGS = -Wall -Wextra -g -std=c11
TARGET = processflow
SRC = main.c parser.c task.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all clean
