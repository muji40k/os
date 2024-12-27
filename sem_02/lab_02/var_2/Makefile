FLAGS := -std=c99 -Wpedantic -Wextra -g3

.PHONY: all clean

# TARGETS := $(foreach file, $(wildcard *.c), ${file:.c=.out})
# TARGETS := $(foreach file, $(wildcard *.c), ${file:.c=.out})

all: server.out client.out

server.out : *.c *.h
	gcc $(FLAGS) -o server.out server.c socket_setup.c

client.out : *.c *.h
	gcc $(FLAGS) -o client.out client.c socket_setup.c

clean:
	rm -f *.out

