#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lex.h"


static FILE *pIF;
static tMorph MorphInit; //alles 0 belegt
static int X; // Eingabezeichen
static int Z; // Aktueller Zustand
static char vBuf[1024+1]; // Buffer zum Sammeln
static char* pBuf; // Pointer in den Buffer
static int line,col; // Zeile und Spalte

static char currLine[1024];
static int currLinePos;



typedef struct stateFunc
{
    int state;
    void (*func)(void);
} stateFunc;


struct keywrd
{
    char* wrdstr;
    enum keywrdtype wrdtype;
};


// Zeichenklassenvektor
static char vZKL[128] = {
/*     0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F */
/*---------------------------------------------------------*/
/* 0*/ 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,/* 0*/
/*10*/ 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,/*10*/
/*20*/ 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 9, 0, 0, 0, 0, 8,/*20*/
/*30*/10, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 0, 5, 4, 6, 0,/*30*/
/*40*/ 0,12,12,12,12,12,12, 2, 2, 2, 2, 2, 2, 2, 2, 2,/*40*/
/*50*/ 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0, 0,/*50*/
/*60*/ 0,12,12,12,12,12,12, 2, 2, 2, 2, 2, 2, 2, 2, 2,/*60*/
/*70*/ 2, 2, 2, 2, 2, 2, 2, 2,11, 2, 2, 0, 0, 0, 0, 7 /*70*/
};

static void fb(void);   // beenden
static void fl(void);   // lesen
static void fsl(void);  // schreiben, lesen
static void fgl(void);  // schreiben als Grossbuchstabe, lesen
static void fslb(void); // schreiben, lesen, beenden
static void frl(void);  // puffer-reset, lesen
                        //
stateFunc vSMatrix[][13] = {
/*          SoZei    Ziffer  Buchstabe     :          =           <           >         sonst      /        *           0         x      a-f, A-F  */
/* Z0 */ {{14,fslb},{1,fsl},  {2,fgl},  {3,fsl},   {14,fslb},  {4,fsl},    {5,fsl},    {0,fl},   {9,fsl}, {14,fslb},{12,fsl}, {2,fgl},  {2,fgl}},
/* Z1 */ {{14,fb},  {1,fsl},  {14,fb},  {14,fb},   {14,fb},    {14,fb},    {14,fb},    {14,fb},  {14,fb}, {14,fb},  {1,fsl},  {14,fb},  {14,fb}},
/* Z2 */ {{14,fb},  {2,fsl},  {2,fgl},  {14,fb},   {14,fb},    {14,fb},    {14,fb},    {14,fb},  {14,fb}, {14,fb},  {2,fsl},  {2,fgl},  {2,fgl}},
/* Z3 */ {{14,fb},  {14,fb},  {14,fb},  {14,fb},   {6,fsl},    {14,fb},    {14,fb},    {14,fb},  {14,fb}, {14,fb},  {14,fb},  {14,fb},  {14,fb}},
/* Z4 */ {{14,fb},  {14,fb},  {14,fb},  {14,fb},   {7,fsl},    {14,fb},    {14,fb},    {14,fb},  {14,fb}, {14,fb},  {14,fb},  {14,fb},  {14,fb}},
/* Z5 */ {{14,fb},  {14,fb},  {14,fb},  {14,fb},   {8,fsl},    {14,fb},    {14,fb},    {14,fb},  {14,fb}, {14,fb},  {14,fb},  {14,fb},  {14,fb}},
/* Z6 */ {{14,fb},  {14,fb},  {14,fb},  {14,fb},   {14,fb},    {14,fb},    {14,fb},    {14,fb},  {14,fb}, {14,fb},  {14,fb},  {14,fb},  {14,fb}},
/* Z7 */ {{14,fb},  {14,fb},  {14,fb},  {14,fb},   {14,fb},    {14,fb},    {14,fb},    {14,fb},  {14,fb}, {14,fb},  {14,fb},  {14,fb},  {14,fb}},
/* Z8 */ {{14,fb},  {14,fb},  {14,fb},  {14,fb},   {14,fb},    {14,fb},    {14,fb},    {14,fb},  {14,fb}, {14,fb},  {14,fb},  {14,fb},  {14,fb}},
/* Z9 */ {{14,fb},  {14,fb},  {14,fb},  {14,fb},   {14,fb},    {14,fb},    {14,fb},    {14,fb},  {14,fb}, {10,fl},  {14,fb},  {14,fb},  {14,fb}},
/* Z10 */{{10,fl},  {10,fl},  {10,fl},  {10,fl},   {10,fl},    {10,fl},    {10,fl},    {10,fl},  {10,fl}, {11,fl},  {10,fl},  {10,fl},  {10,fl}},
/* Z11 */{{10,fl},  {10,fl},  {10,fl},  {10,fl},   {10,fl},    {10,fl},    {10,fl},    {10,fl},  {0,frl}, {11,fl},  {10,fl},  {10,fl},  {10,fl}},
/* Z12 */{{14,fb},  {1,fsl},  {14,fb},  {14,fb},   {14,fb},    {14,fb},    {14,fb},    {14,fb},  {14,fb}, {14,fb},  {1,fsl},  {13,fsl}, {14,fb}},
/* Z13 */{{14,fb},  {13,fsl}, {14,fb},  {14,fb},   {14,fb},    {14,fb},    {14,fb},    {14,fb},  {14,fb}, {14,fb},  {13,fsl}, {14,fb}, {13,fsl}}
};

