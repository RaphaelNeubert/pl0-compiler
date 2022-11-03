#include "lex.h"
#include <stdlib.h>

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
        struct Edge* G;     // Verweis auf Graph
    } edge_val;
    int (*fx)(void);        // Funktion, wenn Bogen akzeptiert wird
    int i_next;              //Folgebogen, wenn Bogen akzeptiert
    int i_alt;               // Alternativbogen, wenn Bogen nicht akzeptiert
};


int pars(struct Edge* p_graph)
{
    struct Edge *p_edge=p_graph;
    int succ=0;

    if (Morph.mc == mcEmpty)
        lex();

    while(1) {
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
            if (p_edge->edge_type & EtSy || p_edge->edge_type & EtMo)
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
