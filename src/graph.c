#include <stdio.h>
#include "graph.h"
#include "graph_funcs.h"

static struct Edge g_expr[];
static struct Edge g_statement[];

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
/*1*/ {EtGr, {(ul)g_expr},  cond_odd, 10,  0},
/*2*/ {EtGr, {(ul)g_expr},  NULL, 3,  0},
/*3*/ {EtSy, {(ul)'='},     set_cond_opp, 9,  4},
/*4*/ {EtSy, {(ul)'#'},     set_cond_opp, 9,  5},
/*5*/ {EtSy, {(ul)'<'},     set_cond_opp, 9,  6},
/*6*/ {EtSy, {(ul)'>'},     set_cond_opp, 9,  7},
/*7*/ {EtSy, {(ul)tle},     set_cond_opp, 9,  8},
/*8*/ {EtSy, {(ul)tge},     set_cond_opp, 9,  0},
/*9*/ {EtGr, {(ul)g_expr},  cond_write, 10,  0},
/*10*/ {EtEn, {(ul)0},       NULL, 0,  0}
};

static struct Edge g_statement[] = {
// a := b+c
/*0*/ {EtMo, {(ul)mcIdent},    stmnt_assign, 1,  3},
/*1*/ {EtSy, {(ul)tErg},       NULL, 2,  0},
/*2*/ {EtGr, {(ul)g_expr},     stmnt_store, 21,  0},
// if
/*3*/ {EtSy, {(ul)tIF},        NULL, 4,  7},
/*4*/ {EtGr, {(ul)g_cond},     stmnt_if_jnot, 5,  0},
/*5*/ {EtSy, {(ul)tTHN},       NULL, 6,  0},
/*6*/ {EtGr, {(ul)g_statement},stmnt_if_jaddr, 21,  0},
// while
/*7*/ {EtSy, {(ul)tWHL},       stmnt_while_jback, 8,  11},
/*8*/ {EtGr, {(ul)g_cond},     stmnt_while_jnot, 9,  0},
/*9*/ {EtSy, {(ul)tDO},        NULL, 10, 0},
/*10*/ {EtGr, {(ul)g_statement},stmnt_while_jaddr, 21,  0},
// begin
/*11*/{EtSy, {(ul)tBGN},       NULL, 12, 15},
/*12*/{EtGr, {(ul)g_statement},NULL, 13, 0},
/*13*/{EtSy, {(ul)';'},        NULL, 12,  14},
/*14*/{EtSy, {(ul)tEND},       NULL, 21, 0},
// call
/*15*/{EtSy, {(ul)tCLL},       NULL, 16, 17},
/*16*/{EtMo, {(ul)mcIdent},    stmnt_call, 21,  0},
// ?
/*17*/{EtSy, {(ul)'?'},        NULL, 18, 19},
/*18*/{EtMo, {(ul)mcIdent},    stmnt_get, 21,  0},
// !
/*19*/{EtSy, {(ul)'!'},        NULL, 20, 21},
/*20*/{EtGr, {(ul)g_expr},     st_putval, 21,  0},

/*21*/{EtEn, {(ul)0},          NULL, 0,  0} 
};

struct Edge g_prog[] = {
/*0*/ {EtGr, {(ul)g_block},NULL, 1, 0},
/*1*/ {EtSy, {(ul)'.'},    end_prog, 2, 0},
/*2*/ {EtEn, {(ul)0},      NULL, 0, 0}
};

static struct Edge g_fact[] = {
/*0*/ {EtMo, {(ul)mcIdent}, fac_ident, 5, 1},
/*1*/ {EtMo, {(ul)mcNum},   fac_num, 5, 2},
/*2*/ {EtSy, {(ul)'('},     NULL, 3, 0},
/*3*/ {EtGr, {(ul)g_expr},  NULL, 4, 0},
/*4*/ {EtSy, {(ul)')'},     NULL, 5, 0},
/*5*/ {EtEn, {(ul)0},       NULL, 0, 0}
};
static struct Edge g_term[] = {
/*0*/ {EtGr, {(ul)g_fact}, NULL, 1, 0},
/*1*/ {EtSy, {(ul)'*'},    NULL, 2, 3},
/*2*/ {EtGr, {(ul)g_fact}, term_mult, 1, 0},
/*3*/ {EtSy, {(ul)'/'},    NULL, 4, 5},
/*4*/ {EtGr, {(ul)g_fact}, term_div, 1, 0},
/*5*/ {EtEn, {(ul)0},      NULL, 0, 0}
};

static struct Edge g_expr[] = {
/*0*/ {EtSy, {(ul)'-'},    NULL, 1, 2}, 
/*1*/ {EtGr, {(ul)g_term}, expr_vz, 3, 0},
/*2*/ {EtGr, {(ul)g_term}, NULL, 3, 0},
/*3*/ {EtSy, {(ul)'+'},    NULL, 4, 5},
/*4*/ {EtGr, {(ul)g_term}, expr_add, 3, 0},
/*5*/ {EtSy, {(ul)'-'},    NULL, 6, 7}, 
/*6*/ {EtGr, {(ul)g_term}, expr_sub, 3, 0},
/*7*/ {EtEn, {(ul)0},      NULL, 0, 0}
};
