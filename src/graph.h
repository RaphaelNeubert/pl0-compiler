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

enum entry_type {NProp, VConst, VVar, VProc};

struct name_prop {
    enum entry_type et; 
    short idx_proc;
    void *vstrct;
    int len;
    char *name;
};

struct vtype_const {
    enum entry_type et; 
    long val;
    int idx;
};

struct vtype_var {
    enum entry_type et; 
    int displ;          //relative address
};

struct vtype_proc {
    enum entry_type et; 
    short idx_proc;
    struct vtype_proc *parent_proc;
    struct list_head *loc_namelist; // locale name list
    int curr_var_offset;
};


extern struct Edge g_prog[];
void init_namelist();

struct name_prop* global_search_nprop(char *name);
struct vtype_const* search_const(long val);
