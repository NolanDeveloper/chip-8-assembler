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

static uint_fast16_t    line    = 1;
static bool             success = true;
static const char       *outputFilePath;

static const char *codeGeneratorErrorMessages[] = {
    [CG_OK]                     = "OK",
    [CG_TOO_MANY_LABELS]        = "Too many labels.",
    [CG_SECOND_DEFINITION]      = "Labels defined multiple times.",
    [CG_TOO_BIG_ARGUMENT]       = "Argument is too big.",
    [CG_TOO_MANY_INSTRUCTIONS]  = "Too many instructions.",
};

static void
handle(CgError e) {
    if (CG_OK == e) return;
    die("%u: %s", (unsigned)line, codeGeneratorErrorMessages[e]);
}

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

end ::= . {
        if (success) {
            if (cgGetNumberOfUndefinedLabels()) die("There are undefined labels.");
            FILE *f = fopen(outputFilePath, "w");
            if (!f) die("Can't open file: %s", strerror(errno));
            uint_fast8_t *mc = cgGetMachineCode();
            size_t ip = cgGetInstructionPointer();
            size_t n = fwrite(mc, sizeof(mc[0]), ip, f);
            if (n != ip) die("Can't write to file: %s", strerror(errno));
            fclose(f);
        }
    }

unit ::= unit line.
unit ::= line.

line ::= instructionOrLabel new_line.
line ::= error new_line.

new_line ::= NEW_LINE. { ++line; }

instructionOrLabel ::= instruction.
instructionOrLabel ::= LABEL(A) COLON. { handle(cgEmitLabel(A.sValue)); }

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

/* ToDo: produce good error messages when instruction is used with wrong arguments
         specifically with too big arguments. */
instruction ::= DATA integer(A).                        { handle(cgEmitData(A)); }
instruction ::= CLS.                                    { handle(cgEmitCls()); }
instruction ::= RET.                                    { handle(cgEmitRet()); }
instruction ::= JP integer(A).                          { handle(cgEmitJpAddri(A)); }
instruction ::= JP label(A).                            { handle(cgEmitJpAddrl(A)); }
instruction ::= CALL integer(A).                        { handle(cgEmitCallAddri(A)); }
instruction ::= CALL label(A).                          { handle(cgEmitCallAddrl(A)); }
instruction ::= SE v(A) COMMA integer(B).               { handle(cgEmitSeVxByte(A, B));  }
instruction ::= SNE v(A) COMMA integer(B).              { handle(cgEmitSneVxByte(A, B)); }
instruction ::= SE v(A) COMMA v(B).                     { handle(cgEmitSeVxVy(A, B)); }
instruction ::= LD v(A) COMMA integer(B).               { handle(cgEmitLdVxByte(A, B));  }
instruction ::= ADD v(A) COMMA integer(B).              { handle(cgEmitAddVxByte(A, B)); }
instruction ::= LD v(A) COMMA v(B).                     { handle(cgEmitLdVxVy(A, B)); }
instruction ::= OR v(A) COMMA v(B).                     { handle(cgEmitOrVxVy(A, B)); }
instruction ::= AND v(A) COMMA v(B).                    { handle(cgEmitAndVxVy(A, B)); }
instruction ::= XOR v(A) COMMA v(B).                    { handle(cgEmitXorVxVy(A, B)); }
instruction ::= ADD v(A) COMMA v(B).                    { handle(cgEmitAddVxVy(A, B)); }
instruction ::= SUB v(A) COMMA v(B).                    { handle(cgEmitSubVxVy(A, B)); }
instruction ::= SHR v(A).                               { handle(cgEmitShrVx(A)); }
instruction ::= SUBN v(A) COMMA v(B).                   { handle(cgEmitSubnVxVy(A, B)); }
instruction ::= SHL v(A).                               { handle(cgEmitShlVx(A)); }
instruction ::= SNE v(A) COMMA v(B).                    { handle(cgEmitSneVxVy(A, B)); }
instruction ::= LD I COMMA integer(A).                  { handle(cgEmitLdIAddri(A)); }
instruction ::= LD I COMMA label(A).                    { handle(cgEmitLdIAddrl(A)); }
instruction ::= JP V0 COMMA integer(A).                 { handle(cgEmitJpV0Addri(A)); }
instruction ::= JP V0 COMMA label(A).                   { handle(cgEmitJpV0Addrl(A)); }
instruction ::= RND v(A) COMMA integer(B).              { handle(cgEmitRndVxByte(A, B)); }
instruction ::= DRW v(A) COMMA v(B) COMMA integer(C).   { handle(cgEmitDrwVxVyNibble(A, B, C)); }
instruction ::= SKP v(A).                               { handle(cgEmitSkpVx(A)); }
instruction ::= SKNP v(A).                              { handle(cgEmitSknpVx(A)); }
instruction ::= LD v(A) COMMA DT.                       { handle(cgEmitLdVxDt(A)); }
instruction ::= LD v(A) COMMA K.                        { handle(cgEmitLdVxK(A)); }
instruction ::= LD DT COMMA v(A).                       { handle(cgEmitLdDtVx(A)); }
instruction ::= LD ST COMMA v(A).                       { handle(cgEmitLdStVx(A)); }
instruction ::= ADD I COMMA v(A).                       { handle(cgEmitAddIVx(A)); }
instruction ::= LD F COMMA v(A).                        { handle(cgEmitLdFVx(A)); }
instruction ::= LD B COMMA v(A).                        { handle(cgEmitLdBVx(A)); }
instruction ::= LD II COMMA v(A).                       { handle(cgEmitLdIIVx(A)); }
instruction ::= LD v(A) COMMA II.                       { handle(cgEmitLdVxII(A)); }

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
