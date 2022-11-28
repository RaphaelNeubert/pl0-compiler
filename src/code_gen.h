
#define MAX_LEN_OF_CODE 7

enum tCode {
    puValVrLocl, puValVrMain, puValVrGlob,
    puAdrVrLocl, puAdrVrMain, puAdrVrGlob,
    puConst, storeVal, putVal, getVal,
    vzMinus, odd, 
    OpAdd, OpSub, OpMult, OpDiv,
    cmpEQ, cmpNE, cmpLT, cmpGT, cmpLE, cmpGE,
    call, retProc, jmp, jnot, entryProc, putStrg, EndOfCode
};

int generate_code(enum tCode code, ...);
