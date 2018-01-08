.SUFFIXES:
.PRECIOUS: build/%.c
.PHONY: all clean

CFLAGS += -g -std=c99 -pedantic -Wall -Wextra -Werror
CPPFLAGS += -Isrc -Ibuild
OBJS := 

all: build/as

clean:
	find ./build ! -name '.gitignore' -and ! -path './build' -exec $(RM) -rf {} +

build/%.o: build/%.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

build/%.o: src/%.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

build/%.c: src/%.y
	ln -fT $< $(patsubst %.c, %.y, $@)
	lemon -q $(patsubst %.c, %.y, $@)

build/%.c: src/%.re 
	re2c $< -o $@

OBJS += build/code_generation.o
OBJS += build/utils.o
OBJS += build/lexer.o
OBJS += build/parser.o

build/lexer.o: build/parser.o

build/as: $(OBJS)
	$(CC) $(LDFLAGS) $(LDLIBS) -o $@ $^
