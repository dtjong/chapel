#ifndef _SYMBOL_H_
#define _SYMBOL_H_

#include "baseAST.h"
#include "analysis.h"
#include "pragma.h"
#include "type.h"

class Stmt;
class ASymbol;
class SymScope;
class MPosition;

enum varType {
//R:Maybe this should be VAR_NOTHING
  VAR_NORMAL,
  VAR_CONFIG,
  VAR_STATE
};

enum consType {
  //and this VAR_NOTMAL 
  VAR_VAR,
  VAR_CONST,
  VAR_PARAM
};

class DefExpr;

class Symbol : public BaseAST {
 public:
  char* name;
  char* cname;   /* Name of symbol for generating C code */
  Type* type;
  bool exportMe;
  bool isDead;
  DefExpr* defPoint; /* Point of definition */
  Pragma *pragmas;

  //the scope in which the symbol was defined
  SymScope* parentScope;
  ASymbol *asymbol;

  Symbol(astType_t astType, char* init_name, Type* init_type = dtUnknown,
         bool init_exportMe = false);
  void setParentScope(SymScope* init_parentScope);

  Symbol* copyList(bool clone = false, Map<BaseAST*,BaseAST*>* map = NULL, CloneCallback* analysis_clone = NULL);
  Symbol* copy(bool clone = false, Map<BaseAST*,BaseAST*>* map = NULL, CloneCallback* analysis_clone = NULL);
  virtual Symbol* copySymbol(bool clone, Map<BaseAST*,BaseAST*>* map, CloneCallback* analysis_clone);

  virtual void traverse(Traversal* traversal, bool atTop = true);
  virtual void traverseDef(Traversal* traversal, bool atTop = true);
  virtual void traverseSymbol(Traversal* traverse);
  virtual void traverseDefSymbol(Traversal* traverse);

  virtual bool isConst(void);
  virtual bool isParam(void);
  bool isThis(void);

  void print(FILE* outfile);
  virtual void printDef(FILE* outfile);
  void printDefList(FILE* outfile, char* separator);
  virtual void codegen(FILE* outfile);
  virtual void codegenDef(FILE* outfile);
  virtual void codegenPrototype(FILE* outfile);
  void codegenDefList(FILE* outfile, char* separator);
  void setDefPoint(DefExpr* init_defPoint);
};
#define forv_Symbol(_p, _v) forv_Vec(Symbol, _p, _v)


class UnresolvedSymbol : public Symbol {
 public:
  UnresolvedSymbol(char* init_name, char* init_cname = NULL);
  virtual Symbol* copySymbol(bool clone, Map<BaseAST*,BaseAST*>* map, CloneCallback* analysis_clone);

  virtual void traverseDefSymbol(Traversal* traverse);

  void codegen(FILE* outfile);
};


class VarSymbol : public Symbol {
 public:
  varType varClass;
  /*bool isConstant;
  bool isParameter;*/
  //Replaced with
  consType consClass;
  Expr* init;
  //changed isconstant flag to reflect var, const, param: 0, 1, 2
  VarSymbol(char* init_name, Type* init_type = dtUnknown,
            Expr* init_expr = NULL,
            varType init_varClass = VAR_NORMAL, consType init_consClass = VAR_VAR);
            
  virtual Symbol* copySymbol(bool clone, Map<BaseAST*,BaseAST*>* map, CloneCallback* analysis_clone);

  virtual void traverseDefSymbol(Traversal* traverse);

  bool initializable(void);
  bool isConst(void);
  //Roxana
  bool isParam(void);

  virtual void codegenDef(FILE* outfile);

  void printDef(FILE* outfile);
};


class ParamSymbol : public Symbol {
 public:
  paramType intent;
  Expr* init;

  ParamSymbol(paramType init_intent, char* init_name, 
              Type* init_type = dtUnknown, Expr* init_init = NULL);
  virtual Symbol* copySymbol(bool clone, Map<BaseAST*,BaseAST*>* map, 
                             CloneCallback* analysis_clone);

