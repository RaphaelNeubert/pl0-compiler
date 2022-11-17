#include <stdlib.h>
#include <stdio.h>
#include "graph.h"

typedef unsigned long ul;


tMorph Morph={0};


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

int main(int argc, char **argv)
{
    if (argc != 2) {
        puts("usage: cpl0 <filename>");
        exit(0);
    }
    initLex(argv[1]);

    int res = pars(g_prog);
    if ((res && Morph.mc == mcEmpty) || Morph.Val.Symb == -1) 
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
