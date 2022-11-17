#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "graph.h"
#include "linkedlist.h"

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

struct vtype_proc main_proc;
struct vtype_proc *curr_proc;

struct Edge g_expr[];
struct Edge g_statement[];

int ires();
int nres();
int sres();

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
/*2*/ {EtGr, {(ul)g_expr},     NULL, 19,  0},
// if
/*3*/ {EtSy, {(ul)tIF},        NULL, 4,  7},
/*4*/ {EtGr, {(ul)g_cond},     NULL, 5,  0},
/*5*/ {EtSy, {(ul)tTHN},       NULL, 6,  0},
/*6*/ {EtGr, {(ul)g_statement},NULL, 19,  0},
// while
/*7*/ {EtSy, {(ul)tWHL},       NULL, 8,  10},
/*8*/ {EtGr, {(ul)g_cond},     NULL, 9,  0},
/*9*/ {EtSy, {(ul)tDO},        NULL, 6, 0},
// begin
/*10*/{EtSy, {(ul)tBGN},       NULL, 11, 14},
/*11*/{EtGr, {(ul)g_statement},NULL, 12, 0},
/*12*/{EtSy, {(ul)';'},        NULL, 11,  13},
/*13*/{EtSy, {(ul)tEND},       NULL, 19, 0},
// call
/*14*/{EtSy, {(ul)tCLL},       NULL, 15, 16},
/*15*/{EtMo, {(ul)mcIdent},    NULL, 19,  0},
// ?
/*16*/{EtSy, {(ul)'?'},        NULL, 15, 17},
// !
/*17*/{EtSy, {(ul)'!'},        NULL, 18, 19},
/*18*/{EtGr, {(ul)g_expr},     NULL, 19,  0},

/*19*/{EtEn, {(ul)0},          NULL, 0,  0} 
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

int init_namelist()
{
    main_proc.et=VProc;
    if ((main_proc.loc_namelist=create_list()) == NULL) {
        fprintf(stderr, "Failed to create local main_proc");
        return -1;
    }
    curr_proc=&main_proc;
    return 0;
}

static struct name_prop* create_nprop(char *name)
{
    struct name_prop *nprop=malloc(sizeof(struct name_prop));
    if (!nprop) {
        perror("malloc");
        exit(-1);
    }
    nprop->et=NProp;
    nprop->idx_proc=curr_proc->idx_proc;
    nprop->vstrct=NULL;
    nprop->len=strlen(name);
    nprop->name=name;

    return nprop;
}
static struct vtype_proc* create_proc(struct vtype_proc* parent)
{
    struct vtype_proc *proc=malloc(sizeof(struct vtype_proc));
    if (!proc) {
        perror("malloc");
        exit(-1);
    }
    proc->et=VProc;
    proc->idx_proc=parent->idx_proc+1;
    proc->parent_proc=parent;
    if ((proc->loc_namelist=create_list()) == NULL) {
        fprintf(stderr, "Failed to create local namelist");
        exit(-1);
    }
    proc->curr_var_offset=0;

    return proc;
}
static int create_var()
{
    struct vtype_var *var=malloc(sizeof(struct vtype_var));
    if (!var) {
        perror("malloc");
        exit(-1);
    }
    var->et=VVar;
    var->displ=curr_proc->curr_var_offset;
    curr_proc->curr_var_offset+=4;

    return var->displ;
}
static struct vtype_const* create_const(long val)
{
    static int idx=0;
    struct vtype_const *cnst=malloc(sizeof(struct vtype_const));
    if (!cnst) {
        perror("malloc");
        exit(-1);
    }
    cnst->et=VConst;
    cnst->val=val;
    cnst->idx=idx;
    //prepare index for next function call
    idx++;
    
    return cnst;
}

static struct name_prop* search_nprop(struct vtype_proc *proc, char *name)
{
    struct list_head *list=proc->loc_namelist;

    struct name_prop *nprop=list_get_first(list);
    while(nprop) {
        if (strcmp(nprop->name, name) == 0)
            return nprop;

        nprop=list_get_next(list);
    }
    return NULL;
}

static struct vtype_const* search_const(long val)
{
    struct list_head *list=main_proc.loc_namelist;

    struct name_prop *nprop=list_get_first(list);
    while(nprop) {
        if (((struct vtype_const*)nprop->vstrct)->et == VConst &&
                ((struct vtype_const*)nprop->vstrct)->val == val)
            return nprop->vstrct;
            
        nprop=list_get_next(list);
    }
    return NULL;
}
static struct name_prop* global_search_nprop(char *name)
{
    struct vtype_proc *proc=curr_proc;
    struct name_prop *nprop;
    struct list_head *list;
    
    while(proc) {
        list=proc->loc_namelist;
        nprop=list_get_first(list);
        while(nprop) {
            if (strcmp(nprop->name, name) == 0)
                return nprop;

            nprop=list_get_next(list);
        }
        proc=proc->parent_proc;
    }
    return NULL;
}
