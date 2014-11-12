
#include <stdio.h>
#include "semantic.h"
#include "table.h"
#include "type.h"
#include "translate.h"

static TAB_table _tabela;

typedef struct expr_type_s expr_type_t;

struct expr_type_s{
//   tr_expr_t expr;
   Ty_ty type;
};

static expr_type_t expr_type(/*tr_expr_t expr,*/ Ty_ty type){

   expr_type_t result;
//   result.expr = expr;
   result.type = type;
   return result;
}

static expr_type_t trans_expr(tr_level_t level, A_exp expr );

static expr_type_t trans_simple_var(tr_level_t level, A_var var){

   Ty_ty entry = TAB_look(_tabela, var->u.simple);
   
   if (!entry){
      EM_error(var->pos, "variavel indefinido: %s", S_name(var->u.simple));
      return expr_type(/*tr_num_expr(0),*/ Ty_Int());
   } 
   else if (entry->kind != Ty_var){
      EM_error(var->pos, "expected '%s' to be a variable, not a function", S_name(var->u.simple));
      return expr_type(/*tr_num_expr(0),*/ Ty_Int());
   }

   return expr_type(/*tr_simple_var(tr_alloc_local(level, 0), level),*/ Ty_Name(var->u.simple, Ty_Void()));
}

static expr_type_t trans_field_var( tr_level_t level, A_var var );
static expr_type_t trans_sub_var( tr_level_t level, A_var var );

static expr_type_t trans_var(tr_level_t level, A_var var){

   switch( var->kind ){

      case A_simpleVar:
         return trans_simple_var( level, var );
/*      case A_fieldVar:
         return trans_field_var( level, var );
      case A_subscriptVar:
         return trans_sub_var( level, var );
*/
   }
}

static expr_type_t trans_assign_expr(tr_level_t level, A_exp expr){

   expr_type_t var = trans_var(level, expr->u.assign.var);
   expr_type_t et  = trans_expr(level, expr->u.assign.exp);

   if (et.type == var.type/*!ty_match(var.type, et.type)*/)
      EM_error(expr->pos, "tipos incompativeis");

   if (expr->u.assign.var->kind == A_simpleVar /*&& var.type->kind == Ty_Int()*/){
      A_var v = expr->u.assign.var;
      A_exp entry = TAB_look(_tabela, v->u.simple);

//      if (entry && entry->kind == A_assignExp && entry->u.forr)
//         EM_error(expr->pos, "assigning to the for variable");
   }
   return expr_type(/*tr_assign_expr(var.expr, et.expr),*/ Ty_Void());
}

static void trans_var_decl(tr_level_t level, A_dec decl){

   expr_type_t init = trans_expr(level, decl->u.var.init);

   Ty_ty type = init.type;

   if (decl->u.var.typ){
   
      type = TAB_look(_tabela, decl->u.var.typ); 

      if (!type)
         type = Ty_Int();

      if (type != init.type)
         EM_error(decl->pos, "initializer has incorrect type");

   }else if (init.type->kind == Ty_nil)
      EM_error(decl->pos, "don't know which record type to take");

    else if (init.type->kind == Ty_void)
      EM_error(decl->pos, "can't assign void value to a variable");

   TAB_enter(_tabela, decl->u.var.var, Ty_Var());

}

static void trans_decl(tr_level_t level, A_dec decl){

   switch(decl->kind){

      case A_varDec:
         printf("\n");
         trans_var_decl(level, decl);
         break;
/*
      case A_typeDec:
         trans_types_decl(level, decl);

      case A_functionDec:
         trans_funcs_decl(level, decl);
*/
   }
}

static expr_type_t trans_let_expr(tr_level_t level, A_exp expr){

   expr_type_t result;
   A_decList p;
   S_beginScope(_tabela);
   //S_beginScope(_tenv);
   for (p = expr->u.let.decs; p; p = p->tail)
      trans_decl(level, p->head);
   result = trans_expr(level, expr->u.let.body);
   S_endScope(_tabela);
   //S_endScope(_tenv);
   return result;
}

static expr_type_t trans_seq_expr(tr_level_t level, A_exp expr){
   
   A_expList p;
   A_expList stmts = NULL, next = NULL;
   for (p = expr->u.seq; p; p = p->tail){
      expr_type_t et = trans_expr(level, p->head);
/*      if (stmts)
         next = next->next = list(et.expr, NULL);
      else
         stmts = next = list(et.expr, NULL);
*/
      if (!p->tail)
         return expr_type(/*tr_seq_expr(stmts),*/ et.type);
   }
   return expr_type(/*tr_num_expr(0),*/ Ty_Void());

}

static expr_type_t trans_expr(tr_level_t level, A_exp exp){
   
   switch( exp->kind ){

      case A_varExp:
         return trans_var(level, exp->u.var);
/*
      case A_nilExp:
	 return trans_nil_expr(level, exp);

      case A_intExp:
         return trans_num_expr(level, exp);

      case A_stringExp:
         return trans_string_expr(level, exp);

      case A_callExp:
         return trans_call_expr(level, exp);

      case A_opExp:
         return trans_op_expr(level, exp);

      case A_recordExp:
         return trans_record_expr(level, exp);
*/
      case A_seqExp:
         return trans_seq_expr(level, exp);

      case A_assignExp:
         return trans_assign_expr(level, exp);
/*
      case A_ifExp:
         return trans_if_expr(level, exp);

      case A_whileExp:
         return trans_while_expr(level, exp);

      case A_forExp:
         return trans_for_expr(level, exp);

      case A_breakExp:
         return trans_break_expr(level, exp);
*/
      case A_letExp:
         return trans_let_expr(level, exp);
/*
      case A_arrayExp:
         return trans_array_expr(level, exp);
*/
   }
}
/*
static Ty_ty trans_name_type(ast_type_t type){

   Ty_ty t = sym_lookup(_tenv, type->u.name);
   if (!t){
      em_error( type->pos, "undefined type '%s'", sym_name( type->u.name ) );
      t = ty_int();
   }
   return t;
}
*/
void analizador_semantico(A_exp prog){

   _tabela = TAB_empty();
//   _tenv = env_base_tenv();
   trans_expr(tr_outermost(), prog);
}
