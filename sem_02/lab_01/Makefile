FLAGS := -std=c99 -Wpedantic -Wextra

.PHONY: all clean

# TARGETS := $(foreach file, $(wildcard *.c), ${file:.c=.out})
TARGETS := $(foreach file, $(wildcard *.c), ${file:.c=.out})

all: $(TARGETS)

alld: FLAGS += -g3
alld: $(TARGETS)

%.out : %.c
	gcc $(FLAGS) -o $@ $^

clean:
	rm -f *.out

