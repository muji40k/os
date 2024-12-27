.PHONY: clean all

TARGETS = $(wildcard *.c)
CFILES = $(wildcard .\src\*.c) $(wildcard .\src\cp\*.c)
FLAGS := -Wall -Werror -Wextra -Wpedantic

all: $(TARGETS:.c=.exe)

run: all
	.\rw.exe

drun: debug
	.\rw.exe

clean:
	rm *.exe

debug: FLAGS += -g3
debug: all

%.exe: %.c inc/*.h src/*.c inc/**/*.h src/**/*c
	gcc $(FLAGS) -Iinc -Iinc/$(@:.exe=) -o $@ ./$(@:.exe=.c) $(CFILES)


