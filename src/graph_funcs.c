#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "parse.h"
#include "lex.h"
#include "linkedlist.h"
#include "stack.h"
#include "code_gen.h"
#include "graph_funcs.h"

struct vtype_proc main_proc;
struct vtype_proc *curr_proc;
static int num_proc=1;
static int *cnst_buf;
static int cnst_buf_idx; //position to write to next
static int cnst_buf_size;

static struct stack_head *jump_pos_stack; //used for jpos and proc calls

static int condition_operator;
static char condition_connector;


void init_namelist()
{
    main_proc.et=VProc;
    if ((main_proc.loc_namelist=create_list()) == NULL) {
        fprintf(stderr, "Failed to create local main_proc\n");
        exit(-1);
    }
    curr_proc=&main_proc;
    cnst_buf=malloc(64*sizeof(int));
    if (!cnst_buf) {
        perror("malloc");
        exit(-1);
    }
    cnst_buf_size=64*sizeof(int);
}
void init_jump_pos_stack()
{
    jump_pos_stack=create_stack();
    if (!jump_pos_stack) {
        fprintf(stderr, "Failed to create jump_pos_stack stack\n");
        exit(-1);
    }
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
    proc->param_counter=0;
    num_proc++;

    return proc;
}
static struct vtype_var* create_param()
{
    struct vtype_var *var=malloc(sizeof(struct vtype_var));
    if (!var) {
        perror("malloc");
        exit(-1);
    }
    var->et=VParam;
    //var->displ=curr_proc->curr_var_offset;
    //curr_proc->curr_var_offset+=4;

    return var;
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
/*
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
            case VParam:
                struct vtype_var *param=nprop->vstrct;
                printf("%04d: Param name: %s, disp: %d\n", proc->idx_proc,nprop->name, param->displ);
                break;
            default:
                puts("type unknown");
                break;
        }

        nprop=list_get_next(list);
    }
}
*/

// creates, checks for redefinition and inserts nprop
static struct name_prop* create_insert_nprop(enum entry_type et)
{
    struct name_prop *nprop;
    struct list_head *list;

    //dcnstisplay_list(curr_proc);