// Hastable zur Schluesselworterkennung
static struct keywrd ktable['Z'-'A'+1][8] = {
/*Len:      2          3          4               5             6         7         8         9*/
/* A */ {{0L,tNIL},{"ND",tAND},{0L,   tNIL},{0L,       tNIL},{0L,tNIL},{0L,tNIL},{0L,tNIL},{0L,tNIL}},
/* B */ {{0L,tNIL},{0L,  tNIL},{0L,   tNIL},{"EGIN",   tBGN},{0L,tNIL},{0L,tNIL},{0L,tNIL},{0L,tNIL}},
/* C */ {{0L,tNIL},{0L,  tNIL},{"ALL",tCLL},{"ONST",   tCST},{0L,tNIL},{0L,tNIL},{0L,tNIL},{0L,tNIL}},
/* D */ {{"O",tDO},{0L,  tNIL},{0L,   tNIL},{0L,       tNIL},{0L,tNIL},{0L,tNIL},{0L,tNIL},{0L,tNIL}},
/* E */ {{0L,tNIL},{"ND",tEND},{"LSE",tELS},{0L,       tNIL},{0L,tNIL},{0L,tNIL},{0L,tNIL},{0L,tNIL}},
/* F */ {{0L,tNIL},{0L,  tNIL},{0L,   tNIL},{0L,       tNIL},{0L,tNIL},{0L,tNIL},{0L,tNIL},{0L,tNIL}},
/* G */ {{0L,tNIL},{"ET",tGET},{0L,   tNIL},{0L,       tNIL},{0L,tNIL},{0L,tNIL},{0L,tNIL},{0L,tNIL}},
/* H */ {{0L,tNIL},{0L,  tNIL},{0L,   tNIL},{0L,       tNIL},{0L,tNIL},{0L,tNIL},{0L,tNIL},{0L,tNIL}},
/* I */ {{"F",tIF},{0L,  tNIL},{0L,   tNIL},{0L,       tNIL},{0L,tNIL},{0L,tNIL},{0L,tNIL},{0L,tNIL}},
/* J */ {{0L,tNIL},{0L,  tNIL},{0L,   tNIL},{0L,       tNIL},{0L,tNIL},{0L,tNIL},{0L,tNIL},{0L,tNIL}},
/* K */ {{0L,tNIL},{0L,  tNIL},{0L,   tNIL},{0L,       tNIL},{0L,tNIL},{0L,tNIL},{0L,tNIL},{0L,tNIL}},
/* L */ {{0L,tNIL},{0L,  tNIL},{0L,   tNIL},{0L,       tNIL},{0L,tNIL},{0L,tNIL},{0L,tNIL},{0L,tNIL}},
/* M */ {{0L,tNIL},{0L,  tNIL},{0L,   tNIL},{0L,       tNIL},{0L,tNIL},{0L,tNIL},{0L,tNIL},{0L,tNIL}},
/* N */ {{0L,tNIL},{0L,  tNIL},{0L,   tNIL},{0L,       tNIL},{0L,tNIL},{0L,tNIL},{0L,tNIL},{0L,tNIL}},
/* O */ {{"R",tOR},{"DD",tODD},{0L,   tNIL},{0L,       tNIL},{0L,tNIL},{0L,tNIL},{0L,tNIL},{0L,tNIL}},
/* P */ {{0L,tNIL},{"UT",tPUT},{0L,   tNIL},{0L,       tNIL},{0L,tNIL},{0L,tNIL},{0L,tNIL},{"ROCEDURE",tPRC}},
/* Q */ {{0L,tNIL},{0L,  tNIL},{0L,   tNIL},{0L,       tNIL},{0L,tNIL},{0L,tNIL},{0L,tNIL},{0L,tNIL}},
/* R */ {{0L,tNIL},{0L,  tNIL},{0L,   tNIL},{0L,       tNIL},{0L,tNIL},{0L,tNIL},{0L,tNIL},{0L,tNIL}},
/* S */ {{0L,tNIL},{0L,  tNIL},{0L,   tNIL},{0L,       tNIL},{0L,tNIL},{0L,tNIL},{0L,tNIL},{0L,tNIL}},
/* T */ {{0L,tNIL},{0L,  tNIL},{"HEN",tTHN},{0L,       tNIL},{0L,tNIL},{0L,tNIL},{0L,tNIL},{0L,tNIL}},
/* U */ {{0L,tNIL},{0L,  tNIL},{0L,   tNIL},{0L,       tNIL},{0L,tNIL},{0L,tNIL},{0L,tNIL},{0L,tNIL}},
/* V */ {{0L,tNIL},{"AR",tVAR},{0L,   tNIL},{0L,       tNIL},{0L,tNIL},{0L,tNIL},{0L,tNIL},{0L,tNIL}},
/* W */ {{0L,tNIL},{0L,  tNIL},{0L,   tNIL},{"HILE",   tWHL},{0L,tNIL},{0L,tNIL},{0L,tNIL},{0L,tNIL}},
/* X */ {{0L,tNIL},{0L,  tNIL},{0L,   tNIL},{0L,       tNIL},{0L,tNIL},{0L,tNIL},{0L,tNIL},{0L,tNIL}},
/* Y */ {{0L,tNIL},{0L,  tNIL},{0L,   tNIL},{0L,       tNIL},{0L,tNIL},{0L,tNIL},{0L,tNIL},{0L,tNIL}},
/* Z */ {{0L,tNIL},{0L,  tNIL},{0L,   tNIL},{0L,       tNIL},{0L,tNIL},{0L,tNIL},{0L,tNIL},{0L,tNIL}}
};


