TARGET = echo_server

TARGET2 = client

LIBS = -lm
CC = gcc
CFLAGS =  -g -Wall -Wno-unused-variable -Wno-unused-but-set-variable

.PHONY: default all clean

default: $(TARGET) $(TARGET2)
	ctags * -R

#OBJECTS = $(patsubst %.c, %.o, $(wildcard *.c))

#제꺼.
OBJECTS = main.o server.o common.o
HEADERS = $(wildcard *.h)

#수정씨꺼.
OBJECTS2 = client.o	 common.o

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

.PRECIOUS: $(TARGET) $(OBJECTS)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -pthread -Wall $(LIBS) -o $@

$(TARGET2): $(OBJECTS2)
	$(CC) $(OBJECTS2) -Wall $(LIBS) -o $@

jaehwan: $(TARGET)
	ctags * -R

soojeong: $(TARGET2)
	ctags * -R

clean:
	-rm -f *.o
	-rm -f $(TARGET)
	-rm -f $(TARGET2)
