.PHONY: clean all

TARGETS = $(wildcard *.c)

all: $(TARGETS:.c=.out)

clean:
	rm -f *.out

%.out: %.c inc/*.h src/*.c inc/**/*.h src/**/*c
	gcc -Wall -Werror -Wextra -Wpedantic -Iinc -Iinc/$(@:.out=) -o $@ $(@:.out=.c) src/*.c src/$(@:.out=)/*.c