static void fb(void)
{
    switch(Z) 
    {
        case 3: // :
        case 4: // <
        case 5: // >
        case 9: // /
        case 0: // special chars
            Morph.Val.Symb=vBuf[0];
            Morph.mc=mcSymb;
            break;
        case 1: // Number
        case 12:// 0
            Morph.Val.Num=atol(vBuf);
            Morph.mc=mcNum;
            break;
        case 13: // Hex-Number
            Morph.Val.Num=strtol(vBuf,NULL,16);
            Morph.mc=mcNum;
            break;
        case 2:
            int i=vBuf[0]-'A';
            int j=strlen(vBuf)-2;
            char *wrd;
            // within bounds of HTable
            if (j >= 0 && j <= 7) {
                if ( (wrd=ktable[i][j].wrdstr) && strcmp(vBuf+1,wrd) == 0) {
                    Morph.Val.Symb=ktable[i][j].wrdtype;
                    Morph.mc=mcSymb;
                    break;
                }
            }
            Morph.Val.pStr=vBuf;
            Morph.mc=mcIdent;
            break;
        case 6: // :=
            Morph.Val.Symb=tErg;
            Morph.mc=mcSymb;
            break;
        case 7: // <= 
            Morph.Val.Symb=tle;
            Morph.mc=mcSymb;
            break;
        case 8: // >= 
            Morph.Val.Symb=tge;
            Morph.mc=mcSymb;
            break;
    }
}
static int myfgetc(FILE *f)
{
    if (currLine[currLinePos] == '\0') {
        if (!fgets(currLine,sizeof(currLine),pIF)) {
            return EOF;
        }
        currLinePos=0;
    }
    return currLine[currLinePos++];
}
// lesen
static void fl(void)
{
    X = myfgetc(pIF);
    if (X == '\n') line++, col=0;
    else col++;
}
// schreiben, lesen
static void fsl(void)
{
    *pBuf=(char)X;
    *(++pBuf)=0;
    fl();
}
// schreiben als Grossbuchstabe, lesen
static void fgl(void)
{
    *pBuf=(char)toupper(X); 
    *(++pBuf)=0;
    fl();
}
// schreiben, lesen, beenden
static void fslb(void)
{
    fsl();
    fb();
}
static void frl(void)
{
    pBuf=vBuf;
    pBuf[0]='\0';
    fl();
}

void print_err_line()
{
    printf("%s", currLine);
    printf("%*c^\n",Morph.posCol-1, ' ');
}
int initLex(FILE *f)
{
    pIF=f;
    X=myfgetc(pIF); 

    return OK;
}

tMorph* lex()
{
    Z=0;    //Anfangszustand
    int zx; //Zustand merken
    Morph=MorphInit; //Morphem mit 0 loeschen
    Morph.posLine=line;
    Morph.posCol=col;
    pBuf=vBuf; //Pointer auf Bufferanfang
               
    do {
        zx=vSMatrix[Z][(int)vZKL[(int)X]].state;
        vSMatrix[Z][(int)vZKL[(int)X]].func();
        Z=zx;
    } while(Z != 14);
    
    return &Morph;
}
