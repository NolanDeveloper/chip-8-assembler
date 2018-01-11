%include {

#include <inttypes.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <stdbool.h>

#include "lexer.h"
#include "utils.h"
#include "code_generation.h"

#define die(...) die_("parser", __VA_ARGS__)

static uint_fast32_t line = 1;
static bool          success = true;

static const char *outputFilePath;

} /* end %include */

%token_type         { union TokenData }

%type v             { uint_fast16_t }
%type integer       { uint_fast16_t }
%type label         { char* }

%syntax_error {
    success = false;
    fprintf(stderr, "parser: Syntax error at line %"PRIuFAST16".\n", line);
}

start ::= begin unit end.

begin ::= . { cgInit(); }

end ::= . { if (success) cgSaveMachineCodeToFile(outputFilePath); }

unit ::= unit line.
unit ::= line.

line ::= instructionOrLabel new_line.
line ::= error new_line.

new_line ::= NEW_LINE. { ++line; }

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

integer(A) ::= INTEGER(B). { A = B.iValue; }

label(A) ::= LABEL(B). { A = B.sValue; }

instruction ::= DATA integer(A).            { cgEmitData(A); }
instruction ::= CLS.                        { cgEmitCls(); }
instruction ::= RET.                        { cgEmitRet(); }
instruction ::= JP integer(A).              { cgEmitJpAddri(A); }
instruction ::= JP label(A).                { cgEmitJpAddrl(A); }
instruction ::= CALL integer(A).            { cgEmitCallAddri(A); }
instruction ::= CALL label(A).              { cgEmitCallAddrl(A); }
instruction ::= SE v(A) COMMA integer(B).   { cgEmitSeVxByte(A, B);  }
instruction ::= SNE v(A) COMMA integer(B).  { cgEmitSneVxByte(A, B); }
instruction ::= SE v(A) COMMA v(B).         { cgEmitSeVxVy(A, B); }
instruction ::= LD v(A) COMMA integer(B).   { cgEmitLdVxByte(A, B);  }
instruction ::= ADD v(A) COMMA integer(B).  { cgEmitAddVxByte(A, B); }
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
instruction ::= LD I COMMA integer(A).      { cgEmitLdIAddri(A); }
instruction ::= LD I COMMA label(A).        { cgEmitLdIAddrl(A); }
instruction ::= JP V0 COMMA integer(A).     { cgEmitJpV0Addri(A); }
instruction ::= JP V0 COMMA label(A).       { cgEmitJpV0Addrl(A); }
instruction ::= RND v(A) COMMA integer(B).  { cgEmitRndVxByte(A, B); }
instruction ::= DRW v(A) COMMA v(B) COMMA integer(C). { cgEmitDrwVxVyNibble(A, B, C); }
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
    if (argc != 3) {
        fprintf(stderr, "usage: %s InputFile.s OutputFile.rom\n", argv[0]);
        exit(1);
    }
    const char *inputFilePath = argv[1];
    outputFilePath = argv[2];
    char *cursor = loadFile(inputFilePath);
    void *parser = ParseAlloc(emalloc);
    struct Token token;
    while ((token.type = lexerNextToken(&cursor, &token.data))) {
        Parse(parser, token.type, token.data);
    }
    Parse(parser, 0, token.data);
}

} /* end %code */
