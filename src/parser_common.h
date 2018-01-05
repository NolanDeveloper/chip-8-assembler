struct Parse {
    void    (*error)(void);
    void    (*addInstruction)(uint16_t instruction);
    void    (*createLabel)(char *label);
    int     (*getLabelAddress)(char *label);
};
