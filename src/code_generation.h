#define CG_OK                       0
#define CG_TOO_MANY_LABELS          1
#define CG_SECOND_DEFINITION        2
#define CG_TOO_BIG_ARGUMENT         3
#define CG_TOO_MANY_INSTRUCTIONS    4

#define MIN_ADDRESS             0x200
#define MAX_ADDRESS             0x1000
#define MAX_LABELS              2048

extern void cgInit(void);

extern uint_fast16_t cgGetNumberOfUndefinedLabels(void);
extern uint_least8_t *cgGetMachineCode(void);
extern uint_fast16_t cgGetInstructionPointer(void);

typedef uint_fast8_t CgError;

extern CgError cgEmitLabel(char *label);

extern CgError cgSaveMachineCodeToFile(const char *path);

extern CgError cgEmitData(uint_fast16_t data);
extern CgError cgEmitCls(void);
extern CgError cgEmitRet(void);
extern CgError cgEmitJpAddri(uint_fast16_t addr);
extern CgError cgEmitJpAddrl(char *label);
extern CgError cgEmitCallAddri(uint_fast16_t addr);
extern CgError cgEmitCallAddrl(char *label);
extern CgError cgEmitSeVxByte(uint_fast16_t x, uint_fast16_t byte);
extern CgError cgEmitSneVxByte(uint_fast16_t x, uint_fast16_t byte);
extern CgError cgEmitSeVxVy(uint_fast16_t x, uint_fast16_t y);
extern CgError cgEmitLdVxByte(uint_fast16_t x, uint_fast16_t byte);
extern CgError cgEmitAddVxByte(uint_fast16_t x, uint_fast16_t byte);
extern CgError cgEmitLdVxVy(uint_fast16_t x, uint_fast16_t y);
extern CgError cgEmitOrVxVy(uint_fast16_t x, uint_fast16_t y);
extern CgError cgEmitAndVxVy(uint_fast16_t x, uint_fast16_t y);
extern CgError cgEmitXorVxVy(uint_fast16_t x, uint_fast16_t y);
extern CgError cgEmitAddVxVy(uint_fast16_t x, uint_fast16_t y);
extern CgError cgEmitSubVxVy(uint_fast16_t x, uint_fast16_t y);
extern CgError cgEmitShrVx(uint_fast16_t x);
extern CgError cgEmitSubnVxVy(uint_fast16_t x, uint_fast16_t y);
extern CgError cgEmitShlVx(uint_fast16_t x);
extern CgError cgEmitSneVxVy(uint_fast16_t x, uint_fast16_t y);
extern CgError cgEmitLdIAddri(uint_fast16_t addr);
extern CgError cgEmitLdIAddrl(char *label);
extern CgError cgEmitJpV0Addri(uint_fast16_t addr);
extern CgError cgEmitJpV0Addrl(char *label);
extern CgError cgEmitRndVxByte(uint_fast16_t x, uint_fast16_t byte);
extern CgError cgEmitDrwVxVyNibble(uint_fast16_t x, uint_fast16_t y, uint_fast16_t nibble);
extern CgError cgEmitSkpVx(uint_fast16_t x);
extern CgError cgEmitSknpVx(uint_fast16_t x);
extern CgError cgEmitLdVxDt(uint_fast16_t x);
extern CgError cgEmitLdVxK(uint_fast16_t x);
extern CgError cgEmitLdDtVx(uint_fast16_t x);
extern CgError cgEmitLdStVx(uint_fast16_t x);
extern CgError cgEmitAddIVx(uint_fast16_t x);
extern CgError cgEmitLdFVx(uint_fast16_t x);
extern CgError cgEmitLdBVx(uint_fast16_t x);
extern CgError cgEmitLdIIVx(uint_fast16_t x);
extern CgError cgEmitLdVxII(uint_fast16_t x);
