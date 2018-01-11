#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <limits.h>
#include <string.h>
#include <stdint.h>

#include "parser.h"
#include "lexer.h"
#include "utils.h"

#define die(...) die_("lexer", __VA_ARGS__)

/* string - string started with decDigits and not necessarily ended with zero byte. */
static int
strtoi(const char *string, char **endptr, int base) {
    char *endptr1;
    long value = strtol(string, &endptr1, base);
    if (endptr) *endptr = endptr1;
    assert(endptr1 != string);
    if (INT_MAX < value || value < INT_MIN) {
        int l = endptr1 - string;
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
        data->iValue = strtoi(token, NULL, 10);
        return INTEGER;
    }
    // Hexadecimal literal
    '0x' [0-9a-fA-F]+ {
        data->iValue = strtoi(token + 2, NULL, 16);
        return INTEGER;
    }
    // Binary literal
    '0b' [01]+ {
        data->iValue = strtoi(token + 2, NULL, 2);
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

extern void
lexerPrintToken(FILE *file, int type, union TokenData data) {
    switch (type) {
    case CLS:       fprintf(file, "CLS\n"); break;
    case RET:       fprintf(file, "RET\n"); break;
    case JP:        fprintf(file, "JP\n"); break;
    case INTEGER:   fprintf(file, "INTEGER: %d\n", data.iValue); break;
    case CALL:      fprintf(file, "CALL\n"); break;
    case SE:        fprintf(file, "SE\n"); break;
    case V0:        fprintf(file, "V0\n"); break;
    case V1:        fprintf(file, "V1\n"); break;
    case V2:        fprintf(file, "V2\n"); break;
    case V3:        fprintf(file, "V3\n"); break;
    case V4:        fprintf(file, "V4\n"); break;
    case V5:        fprintf(file, "V5\n"); break;
    case V6:        fprintf(file, "V6\n"); break;
    case V7:        fprintf(file, "V7\n"); break;
    case V8:        fprintf(file, "V8\n"); break;
    case V9:        fprintf(file, "V9\n"); break;
    case VA:        fprintf(file, "VA\n"); break;
    case VB:        fprintf(file, "VB\n"); break;
    case VC:        fprintf(file, "VC\n"); break;
    case VD:        fprintf(file, "VD\n"); break;
    case VE:        fprintf(file, "VE\n"); break;
    case VF:        fprintf(file, "VF\n"); break;
    case COMMA:     fprintf(file, "COMMA\n"); break;
    case SNE:       fprintf(file, "SNE\n"); break;
    case LD:        fprintf(file, "LD\n"); break;
    case ADD:       fprintf(file, "ADD\n"); break;
    case OR:        fprintf(file, "OR\n"); break;
    case AND:       fprintf(file, "AND\n"); break;
    case XOR:       fprintf(file, "XOR\n"); break;
    case SUB:       fprintf(file, "SUB\n"); break;
    case SHR:       fprintf(file, "SHR\n"); break;
    case SUBN:      fprintf(file, "SUBN\n"); break;
    case SHL:       fprintf(file, "SHL\n"); break;
    case I:         fprintf(file, "I\n"); break;
    case RND:       fprintf(file, "RND\n"); break;
    case DRW:       fprintf(file, "DRW\n"); break;
    case SKP:       fprintf(file, "SKP\n"); break;
    case SKNP:      fprintf(file, "SKNP\n"); break;
    case DT:        fprintf(file, "DT\n"); break;
    case K:         fprintf(file, "K\n"); break;
    case ST:        fprintf(file, "ST\n"); break;
    case F:         fprintf(file, "F\n"); break;
    case B:         fprintf(file, "B\n"); break;
    case II:        fprintf(file, "II\n"); break;
    case LABEL:     fprintf(file, "LABEL: %s\n", data.sValue); break;
    case COLON:     fprintf(file, "COLON\n"); break;
    case NEW_LINE:  fprintf(file, "NEW_LINE\n"); break;
    default:        fprintf(file, "unknown %d\n", type); break;
    }
}
