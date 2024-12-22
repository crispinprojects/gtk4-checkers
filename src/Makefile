CC=gcc
CFLAGS=-c  -Wall 
LDFLAGS=-lm
SOURCES=main.c gameboard.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=mxcheckers

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@ -lncurses

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm $(EXECUTABLE) $(OBJECTS)
