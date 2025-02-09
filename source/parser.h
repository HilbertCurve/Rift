/* date = October 8th 2021 2:09 pm */

#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "defines.h"
#include "str.h"
#include "types.h"

typedef u32 P_Precedence;
enum {
    Prec_None,
    Prec_Assignment,  // =
    Prec_LogOr,       // ||
    Prec_LogAnd,      // &&
    Prec_BitOr,       // |
    Prec_BitXor,      // ^
    Prec_BitAnd,      // &
    Prec_Equality,    // == !=
    Prec_Comparison,  // < > <= >=
    Prec_Term,        // + -
    Prec_Factor,      // * /
    Prec_Unary,       // ! - *(deref) &(addr) ~ ++ --
    Prec_Call,        // . () []
    Prec_Primary
};

typedef u32 P_ValueTypeCollection;
enum {
    ValueTypeCollection_Number,
    ValueTypeCollection_Pointer,
    ValueTypeCollection_WholeNumber,
    ValueTypeCollection_DecimalNumber,
    ValueTypeCollection_Cstring,
    ValueTypeCollection_Char,
    ValueTypeCollection_Bool,
};

#include "bytecode.h"
#include "data_structures.h"

typedef u32 P_ExprType;
enum {
    ExprType_IntLit, ExprType_LongLit, ExprType_FloatLit, ExprType_DoubleLit,
    ExprType_CstringLit, ExprType_CharLit, ExprType_BoolLit, ExprType_Typename,
    ExprType_Funcname, ExprType_Unary, ExprType_Binary, ExprType_Assignment,
    ExprType_Variable, ExprType_FuncCall, ExprType_Dot, ExprType_EnumDot,
    ExprType_Cast, ExprType_Index, ExprType_Addr, ExprType_Deref,
    ExprType_Nullptr, ExprType_ArrayLit, ExprType_Lambda, ExprType_Call,
    ExprType_Sizeof, ExprType_Offsetof, ExprType_Namespacename, ExprType_Arrow,
    ExprType_CompoundLit, ExprType_Increment, ExprType_Decrement,
};

struct P_Stmt;
typedef struct P_Namespace P_Namespace;
typedef struct P_Expr P_Expr;
struct P_Expr {
    P_ExprType type;
    P_ValueType ret_type;
    b8 can_assign;
    b8 is_constant; // TODO(voxel): convert to bitfield maybe
    union {
        i32    integer_lit;
        i64    long_lit;
        f32    float_lit;
        f64    double_lit;
        b8     bool_lit;
        string char_lit; // Is a string for transpiling reasons. :(
        string cstring_lit;
        P_ValueType typename;
        P_Namespace* namespace;
        string lambda;
        P_Expr* cast;
        P_Expr* addr;
        P_Expr* deref;
        expr_array array;
        string variable;
        P_Expr* sizeof_e;
        struct { string name; P_Namespace* namespace; } funcname;
        struct { P_Expr* typename; string member_name; } offsetof_e;
        struct { L_TokenType operator; P_Expr* left; P_Expr* right; } binary;
        struct { L_TokenType operator; P_Expr* operand; } unary;
        struct { P_Expr* left; } increment;
        struct { P_Expr* left; } decrement;
        struct { P_Expr* left; string right; } dot;
        struct { P_Expr* left; string right; } arrow;
        struct { P_Expr* operand; P_Expr* index; } index;
        struct { string left; string right; b8 native; } enum_dot;
        struct { L_TokenType operator; P_Expr* name; P_Expr* value; } assignment;
        struct { string name; P_Expr** params; u32 call_arity; } func_call;
        struct { string_list names; P_Expr** values; u32 val_count; b8 should_cast; } compound_lit;
        struct { P_Expr* left; P_Expr** params; u32 call_arity; } call;
    } op;
};

typedef u32 P_StmtType;
enum {
    StmtType_Nothing, // Used Successful parsing but no code emission e.g. Inner scope functions
    StmtType_Expression, StmtType_Block, StmtType_Return, StmtType_If,
    StmtType_IfElse, StmtType_While, StmtType_DoWhile, 
    StmtType_VarDecl, StmtType_VarDeclAssign, StmtType_FlagEnumDecl,
    StmtType_FuncDecl, StmtType_StructDecl, StmtType_UnionDecl,
    StmtType_EnumDecl, StmtType_For, StmtType_Break, StmtType_Continue,
    StmtType_Switch, StmtType_Match, StmtType_Case, StmtType_MatchCase,
    StmtType_Default, StmtType_MatchDefault
};

typedef struct P_Stmt P_Stmt;
struct P_Stmt {
    P_StmtType type;
    P_Stmt* next;
    union {
        P_Expr* expression;
        P_Expr* returned;
        P_Stmt* block;
        
        struct { P_ValueType type; string name; } var_decl;
        struct { P_ValueType type; string name; P_Expr* val; } var_decl_assign;
        struct { string name; u32 arity; value_type_list param_types; string_list param_names; P_ValueType type; P_Stmt* block; b8 varargs; } func_decl;
        struct { string name; u32 member_count; value_type_list member_types; string_list member_names; } struct_decl;
        struct { string name; u32 member_count; value_type_list member_types; string_list member_names; } union_decl;
        struct { string name; u32 member_count; string_list member_names; P_Expr** member_values; } enum_decl;
        struct { string name; u32 member_count; string_list member_names; P_Expr** member_values; } flagenum_decl;
        
