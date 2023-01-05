
#define OK 0
#define FAIL -1
typedef enum mcodes{mcEmpty,mcSymb,mcNum,mcIdent} tMC;

enum keywrdtype
{
    tNIL,tErg=128,tle,tge,
    tBGN,tCLL,tCST,tDO,tEND,tELS,tIF,tGET,tODD,tPUT,tPRC,tTHN,tVAR,tWHL
};

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

void print_err_line();
int initLex(FILE *f);
tMorph* lex(void);
