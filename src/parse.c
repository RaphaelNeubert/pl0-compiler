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
int ires();

struct Edge g_expr[];
struct Edge g_statement[];

struct Edge g_block[] = {
/*0*/ {EtSy, {(ul)tCST},       NULL, 1,  5},
/*1*/ {EtMo, {(ul)mcIdent},    ires, 2,  0},
/*2*/ {EtMo, {(ul)mcNum},      nres, 3,  0},
/*3*/ {EtSy, {(ul)','},        sres, 1,  4},
/*4*/ {EtSy, {(ul)';'},        sres, 5,  0},
/*5*/ {EtSy, {(ul)tVAR},       NULL, 6,  9},
/*6*/ {EtMo, {(ul)mcIdent},    ires, 7,  0},
/*7*/ {EtSy, {(ul)','},        sres, 6,  8},
/*8*/ {EtSy, {(ul)';'},        sres, 9,  0},
/*9*/ {EtSy, {(ul)tPRC},       NULL, 10, 14},
/*10*/{EtMo, {(ul)mcIdent},    ires, 11, 0},
/*11*/{EtSy, {(ul)';'},        sres, 12, 0},
/*12*/{EtGr, {(ul)g_block},    NULL, 13, 0},
/*13*/{EtSy, {(ul)';'},        sres, 9,  0},
/*14*/{EtNl, {(ul)0},          NULL, 15, 0},
/*15*/{EtGr, {(ul)g_statement},NULL, 16, 16},
/*16*/{EtEn, {(ul)0},          NULL, 0,  0}
};
struct Edge g_cond[] = {
/*0*/ {EtSy, {(ul)tODD},    NULL, 1,  2},
/*1*/ {EtGr, {(ul)g_expr},  NULL, 9,  0},
/*2*/ {EtGr, {(ul)g_expr},  NULL, 3,  0},
/*3*/ {EtSy, {(ul)'='},     NULL, 1,  4},
/*4*/ {EtSy, {(ul)'#'},     NULL, 1,  5},
/*5*/ {EtSy, {(ul)'<'},     NULL, 1,  6},
/*6*/ {EtSy, {(ul)'>'},     NULL, 1,  7},
/*7*/ {EtSy, {(ul)tle},     NULL, 1,  8},
/*8*/ {EtSy, {(ul)tge},     NULL, 1,  0},
/*9*/ {EtEn, {(ul)0},       NULL, 0,  0}
};

struct Edge g_statement[] = {
// a := b+c
/*0*/ {EtMo, {(ul)mcIdent},    NULL, 1,  3},
/*1*/ {EtSy, {(ul)tErg},       NULL, 2,  0},
/*2*/ {EtGr, {(ul)g_expr},     NULL, 21,  0},
// if
/*3*/ {EtSy, {(ul)tIF},        NULL, 4,  7},
/*4*/ {EtGr, {(ul)g_cond},     NULL, 5,  0},
/*5*/ {EtSy, {(ul)tTHN},       NULL, 6,  0},
/*6*/ {EtGr, {(ul)g_statement},NULL, 21,  0},
// while
/*7*/ {EtSy, {(ul)tWHL},       NULL, 8,  11},
/*8*/ {EtGr, {(ul)g_cond},     NULL, 9,  0},
/*9*/ {EtSy, {(ul)tDO},        NULL, 10, 0}, //TODO point to 6 instead of 10
/*10*/{EtGr, {(ul)g_statement},NULL, 21, 0}, 
// begin
/*11*/{EtSy, {(ul)tBGN},       NULL, 12, 15},
/*12*/{EtGr, {(ul)g_statement},NULL, 13, 0},
/*13*/{EtSy, {(ul)';'},        NULL, 12,  14},
/*14*/{EtSy, {(ul)tEND},       NULL, 21, 0},
// call
/*15*/{EtSy, {(ul)tCLL},       NULL, 16, 17},
/*16*/{EtMo, {(ul)mcIdent},    NULL, 21,  0},
// ?
/*17*/{EtSy, {(ul)'?'},        NULL, 18, 19}, //TODO point to 16 instead of 18
/*18*/{EtMo, {(ul)mcIdent},    NULL, 21,  0},
// !
/*19*/{EtSy, {(ul)'!'},        NULL, 20, 21},
/*20*/{EtGr, {(ul)g_expr},     NULL, 21,  0},

/*21*/{EtEn, {(ul)0},          NULL, 0,  0} 
};

