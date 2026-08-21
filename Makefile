CC = gcc
CFLAGS = -Wall -Wextra

TARGET = vcrc/processflow
SRC = vcrc/src/main.c

all:
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET)