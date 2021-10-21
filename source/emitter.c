#include "emitter.h"
#include <stdarg.h>

static void E_Write(E_Emitter* emitter, const char* text) {
    fprintf(emitter->output_file, text);
}

static void E_WriteLine(E_Emitter* emitter, const char* text) {
    fprintf(emitter->output_file, text);
    fprintf(emitter->output_file, "\n");
}

static void E_WriteF(E_Emitter* emitter, const char* text, ...) {
    va_list args;
    va_start(args, text);
    vfprintf(emitter->output_file, text, args);
    va_end(args);
}

static void E_WriteLineF(E_Emitter* emitter, const char* text, ...) {
    va_list args;
    va_start(args, text);
    vfprintf(emitter->output_file, text, args);
    fprintf(emitter->output_file, "\n");
    va_end(args);
}

static void E_BeginEmitting(E_Emitter* emitter) {
    E_WriteLine(emitter, "#include <stdio.h>");
    E_WriteLine(emitter, "#include <stdbool.h>");
    E_WriteLine(emitter, "typedef const char* string;");
    E_WriteLine(emitter, "");
}

static void E_FinishEmitting(E_Emitter* emitter) {}

static void E_EmitStatementChain(E_Emitter* emitter, P_Stmt* stmts, u32 indent);

static void E_EmitExpression(E_Emitter* emitter, P_Expr* expr) {
    switch (expr->type) {
        case ExprType_IntLit: {
            E_WriteF(emitter, "%d", expr->op.integer_lit);
        } break;
        
        case ExprType_LongLit: {
            E_WriteF(emitter, "%I64d", expr->op.long_lit);
        } break;
        
        case ExprType_FloatLit: {
            E_WriteF(emitter, "%f", expr->op.float_lit);
        } break;
        
        case ExprType_DoubleLit: {
            E_WriteF(emitter, "%f", expr->op.double_lit);
        } break;
        
        case ExprType_BoolLit: {
            E_WriteF(emitter, "%s", expr->op.bool_lit ? "true" : "false");
        } break;
        
        case ExprType_CharLit: {
            E_WriteF(emitter, "%.*s", (int)expr->op.char_lit.size, expr->op.char_lit.str);
        } break;
        
        case ExprType_StringLit: {
            E_WriteF(emitter, "%.*s", (int)expr->op.string_lit.size, expr->op.string_lit.str);
        } break;
        
        case ExprType_Assignment: {
            E_EmitExpression(emitter, expr->op.assignment.name);
            E_WriteF(emitter, " = ");
            E_EmitExpression(emitter, expr->op.assignment.value);
        } break;
        
        case ExprType_Binary: {
            E_Write(emitter, "(");
            E_EmitExpression(emitter, expr->op.binary.left);
            E_WriteF(emitter, "%s", L__get_string_from_type__(expr->op.binary.operator).str);
            E_EmitExpression(emitter, expr->op.binary.right);
            E_Write(emitter, ")");
        } break;
        
        case ExprType_Unary: {
            E_Write(emitter, "(");
            E_WriteF(emitter, "%s", L__get_string_from_type__(expr->op.unary.operator).str);
            E_EmitExpression(emitter, expr->op.unary.operand);
            E_Write(emitter, ")");
        } break;
        
        case ExprType_Variable: {
            E_WriteF(emitter, "%.*s", (int)expr->op.variable.size, expr->op.variable.str);
        } break;
        
        case ExprType_FuncCall: {
            E_WriteF(emitter, "%.*s(", (int)expr->op.func_call.name.size, expr->op.func_call.name.str);
            for (u32 i = 0; i < expr->op.func_call.call_arity; i++) {
                E_EmitExpression(emitter, expr->op.func_call.params[i]);
                if (i != expr->op.func_call.call_arity - 1)
                    E_Write(emitter, ", ");
            }
            E_Write(emitter, ")");
        } break;
        
        case ExprType_Dot: {
            E_EmitExpression(emitter, expr->op.dot.left);
            E_WriteF(emitter, ".%.*s", (int)expr->op.dot.right.size, expr->op.dot.right.str);
        } break;
    }
}

