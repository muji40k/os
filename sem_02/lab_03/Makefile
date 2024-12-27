FLAGS := -std=c99 -Wpedantic -Wextra -m64

.PHONY: all clean

TARGETS := $(foreach file, $(wildcard *.c), ${file:.c=.out})

# test:
# 	echo ${TARGETS}

all: ${TARGETS}

%.out : %.c
	gcc $(FLAGS) -o $@ $^

clean:
	rm -f *.out

