#include "lex.h"
#include <stdlib.h>
#include <stdio.h>

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
int nres();
int sres();

extern struct Edge g_expr[];

struct Edge g_fact[] = {
/*0*/ {EtMo, {(ul)mcIdent}, NULL, 5, 1},
/*1*/ {EtMo, {(ul)mcNum},   nres, 5, 2},
/*2*/ {EtSy, {(ul)'('},     NULL, 3, 0},
/*3*/ {EtGr, {(ul)g_expr},  NULL, 4, 0},
/*4*/ {EtSy, {(ul)')'},     NULL, 5, 0},
/*5*/ {EtEn, {(ul)0},       NULL, 0, 0}
};
struct Edge g_term[] = {
/*0*/ {EtGr, {(ul)g_fact}, NULL, 3, 1},
/*1*/ {EtSy, {(ul)'*'},    NULL, 0, 2},
/*2*/ {EtSy, {(ul)'/'},    NULL, 0, 0},
/*3*/ {EtEn, {(ul)0},      NULL, 0, 0}
};

struct Edge g_expr[] = {
/*0*/ {EtSy, {(ul)'-'},    NULL, 1, 1}, 
/*1*/ {EtGr, {(ul)g_term}, NULL, 4, 2},
/*2*/ {EtSy, {(ul)'+'},    sres, 1, 3},
/*3*/ {EtSy, {(ul)'-'},    NULL, 1, 0}, 
/*4*/ {EtEn, {(ul)0},      NULL, 0, 0}
};

tMorph Morph={0};

int nres()
{
    printf("Zahl: %ld wurde geparsed\n",Morph.Val.Num);
    return 1;
}
int sres()
{
    printf("Symbol: %d wurde geparsed\n",Morph.Val.Symb);
    return 1;
}

int pars(struct Edge* p_graph)
{
    struct Edge *p_edge=p_graph;
    int succ=0;

    if (Morph.mc == mcEmpty)
        lex();

    while(1) {
        printf("morph.mc: %d\n",Morph.mc);
        switch(p_edge->edge_type) {
            case EtNl:
                succ=1;
                break;
            case EtSy:
                succ=(Morph.Val.Symb==p_edge->edge_val.S);
                break;
            case EtMo:
                succ=(Morph.mc==p_edge->edge_val.M);
                break;
            case EtGr:
                succ=pars(p_edge->edge_val.G);
                break;
            case EtEn:
                return 1;
        }

        // wenn Kante akzeptiert, dann rufe Kantenfunktion auf
        if (succ && (p_edge->fx != NULL))
            succ=p_edge->fx();

        if (succ) {
            // akzeptiere Token
            if (p_edge->edge_type == EtSy || p_edge->edge_type == EtMo)
                lex();
            p_edge=p_graph+p_edge->i_next;
        }
        else {
            // Alternativbogen probieren
            if (p_edge->i_alt != 0)
                p_edge=p_graph+p_edge->i_alt;
            else
                return FAIL;
        }
    }
}

int main()
{
    initLex("test.pl0");
    int res = pars(g_expr);
    printf("res %d\n",res);
    return 0;
}
