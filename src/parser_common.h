struct Parse {
    void    (*error)(void);
    void    (*addInstruction)(uint16_t instruction);
    void    (*addLabel)(char *label);
    int     (*getLabelAddress)(char *label);
};
