#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "graph.h"
#include "parse.h"
#include "linkedlist.h"
#include "code_gen.h"

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
static int num_proc=1;
static short *cnst_buf;
static short cnst_buf_idx; //position to write to next
static short cnst_buf_size;

static struct Edge g_expr[];
static struct Edge g_statement[];


static int nprop_cnst();
static int add_cnst();
static int add_var();
static int add_proc();
static int delete_nlist();
static int end_prog();
static int start_proc();
static int end_proc();
static int fac_num();
static int st_putval();

static struct Edge g_block[] = {
/*0*/ {EtSy, {(ul)tCST},       NULL, 1,  6},
/*1*/ {EtMo, {(ul)mcIdent},    nprop_cnst, 2,  0},
/*2*/ {EtSy, {(ul)'='},        NULL, 3,  0},
/*3*/ {EtMo, {(ul)mcNum},      add_cnst, 4,  0},
/*4*/ {EtSy, {(ul)','},        NULL, 1,  5},
/*5*/ {EtSy, {(ul)';'},        NULL, 6,  0},

/*6*/ {EtSy, {(ul)tVAR},       NULL, 7,  10},
/*7*/ {EtMo, {(ul)mcIdent},    add_var, 8,  0},
/*8*/ {EtSy, {(ul)','},        NULL, 7,  9},
/*9*/ {EtSy, {(ul)';'},        NULL, 10, 0},

/*10*/ {EtSy, {(ul)tPRC},       NULL,11, 15},
/*11*/{EtMo, {(ul)mcIdent},    add_proc, 12, 0},
/*12*/{EtSy, {(ul)';'},        NULL, 13, 0},
/*13*/{EtGr, {(ul)g_block},    NULL, 14, 0},
/*14*/{EtSy, {(ul)';'},        delete_nlist, 10, 0},

/*15*/{EtNl, {(ul)0},          start_proc, 16, 0},
/*16*/{EtGr, {(ul)g_statement},end_proc, 17, 17},
/*17*/{EtEn, {(ul)0},          NULL, 0,  0}
};
static struct Edge g_cond[] = {
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

static struct Edge g_statement[] = {
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
/*18*/{EtGr, {(ul)g_expr},     st_putval, 19,  0},

/*19*/{EtEn, {(ul)0},          NULL, 0,  0} 
};

struct Edge g_prog[] = {
/*0*/ {EtGr, {(ul)g_block},NULL, 1, 0},
/*1*/ {EtSy, {(ul)'.'},    end_prog, 2, 0},
/*2*/ {EtEn, {(ul)0},      NULL, 0, 0}
};

static struct Edge g_fact[] = {
/*0*/ {EtMo, {(ul)mcIdent}, NULL, 5, 1},
/*1*/ {EtMo, {(ul)mcNum},   fac_num, 5, 2},
/*2*/ {EtSy, {(ul)'('},     NULL, 3, 0},
/*3*/ {EtGr, {(ul)g_expr},  NULL, 4, 0},
/*4*/ {EtSy, {(ul)')'},     NULL, 5, 0},
/*5*/ {EtEn, {(ul)0},       NULL, 0, 0}
};
static struct Edge g_term[] = {
/*0*/ {EtGr, {(ul)g_fact}, NULL, 1, 0},
/*1*/ {EtSy, {(ul)'*'},    NULL, 2, 3},
/*2*/ {EtGr, {(ul)g_fact}, NULL, 1, 0},
/*3*/ {EtSy, {(ul)'/'},    NULL, 4, 5},
/*4*/ {EtGr, {(ul)g_fact}, NULL, 1, 0},
/*5*/ {EtEn, {(ul)0},      NULL, 0, 0}
};

static struct Edge g_expr[] = {
/*0*/ {EtSy, {(ul)'-'},    NULL, 1, 2}, 
/*1*/ {EtGr, {(ul)g_term}, NULL, 3, 0},
/*2*/ {EtGr, {(ul)g_term}, NULL, 3, 0},
/*3*/ {EtSy, {(ul)'+'},    NULL, 4, 5},
/*4*/ {EtGr, {(ul)g_term}, NULL, 3, 0},
/*5*/ {EtSy, {(ul)'-'},    NULL, 6, 7}, 
/*6*/ {EtGr, {(ul)g_term}, NULL, 3, 0},
/*7*/ {EtEn, {(ul)0},      NULL, 0, 0}
};


void init_namelist()
{
    main_proc.et=VProc;
    if ((main_proc.loc_namelist=create_list()) == NULL) {
        fprintf(stderr, "Failed to create local main_proc\n");
        exit(-1);
    }
    curr_proc=&main_proc;
    cnst_buf=malloc(64*sizeof(short));
    if (!cnst_buf) {
        perror("malloc");
        exit(-1);
    }
    cnst_buf_size=64*sizeof(int);
}

static struct name_prop* create_nprop(char *name)
{
    char *new_name;
    struct name_prop *nprop=malloc(sizeof(struct name_prop));
    if (!nprop) {
        perror("malloc");
        exit(-1);
    }
    nprop->et=NProp;
    nprop->idx_proc=curr_proc->idx_proc;
    nprop->vstrct=NULL;
    nprop->len=strlen(name);
    new_name=malloc(nprop->len+1);
    if (!new_name) {
        perror("malloc");
        exit(-1);
    }
    strcpy(new_name, name);
    nprop->name=new_name;

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
    num_proc++;

