#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "graph.h"

typedef unsigned long ul;


tMorph Morph={0};
FILE *pOF;


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
                //puts("going in");
                succ=pars(p_edge->edge_val.G);
                //puts("came out");
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
            //printf("std: %d\n", p_edge->i_next);
            p_edge=p_graph+p_edge->i_next;
        }
        else {
            // Alternativbogen probieren
            if (p_edge->i_alt != 0) {
                //printf("alt: %d\n", p_edge->i_alt);
                p_edge=p_graph+p_edge->i_alt;
            }
            else
                return 0;

        }
    }
}

int main(int argc, char **argv)
{
    char vName[128+1];
    char *oName;
    FILE *pIF;
    if (argc != 2) {
        puts("usage: cpl0 <filename>");
        exit(0);
    }
    strcpy(vName,argv[1]);
    if (strcmp(vName+strlen(vName)-4, ".pl0"))
        strcat(vName, ".pl0");

    pIF=fopen(vName, "r+t");
    if (pIF == NULL) {
        perror("fopen");
        return FAIL;
    }
    vName[strlen(vName)-4]='\0';
    if (!(oName=strrchr(vName,'/')))
        oName=vName;
    oName++;
    strcat(oName,".cl0");
    puts(oName);
    pOF=fopen(oName, "w");
    if (pOF == NULL) {
        perror("fopen");
        return FAIL;
    }
    long x=0L;
    fwrite(&x,sizeof(int32_t),1,pOF);
    initLex(pIF);
    init_namelist();

    int res = pars(g_prog);
    if ((res && Morph.mc == mcEmpty) || Morph.Val.Symb == -1) 
        puts("Erfolg");
    else {
        printf("Fehler, pars returned %d\n",res);
        printf("Zeile: %d, Spalte %d\n",Morph.posLine, Morph.posCol);
    }
        
    fclose(pIF);
    fclose(pOF);
    return 0;
}
