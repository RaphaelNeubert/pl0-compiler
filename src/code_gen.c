#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "lex.h"

#define MAX_LEN_OF_CODE 7

static char *cbuf_curr;
static char *cbuf_start;
static unsigned int cbuf_size;

enum tCode {
    puValVrLocl, puValVrMain, puValVrGlob,
    puAdrVrLocl, puAdrVrMain, puAdrVrGlob,
    puConst, storeVal, putVal, getVal,
    vzMinus, odd, 
    OpAdd, OpSub, OpMult, OpDiv,
    cmpEQ, cmpNE, cmpLT, cmpGT, cmpLE, cmpGE,
    call, retProc, jmp, jnot, entryProc, putStrg, EndOfCode
};
// write to current PC
static void write_code(short x)
{
    *cbuf_curr++=(unsigned char)(x&0xff);
    *cbuf_curr++=(unsigned char)(x>>8);
}
// write to given position without incrementing the PC
static void write_code_at(short x, char *cbuf_curr)
{
    *cbuf_curr=(unsigned char)(x&0xff);
    *(cbuf_curr+1)=(unsigned char)(x>>8);
}

int generate_code(enum tCode code, ...)
{
    va_list ap;
    short arg;
    
    int curr_size=cbuf_curr-cbuf_start;
    if (curr_size+MAX_LEN_OF_CODE >= cbuf_size) {
        cbuf_start=realloc(cbuf_start, (cbuf_size+=1024));
        if (!cbuf_start) {
            perror("failed to realloc memory for code Buffer");
            exit(-1);
        }
        cbuf_curr=cbuf_start+curr_size;
    }
    //write command
    *cbuf_curr++=code;

    va_start(ap,code);
    switch (code) {
        case entryProc:
            arg=va_arg(ap,int);
            write_code(arg);
        case puValVrGlob:
        case puAdrVrGlob:
            arg=va_arg(ap,int);
            write_code(arg);
        case puValVrMain:
        case puAdrVrMain:
        case puValVrLocl:
        case puAdrVrLocl:
        case puConst:
        case jmp:
        case jnot:
        case call:
            arg=va_arg(ap,int);
            write_code(arg);
            break;
        default:
            break;
    }
    va_end(ap);

    return 1; //????
}

