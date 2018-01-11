#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <errno.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "code_generation.h"
#include "utils.h"

#define die(...) die_("codegen", __VA_ARGS__)

#define TABLE_OF_LABELS_SIZE (MAX_LABELS / 5 * 4)
struct LabelAddress {
    uintmax_t       hash;
    char            *label;
    uint_fast16_t   address;
    bool            undefined;
};
static uint_fast32_t        numberOfLabels = 0;
static struct LabelAddress  *labels[TABLE_OF_LABELS_SIZE];

static struct LabelAddress *
lookupLabel(char *label) {
    uintmax_t hash = stringHash(label);
    size_t index = hash % TABLE_OF_LABELS_SIZE;
    struct LabelAddress *la;
    while (NULL != (la = labels[index])) {
        if (hash == la->hash && !strcmp(label, la->label)) {
            return la;
        }
        index = (index + 1) % TABLE_OF_LABELS_SIZE;
    }
    return NULL;
}

static void
addLabel(struct LabelAddress *newLabelAddress) {
    char *label = newLabelAddress->label;
    uintmax_t hash = newLabelAddress->hash = stringHash(label);
    size_t index = hash % TABLE_OF_LABELS_SIZE;
    struct LabelAddress *la;
    while (NULL != (la = labels[index])) {
        if (hash == la->hash && !strcmp(label, la->label)) {
            free(la);
            --numberOfLabels;
            break;
        }
        index = (index + 1) % TABLE_OF_LABELS_SIZE;
    }
    ++numberOfLabels;
    if (MAX_LABELS < numberOfLabels) die("Too many labels");
    labels[index] = newLabelAddress;
}

static uint_least8_t machineCode[BUFFER_SIZE];
static uint_fast16_t instructionPointer      = 0;
static uint_fast16_t numberOfUndefinedLabels = 0;

extern void
cgInit(void) {
    /* Next is required for standard complience. `labels` will be filled with
     * zeroes on launch hence label will have all bits set to zero but NULL can
     * have different binary representation so we still have to manually assign
     * NULL to each pointer. */
    for (size_t i = 0; i < TABLE_OF_LABELS_SIZE; ++i) {
        labels[i] = NULL;
    }
}

extern void
cgEmitLabel(char *label) {
    struct LabelAddress *la = lookupLabel(label);
    if (la) {
        if (!la->undefined) { /* Second definition. */
            die("label(%s) defined multiple times.", label);
        }
        /* First definition. Label was used before. */
        uint_fast16_t address = MIN_ADDRESS + instructionPointer;
        uint_fast16_t i, j = la->address;
        do {
            i = j;
            j = (machineCode[i] & 0x0f) << 8 | machineCode[i + 1];
            machineCode[i    ] = (machineCode[i] & 0xf0) | ((address >> 8) & 0x0f);
            machineCode[i + 1] = address & 0xff;
        } while (j);
        la->undefined = false;
        la->address   = address;
        --numberOfUndefinedLabels;
    } else { /* First definition. Label was not used before. */
        la = emalloc(sizeof(struct LabelAddress));
        *la = (struct LabelAddress) {
            .label     = label,
            .address   = MIN_ADDRESS + instructionPointer,
            .undefined = false,
        };
        addLabel(la);
    }
}

extern void
cgSaveMachineCodeToFile(const char *path) {
    if (numberOfUndefinedLabels) die("There are undefined labels");
    FILE *f = fopen(path, "w");
    if (!f) die("Can't open file, %s", strerror(errno));
    size_t n = fwrite(machineCode, sizeof(machineCode[0]), (size_t)instructionPointer, f);
    if (instructionPointer != n) die("Can't write to file: %s", strerror(errno));
    fclose(f);
}


static void
emit(uint_fast16_t instruction) {
    if (BUFFER_SIZE <= instructionPointer) {
        die("Number of instructions exceeds memory limit.");
    }
    machineCode[instructionPointer++] = (instruction >> 8u) & 0xff;
    machineCode[instructionPointer++] = instruction & 0xff;
}

static void
check(uint_fast16_t x, unsigned n) {
    assert(n < 16);
    uint_fast16_t max = (uint_fast16_t)1 << n;
    if (x < max) return;
    die("%"PRIuLEAST16" is too big number.", x);
}

static void
emitHnnni(uint_fast16_t h, uint_fast16_t nnn) {
    check(h, 4);
    check(nnn, 12);
    emit(h << 12 | nnn);
}

static void
emitHnnnl(uint_fast16_t h, char *label) {
    struct LabelAddress *la = lookupLabel(label);
    if (!la) { /* If label was neither defined nor used before. */
        /* Save instruction pointer in the table of labels to fill address when
         * it will be defined. */
        la = emalloc(sizeof(struct LabelAddress));
        *la = (struct LabelAddress) {
            .label      = label,
            .address    = instructionPointer,
            .undefined  = true,
        };
        addLabel(la);
        emitHnnni(h, 0);
        ++numberOfUndefinedLabels;
    } else if (la->undefined) { /* If label was not defined but was used before. */
        uint_fast16_t previousUsage = la->address;
        /* Update table of labels with new last usage site. */
        la->address = instructionPointer;
        emitHnnni(h, previousUsage);
    } else { /* If label was defined. */
        emitHnnni(h, la->address);
    }
}

