#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <errno.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "code_generation.h"

static uint_fast32_t
stringHash(const char *str) {
    uint_fast32_t hash = 5381, c;
    while ((c = *(const unsigned char *)str++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

#define TABLE_OF_LABELS_SIZE (MAX_LABELS * 5 / 4 + 1)
struct LabelAddress {
    uint_fast32_t   hash;
    char            *label;
    uint_fast16_t   address;
    bool            undefined;
};
static uint_fast16_t        numberOfLabels = 0;
static uint_fast16_t        numberOfUndefinedLabels = 0;
static struct LabelAddress  labels[TABLE_OF_LABELS_SIZE];

extern uint_fast16_t
cgGetNumberOfUndefinedLabels(void) {
    return numberOfUndefinedLabels;
}

static struct LabelAddress *
lookupLabel(char *label) {
    uint_fast32_t hash = stringHash(label);
    size_t n = TABLE_OF_LABELS_SIZE;
    for (size_t i = hash % n; labels[i].label; i = (i + 1) % n) {
        if (hash == labels[i].hash && !strcmp(label, labels[i].label)) {
            return &labels[i];
        }
    }
    return NULL;
}

static CgError
addLabel(struct LabelAddress newLabelAddress) {
    if (numberOfLabels >= MAX_LABELS) {
        return CG_TOO_MANY_LABELS;
    }
    char *label = newLabelAddress.label;
    uint_fast32_t hash = newLabelAddress.hash = stringHash(label);
    size_t i, n = TABLE_OF_LABELS_SIZE;
    for (i = hash % n; labels[i].label; i = (i + 1) % n) {
        if (hash == labels[i].hash && !strcmp(label, labels[i].label)) {
            return CG_SECOND_DEFINITION;
        }
    }
    ++numberOfLabels;
    labels[i] = newLabelAddress;
    return CG_OK;
}

#define BUFFER_SIZE (MAX_ADDRESS - MIN_ADDRESS)
static uint_least8_t machineCode[BUFFER_SIZE];
static uint_fast16_t instructionPointer = 0;

extern uint_least8_t *
cgGetMachineCode(void) {
    return machineCode;
}

extern uint_fast16_t
cgGetInstructionPointer(void) {
    return instructionPointer;
}

extern void
cgInit(void) {
    /* Next is required for standard complience. `labels` will be filled with
     * zeroes on launch hence label will have all bits set to zero but NULL can
     * have different binary representation so we still have to manually assign
     * NULL to each pointer. */
    for (size_t i = 0; i < TABLE_OF_LABELS_SIZE; ++i) {
        labels[i].label = NULL;
    }
}

extern CgError
cgEmitLabel(char *label) {
    struct LabelAddress *la = lookupLabel(label);
    if (la) {
        if (!la->undefined) return CG_SECOND_DEFINITION;
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
        CgError e = addLabel((struct LabelAddress) {
            .label     = label,
            .address   = MIN_ADDRESS + instructionPointer,
            .undefined = false,
        });
        if (CG_OK != e) return e;
    }
    return CG_OK;
}

static CgError
emit(uint_fast16_t instruction) {
    if (BUFFER_SIZE <= instructionPointer + 1) return CG_TOO_MANY_INSTRUCTIONS;
    machineCode[instructionPointer++] = (instruction >> 8u) & 0xff;
    machineCode[instructionPointer++] = instruction & 0xff;
    return CG_OK;
}

#define CHECK_ARGUMENT_SIZE(argument, size) \
    if ((uint_fast16_t)(1 << (size)) < (argument)) return CG_TOO_BIG_ARGUMENT;

static CgError
emitHnnni(uint_fast16_t h, uint_fast16_t nnn) {
    CHECK_ARGUMENT_SIZE(h, 4);
    CHECK_ARGUMENT_SIZE(nnn, 12);
    return emit(h << 12 | nnn);
}

static CgError
emitHnnnl(uint_fast16_t h, char *label) {
    struct LabelAddress *la = lookupLabel(label);
    if (!la) { /* If label was neither defined nor used before. */
        /* Save instruction pointer in the table of labels to fill address when
         * it will be defined. */
        CgError e = addLabel((struct LabelAddress) {
            .label      = label,
            .address    = instructionPointer,
            .undefined  = true,
        });
        if (CG_OK != e) return e;
        e = emitHnnni(h, 0);
        ++numberOfUndefinedLabels;
        return e;
    } else if (la->undefined) { /* If label was not defined but was used before. */
        uint_fast16_t previousUsage = la->address;
        /* Update table of labels with new last usage site. */
        la->address = instructionPointer;
        return emitHnnni(h, previousUsage);
    } else { /* If label was defined. */
        return emitHnnni(h, la->address);
    }
}

static CgError
emitHxkk(uint_fast16_t h, uint_fast16_t x, uint_fast16_t kk) {
    CHECK_ARGUMENT_SIZE(h, 4);
    CHECK_ARGUMENT_SIZE(x, 4);
    CHECK_ARGUMENT_SIZE(kk, 8);
    return emit(h << 12 | x << 8 | kk);
}

static CgError
emitHxyn(uint_fast16_t h, uint_fast16_t x, uint_fast16_t y, uint_fast16_t n) {
    CHECK_ARGUMENT_SIZE(h, 4);
    CHECK_ARGUMENT_SIZE(x, 4);
    CHECK_ARGUMENT_SIZE(y, 4);
    CHECK_ARGUMENT_SIZE(n, 4);
    return emit(h << 12 | x << 8 | y << 4 | n);
}

extern CgError
cgEmitData(uint_fast16_t data) {
    if (BUFFER_SIZE < instructionPointer) return CG_TOO_MANY_INSTRUCTIONS;
    machineCode[instructionPointer++] = data & 0xff;
    return CG_OK;
}

extern CgError
cgEmitCls(void) {
    return emit(0x00e0);
}

extern CgError
cgEmitRet(void) {
    return emit(0x00ee);
}

extern CgError
cgEmitJpAddri(uint_fast16_t addr) {
    return emitHnnni(0x1, addr);
}

extern CgError
cgEmitJpAddrl(char *label) {
    return emitHnnnl(0x1, label);
}

extern CgError
cgEmitCallAddri(uint_fast16_t addr) {
    return emitHnnni(0x2, addr);
}

extern CgError
cgEmitCallAddrl(char *label) {
    return emitHnnnl(0x2, label);
}

extern CgError
cgEmitSeVxByte(uint_fast16_t x, uint_fast16_t byte) {
    return emitHxkk(0x3, x, byte);
}

extern CgError
cgEmitSneVxByte(uint_fast16_t x, uint_fast16_t byte) {
    return emitHxkk(0x4, x, byte);
}

extern CgError
cgEmitSeVxVy(uint_fast16_t x, uint_fast16_t y) {
    return emitHxyn(0x5, x, y, 0);
}

extern CgError
cgEmitLdVxByte(uint_fast16_t x, uint_fast16_t byte) {
    return emitHxkk(0x6, x, byte);
}

extern CgError
cgEmitAddVxByte(uint_fast16_t x, uint_fast16_t byte) {
    return emitHxkk(0x7, x, byte);
}

extern CgError
cgEmitLdVxVy(uint_fast16_t x, uint_fast16_t y) {
    return emitHxyn(0x8, x, y, 0);
}

extern CgError
cgEmitOrVxVy(uint_fast16_t x, uint_fast16_t y) {
    return emitHxyn(0x8, x, y, 1);
}

extern CgError
cgEmitAndVxVy(uint_fast16_t x, uint_fast16_t y) {
    return emitHxyn(0x8, x, y, 2);
}

extern CgError
cgEmitXorVxVy(uint_fast16_t x, uint_fast16_t y) {
    return emitHxyn(0x8, x, y, 3);
}

extern CgError
cgEmitAddVxVy(uint_fast16_t x, uint_fast16_t y) {
    return emitHxyn(0x8, x, y, 4);
}

extern CgError
cgEmitSubVxVy(uint_fast16_t x, uint_fast16_t y) {
    return emitHxyn(0x8, x, y, 5);
}

extern CgError
cgEmitShrVx(uint_fast16_t x) {
    return emitHxkk(0x8, x, 0x06);
}

extern CgError
cgEmitSubnVxVy(uint_fast16_t x, uint_fast16_t y) {
    return emitHxyn(0x8, x, y, 7);
}

extern CgError
cgEmitShlVx(uint_fast16_t x) {
    return emitHxkk(0x8, x, 0x0E);
}

extern CgError
cgEmitSneVxVy(uint_fast16_t x, uint_fast16_t y) {
    return emitHxyn(0x9, x, y, 0);
}

extern CgError
cgEmitLdIAddri(uint_fast16_t addr) {
    return emitHnnni(0xA, addr);
}

extern CgError
cgEmitLdIAddrl(char *label) {
    return emitHnnnl(0xA, label);
}

extern CgError
cgEmitJpV0Addri(uint_fast16_t addr) {
    return emitHnnni(0xB, addr);
}

extern CgError
cgEmitJpV0Addrl(char *label) {
    return emitHnnnl(0xB, label);
}

extern CgError
cgEmitRndVxByte(uint_fast16_t x, uint_fast16_t byte) {
    return emitHxkk(0xC, x, byte);
}

extern CgError
cgEmitDrwVxVyNibble(uint_fast16_t x, uint_fast16_t y, uint_fast16_t nibble) {
    return emitHxyn(0xD, x, y, nibble);
}

extern CgError
cgEmitSkpVx(uint_fast16_t x) {
    return emitHxkk(0xE, x, 0x9E);
}

extern CgError
cgEmitSknpVx(uint_fast16_t x) {
    return emitHxkk(0xE, x, 0xA1);
}

extern CgError
cgEmitLdVxDt(uint_fast16_t x) {
    return emitHxkk(0xF, x, 0x07);
}

extern CgError
cgEmitLdVxK(uint_fast16_t x) {
    return emitHxkk(0xF, x, 0x0A);
}

extern CgError
cgEmitLdDtVx(uint_fast16_t x) {
    return emitHxkk(0xF, x, 0x15);
}

extern CgError
cgEmitLdStVx(uint_fast16_t x) {
    return emitHxkk(0xF, x, 0x18);
}

extern CgError
cgEmitAddIVx(uint_fast16_t x) {
    return emitHxkk(0xF, x, 0x1E);
}

extern CgError
cgEmitLdFVx(uint_fast16_t x) {
    return emitHxkk(0xF, x, 0x29);
}

extern CgError
cgEmitLdBVx(uint_fast16_t x) {
    return emitHxkk(0xF, x, 0x33);
}

extern CgError
cgEmitLdIIVx(uint_fast16_t x) {
    return emitHxkk(0xF, x, 0x55);
}

extern CgError
cgEmitLdVxII(uint_fast16_t x) {
    return emitHxkk(0xF, x, 0x65);
}
