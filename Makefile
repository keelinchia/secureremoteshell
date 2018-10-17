CC = gcc

CFLAGS = -ggdb -Wall -Werror -lsocket 

TARGET = client.o server.o

all: $(TARGET)

${TARGET}: %.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

.PHONY:
clean:
	rm -Rf $(TARGET)