#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <limits.h>
#include <string.h>

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
lexerNextToken(char **cursor, union TokenData *data, int *line, int *column) {
#define RETURN(A) do { *column += *cursor - token; return A; } while(0)
    char *token;
restart:;
    token = *cursor;
    /*!re2c
        re2c:define:YYCTYPE     = char;
        re2c:define:YYCURSOR    = *cursor;
        re2c:yyfill:enable      = 0;
        'CLS'   { RETURN(CLS);  }
        'RET'   { RETURN(RET);  }
        'JP'    { RETURN(JP);   }
        'CALL'  { RETURN(CALL); }
        'SE'    { RETURN(SE);   }
        'SNE'   { RETURN(SNE);  }
        'LD'    { RETURN(LD);   }
        'ADD'   { RETURN(ADD);  }
        'OR'    { RETURN(OR);   }
        'AND'   { RETURN(AND);  }
        'XOR'   { RETURN(XOR);  }
        'SUB'   { RETURN(SUB);  }
        'SHR'   { RETURN(SHR);  }
        'SUBN'  { RETURN(SUBN); }
        'SHL'   { RETURN(SHL);  }
        'RND'   { RETURN(RND);  }
        'DRW'   { RETURN(DRW);  }
        'SKP'   { RETURN(SKP);  }
        'SKNP'  { RETURN(SKNP); }
        ","     { RETURN(COMMA); }
        ":"     { RETURN(COLON); }
        'I'     { RETURN(I);  }
        'DT'    { RETURN(DT); }
        'K'     { RETURN(K);  }
        'ST'    { RETURN(ST); }
        'F'     { RETURN(F);  }
        'B'     { RETURN(B);  }
        '[I]'   { RETURN(II); }
        [0-9]+  { data->iValue = strtoi(token, NULL, 10); 
                  RETURN(INTEGER); }
        'V0'    { RETURN(V0); }
        'V1'    { RETURN(V1); }
        'V2'    { RETURN(V2); }
        'V3'    { RETURN(V3); }
        'V4'    { RETURN(V4); }
        'V5'    { RETURN(V5); }
        'V6'    { RETURN(V6); }
        'V7'    { RETURN(V7); }
        'V8'    { RETURN(V8); }
        'V9'    { RETURN(V9); }
        'VA'    { RETURN(VA); }
        'VB'    { RETURN(VB); }
        'VC'    { RETURN(VC); }
        'VD'    { RETURN(VD); }
        'VE'    { RETURN(VE); }
        'VF'    { RETURN(VF); }
        "." [_a-zA-Z0-9]+ {
            size_t length = *cursor - (token + 1);
            data->sValue = emalloc(length + 1);
            memcpy(data->sValue, token + 1, length);
            data->sValue[length] = '\0';
            RETURN(LABEL);
        }
        [ \t]+ { goto restart; }
        "\n" { 
            ++*line;
            *column = 0;
            goto restart;
        }
        "\000" { return 0; }
    */
    die("Unknown token: %.20s", token);
}

extern void
lexerPrintToken(FILE *file, int type, union TokenData data, int line, int column) {
    fprintf(file, "%3.1d:%-3d\t", line, column);
    switch (type) {
    case CLS:     fprintf(file, "CLS\n");  break;
    case RET:     fprintf(file, "RET\n");  break;
    case JP:      fprintf(file, "JP\n");   break;
    case INTEGER: fprintf(file, "INTEGER: %d\n", data.iValue); break;
    case CALL:    fprintf(file, "CALL\n"); break;
    case SE:      fprintf(file, "SE\n"); break;
    case V0:      fprintf(file, "V0\n"); break;
    case V1:      fprintf(file, "V1\n"); break;        
    case V2:      fprintf(file, "V2\n"); break;              
    case V3:      fprintf(file, "V3\n"); break;              
    case V4:      fprintf(file, "V4\n"); break;              
    case V5:      fprintf(file, "V5\n"); break;              
    case V6:      fprintf(file, "V6\n"); break;              
    case V7:      fprintf(file, "V7\n"); break;              
    case V8:      fprintf(file, "V8\n"); break;              
    case V9:      fprintf(file, "V9\n"); break;              
    case VA:      fprintf(file, "VA\n"); break;              
    case VB:      fprintf(file, "VB\n"); break;              
    case VC:      fprintf(file, "VC\n"); break;              
    case VD:      fprintf(file, "VD\n"); break;              
    case VE:      fprintf(file, "VE\n"); break;              
    case VF:      fprintf(file, "VF\n"); break;              
    case COMMA:   fprintf(file, "COMMA\n"); break;
    case SNE:     fprintf(file, "SNE\n");   break;
    case LD:      fprintf(file, "LD\n");    break;
    case ADD:     fprintf(file, "ADD\n");   break;
    case OR:      fprintf(file, "OR\n");    break;
    case AND:     fprintf(file, "AND\n");   break;
    case XOR:     fprintf(file, "XOR\n");   break;
    case SUB:     fprintf(file, "SUB\n");   break;
    case SHR:     fprintf(file, "SHR\n");   break;
    case SUBN:    fprintf(file, "SUBN\n");  break;
    case SHL:     fprintf(file, "SHL\n");   break;
    case I:       fprintf(file, "I\n");     break;
    case RND:     fprintf(file, "RND\n");   break;
    case DRW:     fprintf(file, "DRW\n");   break;
    case SKP:     fprintf(file, "SKP\n");   break;
    case SKNP:    fprintf(file, "SKNP\n");  break;
    case DT:      fprintf(file, "DT\n");    break;
    case K:       fprintf(file, "K\n");     break;
    case ST:      fprintf(file, "ST\n");    break;
    case F:       fprintf(file, "F\n");     break;
    case B:       fprintf(file, "B\n");     break;
    case II:      fprintf(file, "II\n");    break;
    case LABEL:   fprintf(file, "LABEL: %s\n", data.sValue); break;
    case COLON:   fprintf(file, "COLON\n"); break;
    default:      fprintf(file, "unknown %d\n", type); break;
    }
}
