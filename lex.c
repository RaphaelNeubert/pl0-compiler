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


static char vZKL[128] = {
/*     0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F */
/*---------------------------------------------------------*/
/* 0*/ 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,/* 0*/
/*10*/ 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,/*10*/
/*20*/ 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,/*20*/
/*30*/ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 0, 5, 4, 6, 0,/*30*/
/*40*/ 0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,/*40*/
/*50*/ 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0, 0,/*50*/
/*60*/ 0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,/*60*/
/*70*/ 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0, 7/*70*/
};

typedef struct stateFunc
{
    int state;
    void (*func)(void);
} stateFunc;

static void fb(void);   // beenden
static void fl(void);   // lesen
static void fsl(void);  // schreiben, lesen
static void fgl(void);  // schreiben als Grossbuchstabe, lesen
static void fslb(void); // schreiben, lesen, beenden
                        //
stateFunc vSMatrix[][8] = {
    {{9,fslb},{1,fsl},  {2,fgl},    {3,fsl},    {9,fsl},    {4,fsl},    {5,fsl},    {0,fl}},
    {{9,fb},  {1,fsl},  {9,fb},     {9,fb},     {9,fb},     {9,fb},     {9,fb},     {9,fb}},
    {{9,fb},  {2,fsl},  {2,fgl},    {9,fb},     {9,fb},     {9,fb},     {9,fb},     {9,fb}},
    {{9,fb},  {9,fb},   {9,fb},     {9,fb},     {6,fsl},    {9,fb},     {9,fb},     {9,fb}},
    {{9,fb},  {9,fb},   {9,fb},     {9,fb},     {7,fsl},    {9,fb},     {9,fb},     {9,fb}},
    {{9,fb},  {9,fb},   {9,fb},     {9,fb},     {8,fsl},    {9,fb},     {9,fb},     {9,fb}},
    {{9,fb},  {9,fb},   {9,fb},     {9,fb},     {9,fb},     {9,fb},     {9,fb},     {9,fb}},
    {{9,fb},  {9,fb},   {9,fb},     {9,fb},     {9,fb},     {9,fb},     {9,fb},     {9,fb}},
    {{9,fb},  {9,fb},   {9,fb},     {9,fb},     {9,fb},     {9,fb},     {9,fb},     {9,fb}}
};

static void fb(void)
{
    switch(Z) 
    {
        case 3: // :
        case 4: // <
        case 5: // >
        case 0: // spceial chars
            Morph.Val.Symb=vBuf[0];
            Morph.mc=mcSymb;
            break;
        case 1: // Number
            Morph.Val.Num=atol(vBuf);
            Morph.mc=mcNum;
            break;
        case 2:
            Morph.Val.pStr=vBuf;
            Morph.mc=mcIdent;
            break;

    }
}
// lesen
static void fl(void)
{
    X = fgetc(pIF);
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

int initLex(char *fname)
{
    char vName[128+1];

    strcpy(vName,fname);
    if (strstr(vName, ".pl0") == NULL) {
        //append extension
        strcat(vName, ".pl0");
    }

    pIF=fopen(vName, "r+t");
    if (pIF == NULL) {
        perror("fopen");
        return FAIL;
    }
    X = fgetc(pIF); 
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
        zx=vSMatrix[Z][vZKL[X]].state;
        vSMatrix[Z][vZKL[X]].func();
        Z=zx;
    } while(Z != 9);
    
    return &Morph;
}
