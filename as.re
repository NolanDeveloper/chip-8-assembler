#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "grammar.h"

extern void * ParseAlloc(void * (*mallocProc)(size_t));
extern void ParseFree(void * p, void (*freeProc)(void*));
extern void Parse(void * p, int token, int value);

/*!max:re2c*/
#define SIZE 16

struct {
    char buf[SIZE + YYMAXFILL];
    char * lim;
    char * cur;
    char * tok;
    int eof;
} input;

static int
fill(size_t need) {
    size_t free;

    if (input.eof) return 0;
    free = input.tok - input.buf;
    if (free < need) return 0;
    memmove(input.buf, input.tok, input.lim - input.tok);
    input.lim -= free;
    input.cur -= free;
    input.tok -= free;
    input.lim += fread(input.lim, 1, free, stdin);
    if (input.lim < input.buf + SIZE) {
        input.eof = 1;
        memset(input.lim, 0, YYMAXFILL);
        input.lim += YYMAXFILL;
    }
    return 1;
}

static int
lex(int * value) {
restart:
    input.tok = input.cur;
    /*!re2c
        re2c:define:YYCTYPE = char;
        re2c:define:YYCURSOR = input.cur;
        re2c:define:YYLIMIT = input.lim;
        re2c:define:YYFILL = "if (!fill(@@)) return -1;";
        re2c:define:YYFILL:naked = 1;

        digit = [0-9];
        hex = [0-9A-F];
        ws = [ \t\n];

        'CLS'   { return CLS; }
        'RET'   { return RET; }
        'SYS'   { return SYS; }
        'JP'    { return JP; }
        'CALL'  { return CALL; }
        'SE'    { return SE; }
        'SNE'   { return SNE; }
        'LD'    { return LD; }
        'ADD'   { return ADD; }
        'OR'    { return OR; }
        'AND'   { return AND; }
        'XOR'   { return XOR; }
        'SUB'   { return SUB; }
        'SHR'   { return SHR; }
        'SUBN'  { return SUBN; }
        'SHL'   { return SHL; }
        'RND'   { return RND; }
        'DRW'   { return DRW; }
        'SKP'   { return SKP; }
        'SKNP'  { return SKNP; }

        digit+  { char c = *input.cur;
                  *input.cur = '\0';
                  *value = atoi(input.tok);
                  *input.cur = c;
                  return INTEGER; }

        'V' hex { char c = *input.cur;
                  *input.cur = '\0';
                  *value = strtol(input.tok + 1, NULL, 16);
                  *input.cur = c;
                  return V; }

        ','     { return COMMA; }
        'I'     { return I; }
        'DT'    { return DT; }
        'K'     { return K; }
        'ST'    { return ST; }
        'F'     { return F; }
        'B'     { return B; }
        '[I]'   { return II; }

        ws      { goto restart; }

        '\x00'  { return 0; }
    */
}

int main() {
    void * parser;
    int token, value;

    input.lim = input.buf + SIZE;
    input.cur = input.lim;
    input.tok = input.lim;
    input.eof = 0;

    parser = ParseAlloc(malloc);

    while (-1 != (token = lex(&value))) {
        Parse(parser, token, value);
    }
    Parse(parser, 0, value);

    ParseFree(parser, free);
}
