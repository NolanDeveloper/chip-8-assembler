.SUFFIXES:
.PRECIOUS: build/%.c

CFLAGS += -g -std=c99 -pedantic -Wall -Wextra
CPPFLAGS += -Isrc -Ibuild
OBJS := 

.PHONY: all
all: build/as

.PHONY: clean
clean:
	mv ./build/.gitignore ./src/.gitignore
	$(RM) -rf ./build && mkdir build
	mv ./src/.gitignore ./build/.gitignore 

build/%.o: build/%.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

build/%.o: src/%.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

build/%.c: src/%.y
	ln -fT $< $(patsubst %.c, %.y, $@)
	lemon -q $(patsubst %.c, %.y, $@)

build/%.c: src/%.re 
	re2c $< -o $@

OBJS += build/main.o
OBJS += build/utils.o
OBJS += build/lexer.o
OBJS += build/parser.o

build/main.o: build/parser.o

build/as: $(OBJS)
	$(CC) $(LDFLAGS) $(LDLIBS) -o $@ $^
