DIR_SRC         := src
DIR_HEADER      := inc
DIR_OUT         := out
DIR_UI          := ui

ADD_DIRS_SRC    := $(foreach dir, $(shell find $(DIR_SRC)/ -type d), $(dir:$(DIR_SRC)/%=%))
ADD_DIRS_HEADER := $(foreach dir, $(shell find $(DIR_HEADER)/ -type d), $(dir:$(DIR_HEADER)/%=%))
ADD_DIRS_OUT    := $(ADD_DIRS_SRC)
DIRS_OUT        := $(DIR_OUT) $(foreach dir, $(ADD_DIRS_OUT), $(DIR_OUT)/$(dir))

SRCS = $(wildcard $(DIR_SRC)/*.c) \
	   $(foreach dir, $(ADD_DIRS_SRC), $(wildcard $(DIR_SRC)/$(dir)/*.c))
OBJS = $(SRCS:$(DIR_SRC)/%.c=$(DIR_OUT)/%.o)
DEPS = $(wildcard $(DIR_OUT)/*.d) \
	   $(foreach dir, $(ADD_DIRS_OUT), $(wildcard $(DIR_OUT)/$(dir)/*.d))
UIS  = $(wildcard $(DIR_UI)/*.ui)

FLAGS    = -std=c99 -Wpedantic -Wextra -pthread -I$(DIR_HEADER) \
		   $(foreach dir, $(ADD_DIRS_HEADER), -I$(DIR_HEADER)/$(dir))
# FLAGS    = -I$(DIR_HEADER) \
# 		   $(foreach dir, $(ADD_DIRS_HEADER), -I$(DIR_HEADER)/$(dir))
LFLAGS   = -lm
ADDFLAGS =

MOCFLAGS = -I$(DIR_OUT) -I$(DIR_HEADER) \
		   $(foreach dir, $(ADD_DIRS_HEADER), -I$(DIR_HEADER)/$(dir))

.PHONY: build debug clean run test

run: build
run:
	./app.out

drun: debug
drun:
	./app.out

$(DIR_OUT)/.build%: | $(DIRS_OUT)
	rm -rf $(DIR_OUT)/*
	rm -f $(DIR_OUT)/.build*
	rm -f *.out
	touch $@

build: $(DIR_OUT)/.buildrelease app.out

debug: FLAGS += -g3
debug: $(DIR_OUT)/.builddebug app.out

$(DIRS_OUT):
	mkdir -p $@

$(DIR_OUT)/%.o: $(DIR_SRC)/%.c | $(DIRS_OUT)
	@gcc $(FLAGS) $(ADDFLAGS) -MM -MF $(@:%.o=%.d) -MT $@ -c -o /dev/null \
		 $(@:$(DIR_OUT)/%.o=$(DIR_SRC)/%.c)
	gcc $(FLAGS) $(ADDFLAGS) -c -o $@ $(@:$(DIR_OUT)/%.o=$(DIR_SRC)/%.c)

$(DIR_OUT)/%.h: $(DIR_UI)/%.ui | $(DIRS_OUT)
	uic -o $@ $<

$(DIR_OUT)/moc_%.c: $(DIR_HEADER)/%.h | $(DIRS_OUT)
	moc $(MOCFLAGS) -o $@ $<

app.out: $(OBJS) | $(DIRS_OUT)
	gcc $(FLAGS) -o app.out $(OBJS) $(LFLAGS)

include $(DEPS)

clean:
	rm -f $(DIR_OUT)/.build*
	rm -rf $(DIR_OUT)/*