        struct { P_Expr* switched;  P_Stmt* then; } switch_s;
        struct { P_Expr* matched;   P_Stmt* then; } match_s;
        struct { P_Expr* value;     P_Stmt* then; } case_s;
        struct { P_Expr* value;     P_Stmt* then; } mcase_s;
        struct {                    P_Stmt* then; } default_s;
        struct {                    P_Stmt* then; } mdefault_s;
        struct { P_Expr* condition; P_Stmt* then; } if_s;
        struct { P_Expr* condition; P_Stmt* then; P_Stmt* else_s; } if_else;
        struct { P_Expr* condition; P_Stmt* then; } while_s;
        struct { P_Expr* condition; P_Stmt* then; } do_while;
        struct { P_Expr* condition; P_Stmt* then; P_Stmt* init; P_Expr* loopexec; } for_s;
    } op;
};

typedef u32 P_PreStmtType;
enum {
    PreStmtType_Nothing,
    PreStmtType_ForwardDecl,
    PreStmtType_StructForwardDecl,
    PreStmtType_UnionForwardDecl,
    PreStmtType_EnumForwardDecl,
    PreStmtType_FlagEnumForwardDecl,
    PreStmtType_CInclude,
    PreStmtType_CInsert,
};

typedef struct P_PreStmt P_PreStmt;
struct P_PreStmt {
    P_PreStmtType type;
    P_PreStmt* next;
    union {
        struct { string name; u32 arity; value_type_list param_types; string_list param_names; P_ValueType type; } forward_decl;
        string cinclude;
        string cinsert;
        string struct_fd;
        string union_fd;
        string enum_fd;
        string flagenum_fd;
    } op;
};


typedef struct P_Namespace {
    string unitname;
    string flatname;
    var_hash_table variables;
    func_hash_table functions;
    type_array types;
    namespace_array subspaces;
} P_Namespace;

typedef u32 P_ScopeType;
enum {
    ScopeType_None,
    ScopeType_Lambda,
    ScopeType_For,
    ScopeType_While,
    ScopeType_DoWhile,
    ScopeType_If,
    ScopeType_Else,
    ScopeType_Switch,
    ScopeType_Match,
    ScopeType_Case,
    ScopeType_Default,
    ScopeType_Enum,
};

typedef struct P_ScopeContext {
    P_ValueType prev_function_body_ret;
    b8 prev_directly_in_func_body;
    b8 prev_was_in_private_scope;
    struct P_ScopeContext* prev_scope_ctx;
    u32 usings_pop;
} P_ScopeContext;

typedef struct P_ParserSnap {
    L_Lexer lexer;
    
    L_Token next_two;
    L_Token next;
    L_Token current;
    L_Token previous;
    L_Token previous_two;
    
    u32 scope_depth;
    
    b8 had_error;
    b8 panik_mode;
} P_ParserSnap;

struct P_Parser;
P_ParserSnap P_TakeSnapshot(struct P_Parser* parser);
void         P_ApplySnapshot(struct P_Parser* parser, P_ParserSnap snap);

typedef struct P_Parser {
    M_Arena arena;
    L_Lexer lexer;
    string source;
    string filename;
    string abspath;
    b8 initialized;
    
    P_PreStmt* pre_root;
    P_PreStmt* pre_end;
    P_Stmt*    root;
    P_Stmt*    end;
    
    P_Stmt* lambda_functions_start;
    P_Stmt* lambda_functions_curr;
    P_ScopeContext* curr_scope_ctx;
    
    L_Token next_two;
    L_Token next;
    L_Token current;
    L_Token previous;
    L_Token previous_two;
    
    u32 scope_depth;
    
    b8 had_error;
    b8 panik_mode;
    
    P_Namespace* current_namespace;
    using_stack usings;
    
    P_ScopeType* scopetype_stack;
    u32 scopetype_tos;
    string current_function;
    
    // TODO(voxel): Refactor into bitfield maybe
    b8 block_stmt_should_begin_scope;
    b8 is_directly_in_func_body;
    b8 all_code_paths_return;
    P_ValueType function_body_ret;
    P_ValueType switch_type;
    b8 is_in_private_scope; // This is temporary until I come up with a solution for closures
    b8 encountered_return;
    b8 encountered_default;
    u32 lambda_number;
    u32 import_number;
    
    struct P_Parser* parent;
    // Dynamic array of Parsers for imports and multiple files. Is this a bad idea?
    struct P_Parser** sub;
    u32 sub_cap;
    u32 sub_count;
} P_Parser;

void P_GlobalInit(string_list tags);
void P_GlobalFree();

// Heirarchy stuff
P_Parser* P_AddChild(P_Parser* parent, string source, string filename);

// API
void P_Advance(P_Parser* parser);
void P_Initialize(P_Parser* parser, string source, string filename, b8 is_root);
void P_PreParse(P_Parser* parser);
void P_Parse(P_Parser* parser);
void P_Free(P_Parser* parser);

#endif //PARSER_H
