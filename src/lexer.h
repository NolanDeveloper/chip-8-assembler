union TokenData {
    int iValue;
    char *sValue;
};

struct Token {
    int type;
    union TokenData data;
};

extern int lexerNextToken(char **cursor, union TokenData *data, int *line, int *column);
extern void lexerPrintToken(FILE *file, int type, union TokenData data, int line, int column);
