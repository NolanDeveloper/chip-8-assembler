CFLAGS += -g

.PHONY: all
all: as

grammar.c: grammar.y
	lemon $<

as.c: as.re grammar.c
	re2c $< -o $@

LINK = $(CC) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) -o $@

as: as.o grammar.o
	$(LINK)

.PHONY: clean
clean:
	$(RM) *.o *.out grammar.c grammar.h as.c as