static void E_EmitStatement(E_Emitter* emitter, P_Stmt* stmt, u32 indent) {
    for (int i = 0; i < indent; i++) E_Write(emitter, "\t");
    switch (stmt->type) {
        case StmtType_Expression: {
            E_EmitExpression(emitter, stmt->op.expression);
            E_WriteLine(emitter, ";");
        } break;
        
        case StmtType_Return: {
            E_Write(emitter, "return ");
            E_EmitExpression(emitter, stmt->op.returned);
            E_WriteLine(emitter, ";");
        } break;
        
        case StmtType_Block: {
            E_WriteLine(emitter, "{");
            E_EmitStatementChain(emitter, stmt->op.block, indent + 1);
            E_WriteLine(emitter, "}");
        } break;
        
        case StmtType_VarDecl: {
            E_WriteF(emitter, "%.*s %.*s", (int)stmt->op.var_decl.type.size, stmt->op.var_decl.type.str, (int)stmt->op.var_decl.name.size, stmt->op.var_decl.name.str);
            E_WriteLine(emitter, ";");
        } break;
        
        case StmtType_VarDeclAssign: {
            E_WriteF(emitter, "%.*s %.*s = ", (int)stmt->op.var_decl_assign.type.size, stmt->op.var_decl_assign.type.str, (int)stmt->op.var_decl_assign.name.size, stmt->op.var_decl_assign.name.str);
            E_EmitExpression(emitter, stmt->op.var_decl_assign.val);
            E_WriteLine(emitter, ";");
        } break;
        
        case StmtType_FuncDecl: {
            E_WriteF(emitter, "%.*s %.*s(", (int)stmt->op.func_decl.type.size, stmt->op.func_decl.type.str, (int)stmt->op.func_decl.name.size, stmt->op.func_decl.name.str);
            string_list_node* curr_name = stmt->op.func_decl.param_names.first;
            string_list_node* curr_type = stmt->op.func_decl.param_types.first;
            if (stmt->op.func_decl.arity != 0) {
                for (u32 i = 0; i < stmt->op.func_decl.arity; i++) {
                    E_WriteF(emitter, "%.*s %.*s", curr_type->size, curr_type->str, curr_name->size, curr_name->str);
                    if (i != stmt->op.func_decl.arity - 1)
                        E_Write(emitter, ", ");
                    curr_name = curr_name->next;
                    curr_type = curr_type->next;
                }
            }
            if (stmt->op.func_decl.arity == 0)
                E_Write(emitter, "void");
            E_WriteLine(emitter, ") {");
            E_EmitStatementChain(emitter, stmt->op.func_decl.block, indent + 1);
            E_WriteLine(emitter, "}");
        } break;
        
        case StmtType_StructDecl: {
            E_WriteLineF(emitter, "typedef struct %.*s {", stmt->op.struct_decl.name.size, stmt->op.struct_decl.name.str);
            string_list_node* curr_name = stmt->op.struct_decl.member_names.first;
            string_list_node* curr_type = stmt->op.struct_decl.member_types.first;
            for (u32 i = 0; i < stmt->op.struct_decl.member_count; i++) {
                for (u32 idt = 0; idt < indent + 1; idt++)
                    E_Write(emitter, "\t");
                E_WriteLineF(emitter, "%.*s %.*s;", (int)curr_type->size, curr_type->str, (int)curr_name->size, curr_name->str);
                curr_name = curr_name->next;
                curr_type = curr_type->next;
            }
            E_WriteLineF(emitter, "} %.*s;", (int)stmt->op.struct_decl.name.size, stmt->op.struct_decl.name.str);
        } break;
        
        case StmtType_If: {
            E_Write(emitter, "if (");
            E_EmitExpression(emitter, stmt->op.if_s.condition);
            E_WriteLine(emitter, ")");
            E_EmitStatement(emitter, stmt->op.if_s.then, indent + 1);
        } break;
        
        case StmtType_IfElse: {
            E_Write(emitter, "if (");
            E_EmitExpression(emitter, stmt->op.if_else.condition);
            E_WriteLine(emitter, ")");
            E_EmitStatement(emitter, stmt->op.if_else.then, indent + 1);
            E_WriteLine(emitter, "else");
            E_EmitStatement(emitter, stmt->op.if_else.else_s, indent + 1);
        } break;
        
        case StmtType_While: {
            E_Write(emitter, "while (");
            E_EmitExpression(emitter, stmt->op.while_s.condition);
            E_WriteLine(emitter, ")");
            E_EmitStatement(emitter, stmt->op.while_s.then, indent + 1);
        } break;
        
        case StmtType_DoWhile: {
            E_WriteLine(emitter, "do");
            E_EmitStatement(emitter, stmt->op.do_while.then, indent + 1);
            E_Write(emitter, " while (");
            E_EmitExpression(emitter, stmt->op.while_s.condition);
            E_WriteLine(emitter, ");");
        } break;
    }
}

