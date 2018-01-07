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
extern void Parse(void *p, int type, union TokenData data);

#define MIN_ADDRESS 0x200
#define MAX_ADDRESS 0x1000
#define BUFFER_SIZE (MAX_ADDRESS - MIN_ADDRESS)

static const char *inputFilePath  = NULL;
static const char *outputFilePath = NULL;

static int line   = 1;
static int column = 1;

static struct LabelAddress {
    char            *label;
    int             address;
    UT_hash_handle  hh;
} *labels = NULL;

#define PLACEHOLDER_ADDRESS    (-1)

static uint16_t             machineCode[BUFFER_SIZE];
static struct LabelAddress  *forwardLabel[BUFFER_SIZE];
static int                  currentInstruction = 0;

static void             *parser;
static struct Token     token;

extern void 
error(void) {
    die("Error at %d:%d", line, column);
}

static uint16_t
swapBytes(uint16_t x) {
    return (((x) >> 8) & 0xff) | ((x) & 0xff) << 8;
}

extern void 
addInstruction(uint16_t instruction) {
    if (BUFFER_SIZE <= currentInstruction) {
        die("Number of instructions exceeds memory limit.");
    }
    machineCode[currentInstruction++] = swapBytes(instruction);
}

extern void 
addLabel(char *label) {
    struct LabelAddress *la;
    HASH_FIND_STR(labels, label, la);
    if (la && PLACEHOLDER_ADDRESS == la->address) {
        la->address = MIN_ADDRESS + 2 * currentInstruction;
    } else if (!la) {
        la = emalloc(sizeof(struct LabelAddress));
        *la = (struct LabelAddress) {
            .label      = label,
            .address    = MIN_ADDRESS + 2 * currentInstruction,
        };
        HASH_ADD_STR(labels, label, la);
    } else {
        die("label(%s) is defined multiple times.", label);
    }
}

extern int 
getLabelAddress(char *label) {
    struct LabelAddress *la;
    HASH_FIND_STR(labels, label, la);
    if (!la) {
        la = emalloc(sizeof(struct LabelAddress));
        *la = (struct LabelAddress) {
            .label      = label,
            .address    = PLACEHOLDER_ADDRESS,
        };
        HASH_ADD_STR(labels, label, la);
        forwardLabel[currentInstruction] = la;
    }
    return la->address;
}

static void
fillForwardLabelAddresses(void) {
    for (int i = 0; i < currentInstruction; ++i) {
        struct LabelAddress *la = forwardLabel[i];
        if (NULL == la) continue;
        if (PLACEHOLDER_ADDRESS == la->address) {
            die("Unknown label: %s", la->label);
        }
        uint16_t code       = swapBytes(machineCode[i]);
        uint16_t address    = la->address; /* notify overflow? */
        machineCode[i]      = swapBytes((code & 0xf000) | (address & 0x0fff));
    }
}

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
    char *input, *cursor;
    if (argc != 3) {
        fprintf(stderr, "usage: %s InputFile.s OutputFile.rom\n", argv[0]);
        exit(1);
    }
    inputFilePath   = argv[1];
    outputFilePath  = argv[2];
    input = cursor  = loadFile(inputFilePath);
    parser          = ParseAlloc(emalloc);
    while ((token.type = lexerNextToken(&cursor, &token.data, &line, &column))) {
        Parse(parser, token.type, token.data);
    }
    Parse(parser, 0, token.data);
    free(input);
    fillForwardLabelAddresses();
    saveMachineCode();
}
