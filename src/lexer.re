#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <limits.h>
#include <string.h>
#include <stdint.h>

#include "parser.h"
#include "lexer.h"
#include "utils.h"

/* string - string started with decDigits and not necessarily ended with zero byte. */
static uint_fast16_t
strtoi(const char *string, int base) {
    char *endptr;
    unsigned long value = strtoul(string, &endptr, base);
    assert(endptr != string); /* lexer must not use this function on bad strings */
    if (value > UINT_FAST16_MAX) {
        int l = endptr - string;
        if (l < 20) {
            die("%.*s is too big number", l, string);
        } else {
            die("%.20s... is too big number", string);
        }
    }
    return (int)value;
}

extern int
lexerNextToken(char **cursor, union TokenData *data) {
    char *token, *marker;
restart:;
    token = *cursor;
    /*!re2c
    re2c:define:YYCTYPE     = char;
    re2c:define:YYCURSOR    = *cursor;
    re2c:define:YYMARKER    = marker;
    re2c:yyfill:enable      = 0;
    *       { die("Unknown token: \"%.5s...\"", token); }
    "\x00"  { return 0; }
    'DATA'  { return DATA; }
    'CLS'   { return CLS;  }
    'RET'   { return RET;  }
    'JP'    { return JP;   }
    'CALL'  { return CALL; }
    'SE'    { return SE;   }
    'SNE'   { return SNE;  }
    'LD'    { return LD;   }
    'ADD'   { return ADD;  }
    'OR'    { return OR;   }
    'AND'   { return AND;  }
    'XOR'   { return XOR;  }
    'SUB'   { return SUB;  }
    'SHR'   { return SHR;  }
    'SUBN'  { return SUBN; }
    'SHL'   { return SHL;  }
    'RND'   { return RND;  }
    'DRW'   { return DRW;  }
    'SKP'   { return SKP;  }
    'SKNP'  { return SKNP; }
    ","     { return COMMA; }
    ":"     { return COLON; }
    'I'     { return I;  }
    'DT'    { return DT; }
    'K'     { return K;  }
    'ST'    { return ST; }
    'F'     { return F;  }
    'B'     { return B;  }
    '[I]'   { return II; }
    'V0'    { return V0; }
    'V1'    { return V1; }
    'V2'    { return V2; }
    'V3'    { return V3; }
    'V4'    { return V4; }
    'V5'    { return V5; }
    'V6'    { return V6; }
    'V7'    { return V7; }
    'V8'    { return V8; }
    'V9'    { return V9; }
    'VA'    { return VA; }
    'VB'    { return VB; }
    'VC'    { return VC; }
    'VD'    { return VD; }
    'VE'    { return VE; }
    'VF'    { return VF; }
    "\n"    { return NEW_LINE; }
    // Decimal literal
    [0-9]+ {
        data->iValue = strtoi(token, 10);
        return INTEGER;
    }
    // Hexadecimal literal
    '0x' [0-9a-fA-F]+ {
        data->iValue = strtoi(token + 2, 16);
        return INTEGER;
    }
    // Binary literal
    '0b' [01]+ {
        data->iValue = strtoi(token + 2, 2);
        return INTEGER;
    }
    // Label
    "." [_a-zA-Z0-9]+ {
        size_t length = *cursor - (token + 1);
        data->sValue = emalloc(length + 1);
        memcpy(data->sValue, token + 1, length);
        data->sValue[length] = '\0';
        return LABEL;
    }
    [ \t]+ { goto restart; }
    */
}