    return proc;
}
static struct vtype_var* create_var()
{
    struct vtype_var *var=malloc(sizeof(struct vtype_var));
    if (!var) {
        perror("malloc");
        exit(-1);
    }
    var->et=VVar;
    var->displ=curr_proc->curr_var_offset;
    curr_proc->curr_var_offset+=4;

    return var;
}
static struct vtype_const* create_const(long val, short inc)
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
    idx+=inc;
    
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
    struct vtype_const *cnst;

    struct name_prop *nprop=list_get_first(list);
    while(nprop) {
        cnst=nprop->vstrct;
        if (cnst && cnst->et == VConst && cnst->val == val)
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
static void display_list(struct vtype_proc *proc)
{
    struct list_head *list=proc->loc_namelist;
    struct name_prop *nprop=list_get_first(list);
    while (nprop) {
        switch (nprop->et) {
            case VProc:
                printf("%04d: procedure %s\n",proc->idx_proc,nprop->name);
                break;
            case VConst:
                struct vtype_const *cnst=nprop->vstrct;
                printf("%04d: Const name: %s, idx: %d, val: %ld\n",
                        proc->idx_proc,nprop->name, cnst->idx, cnst->val);
                break;
            case VVar:
                struct vtype_var *var=nprop->vstrct;
                printf("%04d: Var name: %s, disp: %d\n", proc->idx_proc,nprop->name, var->displ);
                break;
            default:
                puts("type unknown");
                break;
        }

        nprop=list_get_next(list);
    }
}

// creates, checks for redefinition and inserts nprop
static struct name_prop* create_insert_nprop(enum entry_type et)
{
    struct name_prop *nprop;
    struct list_head *list;

    //dcnstisplay_list(curr_proc);

    if (search_nprop(curr_proc, Morph.Val.pStr)) {
        //printf("error Line %d, Column %d: redefinition of %s\n",
        //       Morph.posLine, Morph.posCol, Morph.Val.pStr);
        printf("error: redefinition of %s\n", Morph.Val.pStr);
        return NULL;
    }
    nprop=create_nprop(Morph.Val.pStr);
    nprop->et=et;
    list=curr_proc->loc_namelist;
    if (list_insert_tail(list, nprop) == 0) {
        fprintf(stderr, "failed to insert nprop into list\n");
        exit(-1);
    }

    return nprop;
}

// creates and appends nprop to current procedure
static int nprop_cnst()
{
    return (create_insert_nprop(VConst) != NULL);
}
static int search_const_buf(int num)
{
    for (int i=0; i<cnst_buf_idx; i++) {
        if (cnst_buf[i] == num)
            return i;
    }
    return -1; // not found
}
static void save_add_cnst(int num)
{
    if (cnst_buf_size < (cnst_buf_idx+1)*sizeof(int)) {
        cnst_buf=realloc(cnst_buf, cnst_buf_size+=64*sizeof(short));
        if (!cnst_buf) {
            perror("failed to realloc memory for const buffer");
            exit(-1);
        }
    }
    cnst_buf[cnst_buf_idx++]=num;
}
static int add_cnst_buf() 
{
    int num=Morph.Val.Num;
    int idx=search_const_buf(num);
    if (idx < 0) { // not found
        save_add_cnst(num);
    }
    return 1;
}

// creates const type and sets last nprop point to it
static int add_cnst()
{
    struct vtype_const *new_cnst;
    struct list_head *list;
    struct name_prop *nprop;
    long num=Morph.Val.Num;
    struct vtype_const *cnst;

    cnst=search_const(num);
    if (cnst) {
        new_cnst=create_const(num,0); //don't increment idx
        new_cnst->idx=cnst->idx;
    }
    else {
        new_cnst=create_const(num,1); //create and increment idx
    }
    list=curr_proc->loc_namelist;
    nprop=list_get_last(list);
    nprop->vstrct=new_cnst;

    return add_cnst_buf();
}

static int add_var()
{
    struct vtype_var *var;
    struct name_prop *nprop=create_insert_nprop(VVar);
    if (!nprop)
        return 0;
    var=create_var();
    nprop->vstrct=var;
    return 1;
}

static int add_proc()
{
    struct vtype_proc *proc;
    struct name_prop *nprop=create_insert_nprop(VProc);
    if (!nprop)
        return 0;
    proc=create_proc(curr_proc);
    nprop->vstrct=proc;
    curr_proc=proc;
    return 1;
}

static void rec_list_del(struct list_head *list)
{
    struct name_prop *item=list_get_first(list);
    while (item) {
        if (item->et == VProc) {
            rec_list_del(((struct vtype_proc*)item->vstrct)->loc_namelist);
            free(item->vstrct);
        }
        else {
            free(item->vstrct);
        }
        free(item->name);
        free(item);
        list_remove_curr(list);

        item=list_get_first(list);
    }
}

static int delete_nlist()
{
    puts("Displaying list:");
    display_list(curr_proc);
    puts("Delete was called");
    //rec_list_del(main_proc.loc_namelist);
    struct list_head *list=curr_proc->loc_namelist;
    struct name_prop *item=list_get_first(list);
    while (item) {
        free(item->vstrct);
        free(item->name);
        free(item);
        list_remove_curr(list);

        item=list_get_first(list);
    }
    free(list);
    curr_proc=curr_proc->parent_proc;

    return 1;
}

//st 10
static int st_putval()
{
    return generate_code(putVal);
}

// suchen, ggf. anlegen, puConst(ConstIndex)
static int fac_num()
{
    int val=Morph.Val.Num;
    int idx=search_const_buf(val);
    if (idx >= 0) {
        generate_code(puConst, idx);
    }
    else {
        save_add_cnst(val);
        generate_code(puConst,cnst_buf_idx-1);
    }
    return 1;
}
static int start_proc()
{
    int init_size=1024;
    int len_code=0;
    int pidx=curr_proc->idx_proc;
    int len_var=curr_proc->curr_var_offset;

    cbuf_start=malloc(init_size);
    if (!cbuf_start) {
        perror("malloc");
        exit(-1);
    }
    cbuf_curr=cbuf_start;
    cbuf_size=init_size;

    generate_code(entryProc, len_code, pidx, len_var);
    return 1;

}
static int end_proc()
{
    generate_code(retProc);

    puts("---------printing Code Buffer----------");
    for (char* byte=cbuf_start; byte!=cbuf_curr; byte++) {
        printf(" %02hhX ",*byte);
    }
    puts("\n---------done------------");
    write_code2file(curr_proc->curr_var_offset);
    return 1;
}

static int end_prog()
{
    write_consts2file(cnst_buf,cnst_buf_idx);
    write_num_proc2file(num_proc);

    delete_nlist();
    free(cnst_buf);
    return 1;
}
