#include <stdint.h>

#include <uthash.h>

#include "code_generation.h"
#include "utils.h"

#define die(...) die_("codegen", __VA_ARGS__)

static struct LabelAddress {
    char            *label;
    unsigned short  address;
    UT_hash_handle  hh;
} *labels = NULL;

uint_least8_t cgMachineCode[BUFFER_SIZE];
uint_fast16_t cgInstructionPointer = 0;

static struct LabelAddress *forwardLabel[BUFFER_SIZE];

extern void 
cgEmitLabel(char *label) {
    struct LabelAddress *la;
    HASH_FIND_STR(labels, label, la);
    if (la && PLACEHOLDER_ADDRESS == la->address) {
        la->address = MIN_ADDRESS + cgInstructionPointer;
    } else if (!la) {
        la = emalloc(sizeof(struct LabelAddress));
        *la = (struct LabelAddress) {
            .label      = label,
            .address    = MIN_ADDRESS + cgInstructionPointer,
        };
        HASH_ADD_STR(labels, label, la);
    } else {
        die("label(%s) is defined multiple times.", label);
    }
}

extern Instr 
cgGetLabelAddress(char *label) {
    struct LabelAddress *la;
    HASH_FIND_STR(labels, label, la);
    if (!la) {
        la = emalloc(sizeof(struct LabelAddress));
        *la = (struct LabelAddress) {
            .label      = label,
            .address    = PLACEHOLDER_ADDRESS,
        };
        HASH_ADD_STR(labels, label, la);
        forwardLabel[cgInstructionPointer] = la;
    }
    return la->address;
}

extern void 
cgFillForwardLabelAddresses(void) {
    for (uint_fast16_t i = 0; i < cgInstructionPointer; i += 2) {
        struct LabelAddress *la = forwardLabel[i];
        if (NULL == la) continue;
        if (PLACEHOLDER_ADDRESS == la->address) {
            die("Unknown label: %s", la->label);
        }
        /* All you need to understand next lines:
         * 0) Instructions occupy two bytes. 
         * 2) Instructions are stored in big-endian.
         * 1) Address if used occupies 12 least significant bits of instruction. */
        unsigned short address  = la->address;
        cgMachineCode[i    ]      = (cgMachineCode[i] & 0xf0) | ((address >> 8u) & 0x0f);
        cgMachineCode[i + 1]      = address & 0xff;
    }
}

static void
emit(Instr instruction) {
    if (BUFFER_SIZE <= cgInstructionPointer) {
        die("Number of instructions exceeds memory limit.");
    }
    cgMachineCode[cgInstructionPointer++] = (instruction >> 8u) & 0xff;
    cgMachineCode[cgInstructionPointer++] = instruction & 0xff;
}

/* ToDo: add overflow checks and error messages. */

static void
emit_hnnn(Instr h, Instr nnn) {
    emit((h & 0xf) << 12 | (nnn & 0xfff));
}

static void
emit_hxkk(Instr h, Instr x, Instr kk) {
    emit((h & 0xf) << 12 | (x & 0xf) << 8 | (kk & 0xff));
}

static void
emit_hxyn(Instr h, Instr x, Instr y, Instr n) {
    emit((h & 0xf) << 12 | (x & 0xf) << 8 | (y & 0xf) << 4 | (n & 0xf));
}

extern void cgEmitCls(void)                      { emit(0x00e0); }
extern void cgEmitRet(void)                      { emit(0x00ee); }
extern void cgEmitJpAddr(Instr addr)             { emit_hnnn(0x1, addr); }
extern void cgEmitCallAddr(Instr addr)           { emit_hnnn(0x2, addr); }
extern void cgEmitSeVxByte(Instr x, Instr byte)  { emit_hxkk(0x3, x, byte); }
extern void cgEmitSneVxByte(Instr x, Instr byte) { emit_hxkk(0x4, x, byte); }
extern void cgEmitSeVxVy(Instr x, Instr y)       { emit_hxyn(0x5, x, y, 0); }
extern void cgEmitLdVxByte(Instr x, Instr byte)  { emit_hxkk(0x6, x, byte); }
extern void cgEmitAddVxByte(Instr x, Instr byte) { emit_hxkk(0x7, x, byte); }
extern void cgEmitLdVxVy(Instr x, Instr y)       { emit_hxyn(0x8, x, y, 0); }
extern void cgEmitOrVxVy(Instr x, Instr y)       { emit_hxyn(0x8, x, y, 1); }
extern void cgEmitAndVxVy(Instr x, Instr y)      { emit_hxyn(0x8, x, y, 2); }
extern void cgEmitXorVxVy(Instr x, Instr y)      { emit_hxyn(0x8, x, y, 3); }
extern void cgEmitAddVxVy(Instr x, Instr y)      { emit_hxyn(0x8, x, y, 4); }
extern void cgEmitSubVxVy(Instr x, Instr y)      { emit_hxyn(0x8, x, y, 5); }
extern void cgEmitShrVx(Instr x)                 { emit_hxkk(0x8, x, 0x06); }
extern void cgEmitSubnVxVy(Instr x, Instr y)     { emit_hxyn(0x8, x, y, 7); }
extern void cgEmitShlVx(Instr x)                 { emit_hxkk(0x8, x, 0x0E); }
extern void cgEmitSneVxVy(Instr x, Instr y)      { emit_hxyn(0x9, x, y, 0); }
extern void cgEmitLdIAddr(Instr addr)            { emit_hnnn(0xA, addr); }
extern void cgEmitJpV0Addr(Instr addr)           { emit_hnnn(0xB, addr); }
extern void cgEmitRndVxByte(Instr x, Instr byte) { emit_hxkk(0xC, x, byte); }
extern void cgEmitDrwVxVyNibble(Instr x, Instr y, Instr nibble) { emit_hxyn(0xD, x, y, nibble); }
extern void cgEmitSkpVx(Instr x)    { emit_hxkk(0xE, x, 0x9E); }
extern void cgEmitSknpVx(Instr x)   { emit_hxkk(0xE, x, 0xA1); }
extern void cgEmitLdVxDt(Instr x)   { emit_hxkk(0xF, x, 0x07); }
extern void cgEmitLdVxK(Instr x)    { emit_hxkk(0xF, x, 0x0A); }
extern void cgEmitLdDtVx(Instr x)   { emit_hxkk(0xF, x, 0x15); }
extern void cgEmitLdStVx(Instr x)   { emit_hxkk(0xF, x, 0x18); }
extern void cgEmitAddIVx(Instr x)   { emit_hxkk(0xF, x, 0x1E); }
extern void cgEmitLdFVx(Instr x)    { emit_hxkk(0xF, x, 0x29); }
extern void cgEmitLdBVx(Instr x)    { emit_hxkk(0xF, x, 0x33); }
extern void cgEmitLdIIVx(Instr x)   { emit_hxkk(0xF, x, 0x55); }
extern void cgEmitLdVxII(Instr x)   { emit_hxkk(0xF, x, 0x65); }
