FLAGS := -std=c99 -Wpedantic -Wextra -m64 -g3

.PHONY: all clean default

default: main.out

# TARGETS := $(foreach file, $(wildcard *.c), ${file:.c=.out})

# test:
# 	echo ${TARGETS}

# all: ${TARGETS}

%.out : *.c
	gcc $(FLAGS) -o $@ *.c

clean:
	rm -f *.out

