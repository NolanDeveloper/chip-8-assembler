%include {

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include "lexer.h"
#include "utils.h"
#include "code_generation.h"

#define die(...) die_("parser", __VA_ARGS__)

static int line   = 1;
static int column = 1;

} /* end %include */

%token_type         { union TokenData }

%type address       { int }
%type v             { int }
%type instruction   { unsigned short }

%syntax_error       { die("Error at %d:%d", line, column); }

start ::= unit.

unit ::= unit instructionOrLabel.
unit ::= instructionOrLabel.

instructionOrLabel ::= instruction.
instructionOrLabel ::= LABEL(A) COLON. { cgEmitLabel(A.sValue); }

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

instruction ::= CLS.                        { cgEmitCls(); }
instruction ::= RET.                        { cgEmitRet(); }
instruction ::= JP INTEGER(A).              { cgEmitJpAddri(A.iValue); }
instruction ::= JP LABEL(A).                { cgEmitJpAddrl(A.sValue); }
instruction ::= CALL INTEGER(A).            { cgEmitCallAddri(A.iValue); }
instruction ::= CALL LABEL(A).              { cgEmitCallAddrl(A.sValue); }
instruction ::= SE v(A) COMMA INTEGER(B).   { cgEmitSeVxByte(A, B.iValue);  }
instruction ::= SNE v(A) COMMA INTEGER(B).  { cgEmitSneVxByte(A, B.iValue); }
instruction ::= SE v(A) COMMA v(B).         { cgEmitSeVxVy(A, B); }
instruction ::= LD v(A) COMMA INTEGER(B).   { cgEmitLdVxByte(A, B.iValue);  }
instruction ::= ADD v(A) COMMA INTEGER(B).  { cgEmitAddVxByte(A, B.iValue); }
instruction ::= LD v(A) COMMA v(B).         { cgEmitLdVxVy(A, B); }
instruction ::= OR v(A) COMMA v(B).         { cgEmitOrVxVy(A, B); }
instruction ::= AND v(A) COMMA v(B).        { cgEmitAndVxVy(A, B); }
instruction ::= XOR v(A) COMMA v(B).        { cgEmitXorVxVy(A, B); }
instruction ::= ADD v(A) COMMA v(B).        { cgEmitAddVxVy(A, B); }
instruction ::= SUB v(A) COMMA v(B).        { cgEmitSubVxVy(A, B); }
instruction ::= SHR v(A).                   { cgEmitShrVx(A); }
instruction ::= SUBN v(A) COMMA v(B).       { cgEmitSubnVxVy(A, B); }
instruction ::= SHL v(A).                   { cgEmitShlVx(A); }
instruction ::= SNE v(A) COMMA v(B).        { cgEmitSneVxVy(A, B); }
instruction ::= LD I COMMA INTEGER(A).      { cgEmitLdIAddri(A.iValue); }
instruction ::= LD I COMMA LABEL(A).        { cgEmitLdIAddrl(A.sValue); }
instruction ::= JP V0 COMMA INTEGER(A).     { cgEmitJpV0Addri(A.iValue); }
instruction ::= JP V0 COMMA LABEL(A).       { cgEmitJpV0Addrl(A.sValue); }
instruction ::= RND v(A) COMMA INTEGER(B).  { cgEmitRndVxByte(A, B.iValue); }
instruction ::= DRW v(A) COMMA v(B) COMMA INTEGER(C). { cgEmitDrwVxVyNibble(A, B, C.iValue); }
instruction ::= SKP v(A).                   { cgEmitSkpVx(A); }
instruction ::= SKNP v(A).                  { cgEmitSknpVx(A); }
instruction ::= LD v(A) COMMA DT.           { cgEmitLdVxDt(A); }
instruction ::= LD v(A) COMMA K.            { cgEmitLdVxK(A); }
instruction ::= LD DT COMMA v(A).           { cgEmitLdDtVx(A); }
instruction ::= LD ST COMMA v(A).           { cgEmitLdStVx(A); }
instruction ::= ADD I COMMA v(A).           { cgEmitAddIVx(A); }
instruction ::= LD F COMMA v(A).            { cgEmitLdFVx(A); }
instruction ::= LD B COMMA v(A).            { cgEmitLdBVx(A); }
instruction ::= LD II COMMA v(A).           { cgEmitLdIIVx(A); }
instruction ::= LD v(A) COMMA II.           { cgEmitLdVxII(A); }

%code {

int main(int argc, char *argv[]) {
    char *cursor;
    struct Token token;
    if (argc != 3) {
        fprintf(stderr, "usage: %s InputFile.s OutputFile.rom\n", argv[0]);
        exit(1);
    }
    const char * inputFilePath = argv[1];
    const char * outputFilePath = argv[2];
    cursor = loadFile(inputFilePath);
    cgInit();
    void *parser = ParseAlloc(emalloc);
    while ((token.type = lexerNextToken(&cursor, &token.data, &line, &column))) {
        Parse(parser, token.type, token.data);
    }
    Parse(parser, 0, token.data);
    cgSaveMachineCodeToFile(outputFilePath);
}

} /* end %code */
