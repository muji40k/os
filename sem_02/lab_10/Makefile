FLAGS := -std=gnu99 -Wpedantic -Wextra -Wall -Werror -Isrc/

.PHONY: all alld clean

TARGETS := $(foreach file, $(wildcard src/*.c), ${file:src/%.c=out/%.out})

all: $(TARGETS)

alld: FLAGS += -g3
alld: $(TARGETS)

out:
	mkdir -p out

out/%.out : src/%.c | out
	gcc $(FLAGS) -o $@ $^

clean:
	rm -f out/*.out

