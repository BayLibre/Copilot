CC = gcc
CFLAGS = -W -Wall

BINARY = copilot-lite-serialize
LIBRARIES = -lftdi

all:
	$(CC) $(CFLAGS) Main.c $(LIBRARIES) -o $(BINARY)

clean:
	rm -f $(BINARY)
