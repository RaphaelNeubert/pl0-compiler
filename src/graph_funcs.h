
enum entry_type {NProp, VConst, VVar, VProc,VParam};

struct name_prop {
    enum entry_type et; 
    int idx_proc;
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
    int param_counter; //counts up at proc declaration, down at call
};

void init_namelist();
void init_jump_pos_stack();

int nprop_cnst();
int add_cnst();
int add_var();
int add_param();
int add_proc();
int delete_nlist();
int end_prog();
int start_proc();
int end_proc();
int fac_num();
int fac_ident();
int st_putval();
int expr_add();
int expr_sub();
int expr_vz();
int term_mult();
int term_div();
int stmnt_assign();
int stmnt_store();
int stmnt_get();
int stmnt_if_jaddr();
int stmnt_if_jnot();
int stmnt_else_jaddr();
int stmnt_else_jmp();
int stmnt_while_jaddr();
int stmnt_while_jnot();
int stmnt_while_jback();
int stmnt_call();
int stmnt_call_param();
int stmnt_call_end();
int cond_odd();
int set_cond_opp();
int cond_write();
int cond_and();
int cond_or();
int enter_param_displ();
