
CC = gcc

CFLAGS = -Wall -Wextra `sdl2-config --cflags`

LDFLAGS = `sdl2-config --libs`

TARGET = game_of_life

SRC = game_of_life.c

OBJ = $(SRC:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET) $(LDFLAGS)

$(OBJ): $(SRC)
	$(CC) $(CFLAGS) -c $(SRC) -o $(OBJ)

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean
