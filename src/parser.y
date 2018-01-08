%include {

#include "parser_common.h"
#include "lexer.h"

static unsigned short hnnn(unsigned short h, unsigned short nnn) {
    return (h & 0xf) << 12 | (nnn & 0xfff);
}

static unsigned short
hxkk(unsigned short h, unsigned short x, unsigned short kk) {
    return (h & 0xf) << 12 | (x & 0xf) << 8 | (kk & 0xff);
}

static unsigned short
hxyn(unsigned short h, unsigned short x, unsigned short y, unsigned short n) {
    return (h & 0xf) << 12 | (x & 0xf) << 8 | (y & 0xf) << 4 | (n & 0xf);
}

} /* end %include */

%token_type         { union TokenData }

%type address       { int }
%type v             { int }
%type instruction   { unsigned short }

%syntax_error       { error(); }

start ::= unit.

unit ::= unit instructionOrLabel.
unit ::= instructionOrLabel.

instructionOrLabel ::= instruction(A). { addInstruction(A);  }
instructionOrLabel ::= LABEL(A) COLON. { addLabel(A.sValue); }

v(A) ::= V0. { A = 0x0; }
v(A) ::= V1. { A = 0x1; }
v(A) ::= V2. { A = 0x2; }
v(A) ::= V3. { A = 0x3; }
v(A) ::= V4. { A = 0x4; }
v(A) ::= V5. { A = 0x5; }
v(A) ::= V6. { A = 0x6; }
v(A) ::= V7. { A = 0x7; }
v(A) ::= V8. { A = 0x8; }
v(A) ::= V9. { A = 0x9; }
v(A) ::= VA. { A = 0xA; }
v(A) ::= VB. { A = 0xB; }
v(A) ::= VC. { A = 0xC; }
v(A) ::= VD. { A = 0xD; }
v(A) ::= VE. { A = 0xE; }
v(A) ::= VF. { A = 0xF; }

address(A) ::= INTEGER(B). { A = B.iValue; }
address(A) ::= LABEL(B).   { A = getLabelAddress(B.sValue); }

instruction(A) ::= CLS.                       { A = 0x00e0; }
instruction(A) ::= RET.                       { A = 0x00ee; }
instruction(A) ::= JP address(B).             { A = hnnn(1, B); }
instruction(A) ::= CALL address(B).           { A = hnnn(2, B); }
instruction(A) ::= SE v(B) COMMA INTEGER(C).  { A = hxkk(3, B, C.iValue); }
instruction(A) ::= SNE v(B) COMMA INTEGER(C). { A = hxkk(4, B, C.iValue); }
instruction(A) ::= SE v(B) COMMA v(C).        { A = hxyn(5, B, C, 0); }
instruction(A) ::= LD v(B) COMMA INTEGER(C).  { A = hxkk(6, B, C.iValue); }
instruction(A) ::= ADD v(B) COMMA INTEGER(C). { A = hxkk(7, B, C.iValue); }
instruction(A) ::= LD v(B) COMMA v(C).        { A = hxyn(8, B, C, 0); }
instruction(A) ::= OR v(B) COMMA v(C).        { A = hxyn(8, B, C, 1); }
instruction(A) ::= AND v(B) COMMA v(C).       { A = hxyn(8, B, C, 2); }
instruction(A) ::= XOR v(B) COMMA v(C).       { A = hxyn(8, B, C, 3); }
instruction(A) ::= ADD v(B) COMMA v(C).       { A = hxyn(8, B, C, 4); }
instruction(A) ::= SUB v(B) COMMA v(C).       { A = hxyn(8, B, C, 5); }
instruction(A) ::= SHR v(B).                  { A = hxkk(8, B, 0x06); }
instruction(A) ::= SUBN v(B) COMMA v(C).      { A = hxyn(8, B, C, 7); }
instruction(A) ::= SHL v(B).                  { A = hxkk(8, B, 0x0E); }
instruction(A) ::= SNE v(B) COMMA v(C).       { A = hxyn(9, B, C, 0); }
instruction(A) ::= LD I COMMA address(B).     { A = hnnn(0xA, B); }
instruction(A) ::= JP V0 COMMA address(B).    { A = hnnn(0xB, B); }
instruction(A) ::= RND v(B) COMMA INTEGER(C). { A = hxkk(0xC, B, C.iValue); }
instruction(A) ::= DRW v(B) COMMA v(C) COMMA INTEGER(D). 
                                     { A = hxyn(0xD, B, C, D.iValue); }
instruction(A) ::= SKP v(B).         { A = hxkk(0xE, B, 0x9E); }
instruction(A) ::= SKNP v(B).        { A = hxkk(0xE, B, 0xA1); }
instruction(A) ::= LD v(B) COMMA DT. { A = hxkk(0xF, B, 0x07); }
instruction(A) ::= LD v(B) COMMA K.  { A = hxkk(0xF, B, 0x0A); }
instruction(A) ::= LD DT COMMA v(B). { A = hxkk(0xF, B, 0x15); }
instruction(A) ::= LD ST COMMA v(B). { A = hxkk(0xF, B, 0x18); }
instruction(A) ::= ADD I COMMA v(B). { A = hxkk(0xF, B, 0x1E); }
instruction(A) ::= LD F COMMA v(B).  { A = hxkk(0xF, B, 0x29); }
instruction(A) ::= LD B COMMA v(B).  { A = hxkk(0xF, B, 0x33); }
instruction(A) ::= LD II COMMA v(B). { A = hxkk(0xF, B, 0x55); }
instruction(A) ::= LD v(B) COMMA II. { A = hxkk(0xF, B, 0x65); }
