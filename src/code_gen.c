#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "lex.h"
#include "code_gen.h"
#include "parse.h"

#define MAX_LEN_OF_CODE 7

char *cbuf_curr;
char *cbuf_start;
unsigned int cbuf_size;

// write to current PC
static void write_code(short x)
{
    // Byteorder is little Endian (intel)
    *cbuf_curr++=(unsigned char)(x&0xff);
    *cbuf_curr++=(unsigned char)(x>>8);
}
// write to given position without incrementing the PC
void write_code_at(short x, char *cbuf_curr)
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
    //write op-code
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

int write_code2file(int curr_var_offset)
{
  unsigned short len=(unsigned short)(cbuf_curr-cbuf_start);
  write_code_at((short)len,cbuf_start+1);
  write_code_at((short)curr_var_offset,cbuf_start+5);
  if (len==fwrite(cbuf_start,sizeof(char),len,pOF))
      return OK;
  else
      return FAIL;
}
int write_consts2file(int *cnst_buf, int cnst_buf_idx)
{
    int len=cnst_buf_idx*sizeof(int);
    if (len==fwrite(cnst_buf,sizeof(int),cnst_buf_idx,pOF))
        return OK;
    else
        return FAIL;
}
int write_num_proc2file(int num_proc)
{
    int len=sizeof(num_proc);
    rewind(pOF);
    if (len==fwrite(&num_proc,len,1,pOF))
        return OK;
    else
        return FAIL;
}
