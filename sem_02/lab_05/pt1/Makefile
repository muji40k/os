KVER := $(shell uname -r)
KDIR := /lib/modules/$(KVER)/build
OBJS := md.o

obj-m := $(OBJS)

.PHONY: build clean default

default: build

build:
	make -C $(KDIR) M=$(shell pwd) modules

clean:
	make -C $(KDIR) M=$(shell pwd) clean

