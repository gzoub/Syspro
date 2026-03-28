CC = gcc
CFLAGS = -Wall -g -Iinclude

TARGETS = jms_coord jms_console jms_pool

all: $(TARGETS)

jms_coord: jms_coord.o src/requests.o
	$(CC) $(CFLAGS) -o $@ $^

jms_console: jms_console.o src/requests.o
	$(CC) $(CFLAGS) -o $@ $^

jms_pool: src/jms_pool.o
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c include/util.h
	$(CC) $(CFLAGS) -c $<

src/%.o: src/%.c include/requests.h include/util.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o src/*.o $(TARGETS) jms_in jms_out tmp/*
	rm -rf my_outputs/*

.PHONY: all clean