static void E_EmitStatementChain(E_Emitter* emitter, P_Stmt* stmts, u32 indent) {
    for (P_Stmt* stmt = stmts; stmt != nullptr; stmt = stmt->next) {
        E_EmitStatement(emitter, stmt, indent);
    }
}

static void E_EmitPreStatement(E_Emitter* emitter, P_PreStmt* stmt, u32 indent) {
    for (int i = 0; i < indent; i++) E_Write(emitter, "\t");
    
    switch (stmt->type) {
        case PreStmtType_ForwardDecl: {
            E_WriteF(emitter, "%.*s %.*s(", (int)stmt->op.forward_decl.type.size, stmt->op.forward_decl.type.str, (int)stmt->op.forward_decl.name.size, stmt->op.forward_decl.name.str);
            string_list_node* curr_name = stmt->op.forward_decl.param_names.first;
            string_list_node* curr_type = stmt->op.forward_decl.param_types.first;
            if (stmt->op.forward_decl.arity != 0) {
                for (u32 i = 0; i < stmt->op.forward_decl.arity; i++) {
                    E_WriteF(emitter, "%.*s %.*s", curr_type->size, curr_type->str, curr_name->size, curr_name->str);
                    if (i != stmt->op.forward_decl.arity - 1)
                        E_Write(emitter, ", ");
                    curr_name = curr_name->next;
                    curr_type = curr_type->next;
                }
            }
            if (stmt->op.forward_decl.arity == 0)
                E_Write(emitter, "void");
            E_WriteLine(emitter, ");");
        }
    }
}

static void E_EmitPreStatementChain(E_Emitter* emitter, P_PreStmt* stmts, u32 indent) {
    for (P_PreStmt* stmt = stmts; stmt != nullptr; stmt = stmt->next) {
        E_EmitPreStatement(emitter, stmt, indent);
    }
}

void E_Initialize(E_Emitter* emitter, string source) {
    P_Initialize(&emitter->parser, source);
    emitter->line = 0;
    emitter->output_file = fopen("./generated.c", "w");
}

void E_Emit(E_Emitter* emitter) {
    E_BeginEmitting(emitter);
    P_PreParse(&emitter->parser);
    if (!emitter->parser.had_error)
        E_EmitPreStatementChain(emitter, emitter->parser.pre_root, 0);
    else return;
    
    E_WriteLine(emitter, "");
    
    P_Parse(&emitter->parser);
    if (!emitter->parser.had_error)
        E_EmitStatementChain(emitter, emitter->parser.root, 0);
    E_FinishEmitting(emitter);
}

void E_Free(E_Emitter* emitter) {
    fclose(emitter->output_file);
    P_Free(&emitter->parser);
}
