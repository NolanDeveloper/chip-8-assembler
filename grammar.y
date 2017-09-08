
%include {
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

static void
word(unsigned short w) {
    putchar(w >> 8);
    putchar(w);
}

static void
hnnn(unsigned h, unsigned nnn) {
    word(h << 12 | nnn);
}

static void
hxkk(unsigned h, unsigned x, unsigned kk) {
    word(h << 12 | x << 8 | kk);
}

static void
hxyn(unsigned h, unsigned x, unsigned y, unsigned n) {
    word(h << 12 | x << 8 | y << 4 | n);
}

}

%token_type { int }

unit ::= instrs.

instrs ::= instrs instr.
instrs ::= instr.

instr ::= CLS. { word(0x00e0); }
instr ::= RET. { word(0x00ee); }
instr ::= JP INTEGER(addr). { hnnn(1, addr); }
instr ::= CALL INTEGER(addr). { hnnn(2, addr); }
instr ::= SE V(x) COMMA INTEGER(byte). { hxkk(3, x, byte); }
instr ::= SNE V(x) COMMA INTEGER(byte). { hxkk(4, x, byte); }
instr ::= SE V(x) COMMA V(y). { hxyn(5, x, y, 0); }
instr ::= LD V(x) COMMA INTEGER(byte). { hxkk(6, x, byte); }
instr ::= ADD V(x) COMMA INTEGER(byte). { hxkk(7, x, byte); }
instr ::= LD V(x) COMMA V(y). { hxyn(8, x, y, 0); }
instr ::= OR V(x) COMMA V(y). { hxyn(8, x, y, 1); }
instr ::= AND V(x) COMMA V(y). { hxyn(8, x, y, 2); }
instr ::= XOR V(x) COMMA V(y). { hxyn(8, x, y, 3); }
instr ::= ADD V(x) COMMA V(y). { hxyn(8, x, y, 4); }
instr ::= SUB V(x) COMMA V(y). { hxyn(8, x, y, 5); }
instr ::= SHR V(x). { hxkk(8, x, 0x06); }
instr ::= SUBN V(x) COMMA V(y). { hxyn(8, x, y, 7); }
instr ::= SHL V(x). { hxkk(8, x, 0x0E); }
instr ::= SNE V(x) COMMA V(y). { hxyn(9, x, y, 0); }
instr ::= LD I COMMA INTEGER(addr). { hnnn(0xA, addr); }
instr ::= JP V(x) COMMA INTEGER(addr). { x ? exit(1) : hnnn(0xB, addr); }
instr ::= RND V(x) COMMA INTEGER(byte). { hxkk(0xC, x, byte); }
instr ::= DRW V(x) COMMA V(y) COMMA INTEGER(nibble). { hxyn(0xD, x, y, nibble); }
instr ::= SKP V(x). { hxkk(0xE, x, 0x9E); }
instr ::= SKNP V(x). { hxkk(0xE, x, 0xA1); }
instr ::= LD V(x) COMMA DT. { hxkk(0xF, x, 0x07); }
instr ::= LD V(x) COMMA K. { hxkk(0xF, x, 0x0A); }
instr ::= LD DT COMMA V(x). { hxkk(0xF, x, 0x15); }
instr ::= LD ST COMMA V(x). { hxkk(0xF, x, 0x18); }
instr ::= ADD I COMMA V(x). { hxkk(0xF, x, 0x1E); }
instr ::= LD F COMMA V(x). { hxkk(0xF, x, 0x29); }
instr ::= LD B COMMA V(x). { hxkk(0xF, x, 0x33); }
instr ::= LD II COMMA V(x). { hxkk(0xF, x, 0x55); }
instr ::= LD V(x) COMMA II. { hxkk(0xF, x, 0x65); }

