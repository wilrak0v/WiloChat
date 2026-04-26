TARGET = wlch
CC = gcc

CFLAGS = -Wall -Wextra -Isrc -O2

LDFLAGS = -lsqlite3 -lpthread

SRCS = src/main.c src/db.c src/btmp.c src/mongoose.c

OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

re: clean all

.PHONY: all clean re
