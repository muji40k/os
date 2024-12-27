KVER := $(shell uname -r)
KDIR := /lib/modules/$(KVER)/build

SRCS := $(wildcard *.c)
OBJS := $(SRCS:.c=.o)

obj-m := tasklet.o

.PHONY: build clean default test

default: build

test:
	@echo $(OBJS)

build:
	make -C $(KDIR) M=$(shell pwd) modules

clean:
	make -C $(KDIR) M=$(shell pwd) clean

