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
            E_WriteF(emitter, "%s", expr->op.char_lit);
        } break;
        
        case ExprType_StringLit: {
            E_WriteF(emitter, "%s", expr->op.string_lit);
        } break;
        
        case ExprType_Assignment: {
            E_WriteF(emitter, "%.*s=", expr->op.assignment.name.size, expr->op.assignment.name.str);
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
            E_WriteF(emitter, "%.*s", expr->op.variable.size, expr->op.variable.str);
        } break;
        
        case ExprType_FuncCall: {
            E_WriteF(emitter, "%.*s(", expr->op.func_call.name.size, expr->op.func_call.name.str);
            for (u32 i = 0; i < expr->op.func_call.call_arity; i++) {
                E_EmitExpression(emitter, expr->op.func_call.params[i]);
                if (i != expr->op.func_call.call_arity - 1)
                    E_Write(emitter, ", ");
            }
            E_Write(emitter, ")");
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
            E_WriteF(emitter, "%s %.*s", P__get_value_type_name__(stmt->op.var_decl.type), stmt->op.var_decl.name.size, stmt->op.var_decl.name.str);
            E_WriteLine(emitter, ";");
        } break;
        
        case StmtType_FuncDecl: {
            E_WriteF(emitter, "%s %.*s(", P__get_value_type_name__(stmt->op.var_decl.type), stmt->op.func_decl.name.size, stmt->op.func_decl.name.str);
            for (u32 i = 0; i < stmt->op.func_decl.arity; i++) {
                E_WriteF(emitter, "%s %.*s", P__get_value_type_name__(stmt->op.func_decl.param_types[i]), stmt->op.func_decl.param_names[i].size, stmt->op.func_decl.param_names[i].str);
                if (i != stmt->op.func_decl.arity - 1)
                    E_Write(emitter, ", ");
            }
            E_WriteLine(emitter, ") {");
            E_EmitStatementChain(emitter, stmt->op.func_decl.block, indent + 1);
            E_WriteLine(emitter, "}");
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
    }
}

static void E_EmitStatementChain(E_Emitter* emitter, P_Stmt* stmts, u32 indent) {
    for (P_Stmt* stmt = stmts; stmt != nullptr; stmt = stmt->next) {
        E_EmitStatement(emitter, stmt, indent);
    }
}

void E_Initialize(E_Emitter* emitter, string source) {
    P_Initialize(&emitter->parser, source);
    emitter->line = 0;
    emitter->output_file = fopen("./generated.c", "w");
}

void E_Emit(E_Emitter* emitter) {
    E_BeginEmitting(emitter);
    P_Parse(&emitter->parser);
    if (!emitter->parser.had_error)
        E_EmitStatementChain(emitter, emitter->parser.root, 0);
    E_FinishEmitting(emitter);
}

void E_Free(E_Emitter* emitter) {
    fclose(emitter->output_file);
    P_Free(&emitter->parser);
}
