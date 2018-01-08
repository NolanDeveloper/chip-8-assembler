
typedef uint_fast16_t Instr; 

#define MIN_ADDRESS             0x200
#define MAX_ADDRESS             0x1000
#define BUFFER_SIZE             (MAX_ADDRESS - MIN_ADDRESS)
#define PLACEHOLDER_ADDRESS     MAX_ADDRESS

extern uint_least8_t cgMachineCode[BUFFER_SIZE];
extern uint_fast16_t cgInstructionPointer;

extern void cgEmitLabel(char *label);
extern Instr cgGetLabelAddress(char *label);

extern void cgFillForwardLabelAddresses(void);

extern void cgEmitCls(void);
extern void cgEmitRet(void);
extern void cgEmitJpAddr(Instr addr);
extern void cgEmitCallAddr(Instr addr);
extern void cgEmitSeVxByte(Instr x, Instr byte);
extern void cgEmitSneVxByte(Instr x, Instr byte);
extern void cgEmitSeVxVy(Instr x, Instr y);
extern void cgEmitLdVxByte(Instr x, Instr byte);
extern void cgEmitAddVxByte(Instr x, Instr byte);
extern void cgEmitLdVxVy(Instr x, Instr y);
extern void cgEmitOrVxVy(Instr x, Instr y);
extern void cgEmitAndVxVy(Instr x, Instr y);
extern void cgEmitXorVxVy(Instr x, Instr y);
extern void cgEmitAddVxVy(Instr x, Instr y);
extern void cgEmitSubVxVy(Instr x, Instr y);
extern void cgEmitShrVx(Instr x);
extern void cgEmitSubnVxVy(Instr x, Instr y);
extern void cgEmitShlVx(Instr x);
extern void cgEmitSneVxVy(Instr x, Instr y);
extern void cgEmitLdIAddr(Instr addr);
extern void cgEmitJpV0Addr(Instr addr);
extern void cgEmitRndVxByte(Instr x, Instr byte);
extern void cgEmitDrwVxVyNibble(Instr x, Instr y, Instr nibble);
extern void cgEmitSkpVx(Instr x);
extern void cgEmitSknpVx(Instr x);
extern void cgEmitLdVxDt(Instr x);
extern void cgEmitLdVxK(Instr x);
extern void cgEmitLdDtVx(Instr x);
extern void cgEmitLdStVx(Instr x);
extern void cgEmitAddIVx(Instr x);
extern void cgEmitLdFVx(Instr x);
extern void cgEmitLdBVx(Instr x);
extern void cgEmitLdIIVx(Instr x);
extern void cgEmitLdVxII(Instr x);