    if (search_nprop(curr_proc, Morph.Val.pStr)) {
        printf("Error: redefinition of %s\n", Morph.Val.pStr);
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
int nprop_cnst()
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
        cnst_buf=realloc(cnst_buf, cnst_buf_size+=64*sizeof(int));
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
int add_cnst()
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

int add_param()
{
    struct vtype_var *var;
    struct name_prop *nprop=create_insert_nprop(VParam);
    if (!nprop)
        return 0;
    var=create_param();
    nprop->vstrct=var;
    curr_proc->param_counter++;
    return 1;
}
int add_var()
{
    struct vtype_var *var;
    struct name_prop *nprop=create_insert_nprop(VVar);
    if (!nprop)
        return 0;
    var=create_var();
    nprop->vstrct=var;
    return 1;
}

int add_proc()
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

int delete_nlist()
{
    //puts("Displaying list:");
    //display_list(curr_proc);
    //puts("Delete was called");
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
int st_putval()
{
    return generate_code(putVal);
}
//ex1
int expr_vz()
{
    return generate_code(vzMinus);
}
//ex2
int expr_add()
{
    return generate_code(OpAdd);
}
//ex3
int expr_sub()
{
    return generate_code(OpSub);
}
// te1
int term_mult()
{
    return generate_code(OpMult);
}
// te2
int term_div()
{
    return generate_code(OpDiv);
}

// suchen, ggf. anlegen, puConst(ConstIndex) (fa1)
int fac_num()
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
// (fa2)
int fac_ident()
{
    struct name_prop *nprop=global_search_nprop(Morph.Val.pStr);
    if (!nprop) {
        printf("Error: Identifier: %s was not declared!\n", Morph.Val.pStr);
        return 0;
    }
    if (nprop->et == VConst) {
        generate_code(puConst, ((struct vtype_const*)(nprop->vstrct))->idx);
    }
    else if (nprop->et == VVar || nprop->et == VParam) {
        struct vtype_var *var=nprop->vstrct;
        if (nprop->idx_proc == 0) {
            // Main
            generate_code(puValVrMain, var->displ);
        }
        else if (nprop->idx_proc == curr_proc->idx_proc) {
            // Local
            generate_code(puValVrLocl, var->displ);
        }
        else {
            // Global
            generate_code(puValVrGlob, var->displ, nprop->idx_proc);
        }
    }
    else if (nprop->et == VProc) {
        puts("can't use a procedure as a factor");
    }
    return 1;
}
static int check_var(struct name_prop *nprop)
{
    if (nprop->et == VConst) {
        puts("assignment to const not allowed!");
        return 0;
    }
    else if (nprop->et == VProc) {
        puts("assignment to procedure not allowed!");
        return 0;
    }
    else if (nprop->et == VVar) {
        return 1;
    }
    else {
        return 0;
    }
}

static void gen_var_adr_code(struct name_prop *nprop)
{
    struct vtype_var *var=nprop->vstrct;
    if (nprop->idx_proc == 0) {
        // Main
        generate_code(puAdrVrMain, var->displ);
    }
    else if (nprop->idx_proc == curr_proc->idx_proc) {
        // Local
        generate_code(puAdrVrLocl, var->displ);
    }
    else {
        // Global
        generate_code(puAdrVrGlob, var->displ, nprop->idx_proc);
    }
}
//st 1
int stmnt_assign()
{
    struct name_prop *nprop=global_search_nprop(Morph.Val.pStr);
    if (!nprop) {
        puts("Identifier was not declared!");
        return 0;
    }
    if (check_var(nprop)) {
        gen_var_adr_code(nprop);
        return 1;
    }
    else {
        return 0;
    }
}
// st2
int stmnt_store()
{
    return generate_code(storeVal);
}
// st 9
int stmnt_get()
{
    struct name_prop *nprop=global_search_nprop(Morph.Val.pStr);
    if (!nprop) {
        printf("Error: Identifier: %s was not declared!\n",Morph.Val.pStr);
        return 0;
    }
    if (check_var(nprop)) {
        gen_var_adr_code(nprop);
        generate_code(getVal);

    }
    else {
        return 0;
    }
    return 1;
}

//co1
int cond_odd()
{
    return generate_code(odd);
}
//co2-7
int set_cond_opp()
{
    condition_operator=Morph.Val.Symb;
    return 1;
}
//co8
int cond_write()
{
    switch(condition_operator) {
        case '=':
            generate_code(cmpEQ);
            break;
        case '#':
            generate_code(cmpNE);
            break;
        case '<':
            generate_code(cmpLT);
            break;
        case '>':
            generate_code(cmpGT);
            break;
        case tle:
            generate_code(cmpLE);
            break;
        case tge:
            generate_code(cmpGE);
            break;
        default:
            return 0;
    }
    switch (condition_connector) {
        case 'a': // and
            generate_code(OpMult);
            condition_operator=0;
            break;
        case 'o': // or
            generate_code(OpAdd);
            condition_operator=0;
            break;
        default:
            break;
    }
    return 1;
}
int cond_and()
{
    condition_connector='a';
    return 1;
}
int cond_or()
{
    condition_connector='o';
    return 1;
}
//st3
int stmnt_if_jnot()
{
    stack_push(jump_pos_stack, cbuf_curr);
    generate_code(jnot, 0);
    return 1;
}
//st4
int stmnt_if_jaddr()
{
    char *pjnot=stack_pop(jump_pos_stack);
    //jump starts after relative addr parameter 
    //also note that we need to jump behind the jmp of the else
    short rel_addr=cbuf_curr-pjnot; 
    write_code_at(rel_addr, pjnot+1);
    generate_code(jmp, 0); // jump for else
    return 1;
}
int stmnt_else_jmp()
{
    stack_push(jump_pos_stack, cbuf_curr);
    return 1;
}
int stmnt_else_jaddr()
{
    char *pjnot=stack_pop(jump_pos_stack);
    //jump starts after relative addr parameter 
    short rel_addr=cbuf_curr-pjnot; 
    write_code_at(rel_addr, pjnot-2);
    return 1;
}
//st5
int stmnt_while_jback()
{
    stack_push(jump_pos_stack, cbuf_curr);
    return 1;

}
//st6
int stmnt_while_jnot()
{
    stack_push(jump_pos_stack, cbuf_curr);
    generate_code(jnot, 0);
    return 1;
}
//st7
int stmnt_while_jaddr()
{
    char *pjnot=stack_pop(jump_pos_stack);
    char *pjback=stack_pop(jump_pos_stack);
    short rel_addr=cbuf_curr-pjnot;  //jnot
    write_code_at(rel_addr, pjnot+1);
    rel_addr=-1*(cbuf_curr-pjback+3);         //jback
    generate_code(jmp, rel_addr);
    return 1;
}
//st8
int stmnt_call()
{
    struct name_prop *nprop=global_search_nprop(Morph.Val.pStr);
    if (!nprop) {
        printf("Error: Procedure: %s undeclared\n", Morph.Val.pStr);
        return 0;
    }
    else if(nprop->et != VProc) {
        printf("Error: Variable: %s is not a procedure but expected to be one\n", Morph.Val.pStr);
        return 0;
    }
    stack_push(jump_pos_stack, nprop); 
    //call_proc_num=((struct vtype_proc*)nprop->vstrct)->idx_proc;
    return 1;
}
int stmnt_call_param()
{
    struct name_prop *nprop=stack_top(jump_pos_stack);
    struct vtype_proc *proc=(struct vtype_proc*)nprop->vstrct;
    proc->param_counter--;
    return 1;
}
int stmnt_call_end()
{
    struct name_prop *nprop=stack_pop(jump_pos_stack); 
    struct vtype_proc *proc=(struct vtype_proc*)nprop->vstrct;
    if (proc->param_counter > 0) {
        printf("Error: Not enough parameters in call of function: %s\n",nprop->name);
        return 0;
    }
    else if (proc->param_counter < 0) {
        printf("Error: Too many parameters in call of function: %s\n",nprop->name);
        return 0;
    }
    return generate_code(call, proc->idx_proc);
}

int start_proc()
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
int end_proc()
{
    generate_code(retProc);

    write_code2file(curr_proc->curr_var_offset);
    //free(jump_pos_stack);
    //jump_pos_stack=NULL;
    return 1;
}

int end_prog()
{
    write_consts2file(cnst_buf,cnst_buf_idx);
    write_num_proc2file(num_proc);

    delete_nlist();
    free(jump_pos_stack);
    free(cnst_buf);
    free(cbuf_start);
    return 1;
}

int enter_param_displ()
{
    struct list_head *list=curr_proc->loc_namelist;
    struct vtype_var *param;
    int offset=-24;

    struct name_prop *nprop=list_get_last(list);
    while(nprop) {
        if (nprop->et == VParam) {
            param=nprop->vstrct;
            param->displ=offset;
            offset-=4;
        }
        nprop=list_get_prev(list);
    }
    return 1;
}
