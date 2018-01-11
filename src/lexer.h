union TokenData {
    uint_fast16_t iValue;
    char *sValue;
};

struct Token {
    int type;
    union TokenData data;
};

extern int lexerNextToken(char **cursor, union TokenData *data);
