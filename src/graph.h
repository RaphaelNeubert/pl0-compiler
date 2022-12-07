#include "lex.h"

typedef unsigned long ul;

enum Etype
{
    EtNl=0, // NIL
    EtSy=1, // Symbol
    EtMo=2, // Morphem
    EtGr=4,  // Graph
    EtEn=8  // Graphende
};

struct Edge 
{
    enum Etype edge_type;   //(Nil, Symbol, Name,Zahl)
    union Eval
    {
        unsigned long X;    // fuer Initialisierung
        int S;              // Symbol
        tMC M;              // Morphemtyp
        struct Edge *G;     // Verweis auf Graph
    } edge_val;
    int (*fx)(void);        // Funktion, wenn Bogen akzeptiert wird
    int i_next;              //Folgebogen, wenn Bogen akzeptiert
    int i_alt;               // Alternativbogen, wenn Bogen nicht akzeptiert
};


extern struct Edge g_prog[];
void init_namelist();
void init_jump_pos_stack();