  virtual void traverseDefSymbol(Traversal* traverse);

  bool requiresCPtr(void);
  bool requiresCopyBack(void);
  bool requiresCTmp(void);
  bool isConst(void);

  void printDef(FILE* outfile);
  void codegen(FILE* outfile);
  void codegenDef(FILE* outfile);
};


class TypeSymbol : public Symbol {
 public:
  TypeSymbol(char* init_name, Type* init_definition);
  virtual Symbol* copySymbol(bool clone, Map<BaseAST*,BaseAST*>* map, CloneCallback* analysis_clone);
  TypeSymbol* clone(CloneCallback* clone_callback, Map<BaseAST*,BaseAST*>* map);
  virtual void traverseDefSymbol(Traversal* traverse);
  virtual void codegenDef(FILE* outfile);
  virtual void codegenPrototype(FILE* outfile);
};


typedef enum __method_type {
  NON_METHOD,
  PRIMARY_METHOD,
  SECONDARY_METHOD
} _method_type;

class FnSymbol : public Symbol {
 public:
  Symbol* formals;
  Type* retType;
  Symbol* _this;
  VarSymbol* _setter; /* the variable this function sets if it is a setter */
  VarSymbol* _getter; /* the variable this function gets if it is a getter */
  Stmt* body;
  Symbol* classBinding;
  _method_type method_type;
  SymScope* paramScope;
  bool isConstructor;

  FnSymbol* overload;

  FnSymbol(char* init_name, Symbol* init_formals, Type* init_retType,
           Stmt* init_body, bool init_exportMe=false,
           Symbol* init_classBinding = NULL);
  FnSymbol(char* init_name, Symbol* init_classBinding = NULL);
  void finishDef(Symbol* init_formals, Type* init_retType, Stmt* init_body,
                 SymScope* init_paramScope, bool init_exportMe=false);
  virtual Symbol* copySymbol(bool clone, Map<BaseAST*,BaseAST*>* map, CloneCallback* analysis_clone);
  virtual void traverseDefSymbol(Traversal* traverse);

  FnSymbol* clone(CloneCallback* clone_callback, Map<BaseAST*,BaseAST*>* map);
  FnSymbol* order_wrapper(Map<MPosition *, MPosition *> *formals_to_actuals);
  FnSymbol* coercion_wrapper(Map<MPosition *, Symbol *> *coercion_substitutions);
  FnSymbol* default_wrapper(Vec<MPosition *> *defaults);
  FnSymbol* instantiate_generic(Map<Type *, Type *> *generic_substitutions);

  void codegenHeader(FILE* outfile);
  void codegenDef(FILE* outfile);

  void printDef(FILE* outfile);

  static FnSymbol* mainFn;
  static void init(void);
};


class EnumSymbol : public Symbol {
 public:
  Expr* init;
  int val;

  EnumSymbol(char* init_name, Expr* init_init, int init_val = 0);
  virtual Symbol* copySymbol(bool clone, Map<BaseAST*,BaseAST*>* map, CloneCallback* analysis_clone);
  virtual void traverseDefSymbol(Traversal* traverse);
  void set_values(void);
  void codegenDef(FILE* outfile);
};


class ModuleSymbol : public Symbol {
 public:
  bool internal;
  Stmt* stmts;
  FnSymbol* initFn;

  SymScope* modScope;

  ModuleSymbol(char* init_name, bool init_internal);
  void setModScope(SymScope* init_modScope);
  virtual void traverseDefSymbol(Traversal* traverse);
  void startTraversal(Traversal* traversal);

  void codegenDef(void);
  void createInitFn(void);
  bool isFileModule(void);
};


class LabelSymbol : public Symbol {
 public:
  LabelSymbol(char* init_name);
  virtual void codegenDef(FILE* outfile);
};


#endif
