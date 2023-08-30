CC = gcc
CFLAGS = -W -Wall

BINARY = copilot-lite-serialize
INCLUDES = $(shell libftdi1-config --cflags)
LIBRARIES = -lftdi1

all:
	$(CC) $(CFLAGS) Main.c $(INCLUDES) $(LIBRARIES) -o $(BINARY)

clean:
	rm -f $(BINARY)