static void
emitHxkk(uint_fast16_t h, uint_fast16_t x, uint_fast16_t kk) {
    check(h, 4);
    check(x, 4);
    check(kk, 8);
    emit(h << 12 | x << 8 | kk);
}

static void
emitHxyn(uint_fast16_t h, uint_fast16_t x, uint_fast16_t y, uint_fast16_t n) {
    check(h, 4);
    check(x, 4);
    check(y, 4);
    check(n, 4);
    emit(h << 12 | x << 8 | y << 4 | n);
}

extern void cgEmitData(uint_fast16_t data)                       { emit(data); }
extern void cgEmitCls(void)                                      { emit(0x00e0); }
extern void cgEmitRet(void)                                      { emit(0x00ee); }
extern void cgEmitJpAddri(uint_fast16_t addr)                    { emitHnnni(0x1, addr); }
extern void cgEmitJpAddrl(char *label)                           { emitHnnnl(0x1, label); }
extern void cgEmitCallAddri(uint_fast16_t addr)                  { emitHnnni(0x2, addr); }
extern void cgEmitCallAddrl(char *label)                         { emitHnnnl(0x2, label); }
extern void cgEmitSeVxByte(uint_fast16_t x, uint_fast16_t byte)  { emitHxkk(0x3, x, byte); }
extern void cgEmitSneVxByte(uint_fast16_t x, uint_fast16_t byte) { emitHxkk(0x4, x, byte); }
extern void cgEmitSeVxVy(uint_fast16_t x, uint_fast16_t y)       { emitHxyn(0x5, x, y, 0); }
extern void cgEmitLdVxByte(uint_fast16_t x, uint_fast16_t byte)  { emitHxkk(0x6, x, byte); }
extern void cgEmitAddVxByte(uint_fast16_t x, uint_fast16_t byte) { emitHxkk(0x7, x, byte); }
extern void cgEmitLdVxVy(uint_fast16_t x, uint_fast16_t y)       { emitHxyn(0x8, x, y, 0); }
extern void cgEmitOrVxVy(uint_fast16_t x, uint_fast16_t y)       { emitHxyn(0x8, x, y, 1); }
extern void cgEmitAndVxVy(uint_fast16_t x, uint_fast16_t y)      { emitHxyn(0x8, x, y, 2); }
extern void cgEmitXorVxVy(uint_fast16_t x, uint_fast16_t y)      { emitHxyn(0x8, x, y, 3); }
extern void cgEmitAddVxVy(uint_fast16_t x, uint_fast16_t y)      { emitHxyn(0x8, x, y, 4); }
extern void cgEmitSubVxVy(uint_fast16_t x, uint_fast16_t y)      { emitHxyn(0x8, x, y, 5); }
extern void cgEmitShrVx(uint_fast16_t x)                         { emitHxkk(0x8, x, 0x06); }
extern void cgEmitSubnVxVy(uint_fast16_t x, uint_fast16_t y)     { emitHxyn(0x8, x, y, 7); }
extern void cgEmitShlVx(uint_fast16_t x)                         { emitHxkk(0x8, x, 0x0E); }
extern void cgEmitSneVxVy(uint_fast16_t x, uint_fast16_t y)      { emitHxyn(0x9, x, y, 0); }
extern void cgEmitLdIAddri(uint_fast16_t addr)                   { emitHnnni(0xA, addr); }
extern void cgEmitLdIAddrl(char *label)                          { emitHnnnl(0xA, label); }
extern void cgEmitJpV0Addri(uint_fast16_t addr)                  { emitHnnni(0xB, addr); }
extern void cgEmitJpV0Addrl(char *label)                         { emitHnnnl(0xB, label); }
extern void cgEmitRndVxByte(uint_fast16_t x, uint_fast16_t byte) { emitHxkk(0xC, x, byte); }
extern void cgEmitDrwVxVyNibble(uint_fast16_t x, uint_fast16_t y, uint_fast16_t nibble)
                                                                 { emitHxyn(0xD, x, y, nibble); }
extern void cgEmitSkpVx(uint_fast16_t x)                         { emitHxkk(0xE, x, 0x9E); }
extern void cgEmitSknpVx(uint_fast16_t x)                        { emitHxkk(0xE, x, 0xA1); }
extern void cgEmitLdVxDt(uint_fast16_t x)                        { emitHxkk(0xF, x, 0x07); }
extern void cgEmitLdVxK(uint_fast16_t x)                         { emitHxkk(0xF, x, 0x0A); }
extern void cgEmitLdDtVx(uint_fast16_t x)                        { emitHxkk(0xF, x, 0x15); }
extern void cgEmitLdStVx(uint_fast16_t x)                        { emitHxkk(0xF, x, 0x18); }
extern void cgEmitAddIVx(uint_fast16_t x)                        { emitHxkk(0xF, x, 0x1E); }
extern void cgEmitLdFVx(uint_fast16_t x)                         { emitHxkk(0xF, x, 0x29); }
extern void cgEmitLdBVx(uint_fast16_t x)                         { emitHxkk(0xF, x, 0x33); }
extern void cgEmitLdIIVx(uint_fast16_t x)                        { emitHxkk(0xF, x, 0x55); }
extern void cgEmitLdVxII(uint_fast16_t x)                        { emitHxkk(0xF, x, 0x65); }
