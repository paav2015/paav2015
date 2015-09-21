CFLAGS = -Wall -Wextra -O2 -g -DDEBUG=0 -std=c99
LDFLAGS := -pthread
CC = gcc
RM = rm -f

SRC = tinyjpeg.c loadjpeg.c tinyjpeg-parse.c jidctflt.c conv_yuvbgr.c huffman.c
OBJ = $(SRC:.c=.o)
EXEC = tinyjpeg

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) -o $@ $(LDFLAGS) $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(EXEC) $(OBJ)

