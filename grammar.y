
%include {
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

static void
ins(unsigned short w) {
    putchar(w >> 8);
    putchar(w);
}

static void
ins_hnnn(unsigned h, unsigned nnn) {
    ins(h << 12 | nnn);
}

static void
ins_hxkk(unsigned h, unsigned x, unsigned kk) {
    ins(h << 12 | x << 8 | kk);
}

static void
ins_hxyn(unsigned h, unsigned x, unsigned y, unsigned n) {
    ins(h << 12 | x << 8 | y << 4 | n);
}

}

%token_type { int }

unit ::= instructions.

instructions ::= instructions instruction.
instructions ::= .

instruction ::= CLS. { ins(0x00e0); }
instruction ::= RET. { ins(0x00ee); }
instruction ::= SYS INTEGER(addr). { ins_hnnn(0, addr); }
instruction ::= JP INTEGER(addr). { ins_hnnn(1, addr); }
instruction ::= CALL INTEGER(addr). { ins_hnnn(2, addr); }
instruction ::= SE V(x) COMMA INTEGER(byte). { ins_hxkk(3, x, byte); }
instruction ::= SNE V(x) COMMA INTEGER(byte). { ins_hxkk(4, x, byte); }
instruction ::= SE V(x) COMMA V(y). { ins_hxyn(5, x, y, 0); }
instruction ::= LD V(x) COMMA INTEGER(byte). { ins_hxkk(6, x, byte); }
instruction ::= ADD V(x) COMMA INTEGER(byte). { ins_hxkk(7, x, byte); }
instruction ::= LD V(x) COMMA V(y). { ins_hxyn(8, x, y, 0); }
instruction ::= OR V(x) COMMA V(y). { ins_hxyn(8, x, y, 1); }
instruction ::= AND V(x) COMMA V(y). { ins_hxyn(8, x, y, 2); }
instruction ::= XOR V(x) COMMA V(y). { ins_hxyn(8, x, y, 3); }
instruction ::= ADD V(x) COMMA V(y). { ins_hxyn(8, x, y, 4); }
instruction ::= SUB V(x) COMMA V(y). { ins_hxyn(8, x, y, 5); }
instruction ::= SHR V(x). { ins_hxkk(8, x, 0x06); }
instruction ::= SUBN V(x) COMMA V(y). { ins_hxyn(8, x, y, 7); }
instruction ::= SHL V(x). { ins_hxkk(8, x, 0x0E); }
instruction ::= SNE V(x) COMMA V(y). { ins_hxyn(9, x, y, 0); }
instruction ::= LD I COMMA INTEGER(addr). { ins_hnnn(0xA, addr); }
instruction ::= JP V(x) COMMA INTEGER(addr). { if (x) exit(1);
                                               ins_hnnn(0xB, addr); }
instruction ::= RND V(x) COMMA INTEGER(byte). { ins_hxkk(0xC, x, byte); }
instruction ::= DRW V(x) COMMA V(y) COMMA INTEGER(nibble).
                { ins_hxyn(0xD, x, y, nibble); }
instruction ::= SKP V(x). { ins_hxkk(0xE, x, 0x9E); }
instruction ::= SKNP V(x). { ins_hxkk(0xE, x, 0xA1); }
instruction ::= LD V(x) COMMA DT. { ins_hxkk(0xF, x, 0x07); }
instruction ::= LD V(x) COMMA K. { ins_hxkk(0xF, x, 0x0A); }
instruction ::= LD DT COMMA V(x). { ins_hxkk(0xF, x, 0x15); }
instruction ::= LD ST COMMA V(x). { ins_hxkk(0xF, x, 0x18); }
instruction ::= ADD I COMMA V(x). { ins_hxkk(0xF, x, 0x1E); }
instruction ::= LD F COMMA V(x). { ins_hxkk(0xF, x, 0x29); }
instruction ::= LD B COMMA V(x). { ins_hxkk(0xF, x, 0x33); }
instruction ::= LD II COMMA V(x). { ins_hxkk(0xF, x, 0x55); }
instruction ::= LD V(x) COMMA II. { ins_hxkk(0xF, x, 0x65); }

