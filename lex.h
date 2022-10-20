
#define OK 0
#define FAIL -1
typedef enum mcodes{mcEmpty,mcSymb,mcNum,mcIdent} tMC;

typedef struct morph
{
    tMC mc;
    int posLine;
    int posCol;
    int MLen;
    union VAL
    {
        long Num;
        char *pStr;
        int Symb;
    } Val;
} tMorph;

extern tMorph Morph; // globale Variable f. Akt. Token

int initLex(char *fname);
tMorph* lex(void);
