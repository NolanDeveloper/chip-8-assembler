#define _POSIX_C_SOURCE 200809L

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>

#include <uthash.h>

#include "utils.h"

#define die(...) die_("main", __VA_ARGS__)

#include "lexer.h"
#include "parser_common.h"
#include "parser.h"

extern void *ParseAlloc(void *(*mallocProc)(size_t));
extern void ParseFree(void *p, void (*freeProc)(void*));
extern void Parse(void *p, int type, union TokenData data, struct Parse *parse);

/*

План:
1)   Сделать хорошие сообщения об ошибках.
2)   Реализовать номальные метки!

*/

#define MIN_ADDRESS 0x200
#define MAX_ADDRESS 0x1000
#define BUFFER_SIZE (MAX_ADDRESS - MIN_ADDRESS)

static const char *inputFilePath  = NULL;
static const char *outputFilePath = NULL;

static int line     = 1;
static int column   = 1;

static uint16_t machineCode[BUFFER_SIZE];
static int      currentInstruction = 0;

struct LabelPosition {
    char            *label;
    int             position;
    UT_hash_handle  hh;
} *labels = NULL;

static void             *parser;
static struct Token     token;

static void 
error(void) {
    fprintf(stderr, "oh shi... at %d:%d\n", line, column);
    /* ToDo: good error messages */
}

#define LITTLE_TO_BIG(x) ((((x) >> 8) & 0xff) | ((x) & 0xff) << 8)

static void 
addInstruction(uint16_t instruction) {
    if (BUFFER_SIZE <= currentInstruction) {
        die("Number of instructions exceeds memory limit.");
    }
    machineCode[currentInstruction++] = LITTLE_TO_BIG(instruction);
}

static void 
createLabel(char *label) {
    struct LabelPosition *lp;
    HASH_FIND_STR(labels, label, lp);
    if (lp) die("label(%s) is defined multiple times.", label);
    lp = emalloc(sizeof(struct LabelPosition));
    *lp = (struct LabelPosition) {
        .label = label,
        .position = MIN_ADDRESS + 2 * currentInstruction,
    };
    HASH_ADD_STR(labels, label, lp);
}

static int 
getLabelAddress(char *label) {
    struct LabelPosition *lp;
    HASH_FIND_STR(labels, label, lp);
    if (!lp) die("Unknown label: %s.", label);
    return lp->position;
    /* ToDo: remember label and cursor position for second pass if label doesn't exist yet */
}

static struct Parse parse = {
    .error              = error,
    .addInstruction     = addInstruction,
    .createLabel        = createLabel,
    .getLabelAddress    = getLabelAddress,
};

static void
saveMachineCode(void) {
#define TEMPLATE "CHIP8-AS-XXXXXX"
    char name[sizeof(TEMPLATE)];
    strcpy(name, TEMPLATE);
    int fd = mkstemp(name);
    if (-1 == fd) die("Can't create temporary file: %s.", strerror(errno));
    FILE *f = fdopen(fd, "w");
    if (!f) die("Can't open file.");
    if (1 != fwrite(machineCode, 2 * currentInstruction, 1, f)) {
        die("Can't write to file: %s", strerror(errno));
    }
    fclose(f);
    if (rename(name, outputFilePath)) {
        die("Can't move temp file to output file location: %s", strerror(errno));
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "usage: %s InputFile.s OutputFile.rom\n", argv[0]);
        exit(1);
    }
    inputFilePath = argv[1];
    outputFilePath = argv[2];
    char *input, *cursor;
    input = cursor = loadFile(inputFilePath);
    parser = ParseAlloc(emalloc);
    while ((token.type = lexerNextToken(&cursor, &token.data, &line, &column))) {
#ifndef NDEBUG
        lexerPrintToken(stderr, token.type, token.data, line, column);
#endif
        Parse(parser, token.type, token.data, &parse);
    }
    Parse(parser, 0, token.data, &parse);
    free(input);
    saveMachineCode();
}
