
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

extern char *cbuf_curr;
extern char *cbuf_start;
extern unsigned int cbuf_size;


int generate_code(enum tCode code, ...);
int write_code2file(int curr_var_offset);
int write_consts2file(short *cnst_buf, short cnst_buf_idx);
int write_num_proc2file(int num_proc);