struct Edge g_prog[] = {
/*0*/ {EtGr, {(ul)g_block}, NULL, 1, 0},
/*1*/ {EtSy, {(ul)'.'},    NULL, 2, 0},
/*2*/ {EtEn, {(ul)0},      NULL, 0, 0}
};

struct Edge g_fact[] = {
/*0*/ {EtMo, {(ul)mcIdent}, ires, 5, 1},
/*1*/ {EtMo, {(ul)mcNum},   nres, 5, 2},
/*2*/ {EtSy, {(ul)'('},     sres, 3, 0},
/*3*/ {EtGr, {(ul)g_expr},  NULL, 4, 0},
/*4*/ {EtSy, {(ul)')'},     sres, 5, 0},
/*5*/ {EtEn, {(ul)0},       NULL, 0, 0}
};
struct Edge g_term[] = {
/*0*/ {EtGr, {(ul)g_fact}, NULL, 1, 0},
/*1*/ {EtSy, {(ul)'*'},    sres, 2, 3},
/*2*/ {EtGr, {(ul)g_fact}, NULL, 1, 0},
/*3*/ {EtSy, {(ul)'/'},    sres, 4, 5},
/*4*/ {EtGr, {(ul)g_fact}, NULL, 1, 0},
/*5*/ {EtEn, {(ul)0},      NULL, 0, 0}
};

struct Edge g_expr[] = {
/*0*/ {EtSy, {(ul)'-'},    sres, 1, 2}, 
/*1*/ {EtGr, {(ul)g_term}, NULL, 3, 0},
/*2*/ {EtGr, {(ul)g_term}, NULL, 3, 0},
/*3*/ {EtSy, {(ul)'+'},    sres, 4, 5},
/*4*/ {EtGr, {(ul)g_term}, NULL, 3, 0},
/*5*/ {EtSy, {(ul)'-'},    sres, 6, 7}, 
/*6*/ {EtGr, {(ul)g_term}, NULL, 3, 0},
/*7*/ {EtEn, {(ul)0},      NULL, 0, 0}
};

tMorph Morph={0};

int ires()
{
    printf("Identifier: %s wurde geparsed\n",Morph.Val.pStr);
    return 1;
}

int nres()
{
    printf("Zahl: %ld wurde geparsed\n",Morph.Val.Num);
    return 1;
}
int sres()
{
    printf("Symbol: %c wurde geparsed\n",(char) Morph.Val.Symb);
    return 1;
}

int pars(struct Edge* p_graph)
{
    struct Edge *p_edge=p_graph;
    int succ=0;

    if (Morph.mc == mcEmpty)
        lex();

    while(1) {
        //printf("Morph.mc %d\n",Morph.mc);
        //printf("Morph.Val %ld\n",Morph.Val.Num);
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
                puts("going in");
                succ=pars(p_edge->edge_val.G);
                puts("came out");
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
            printf("std: %d\n", p_edge->i_next);
            p_edge=p_graph+p_edge->i_next;
        }
        else {
            // Alternativbogen probieren
            if (p_edge->i_alt != 0) {
                printf("alt: %d\n", p_edge->i_alt);
                p_edge=p_graph+p_edge->i_alt;
            }
            else
                return 0;

        }
    }
}

int main()
{
    initLex("tests/fakultRecursiv.pl0");

    int res = pars(g_prog);
    if (res && Morph.mc == mcEmpty) 
        puts("Erfolg");
    else {
        printf("Fehler, pars returned %d\n",res);
        printf("Zeile: %d, Spalte %d\n",Morph.posLine, Morph.posCol);
    }
        

    //do {
    //    lex();
    //    printf("lexres char: %c,int: %ld\n",Morph.Val.Num ,Morph.Val.Num);
    //} while (Morph.mc != mcEmpty);
    return 0;
}
