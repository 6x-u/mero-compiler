/**
 * MERO Compiler - IR Builder
 * Converts Python AST to MERO IR for optimization and code generation.
 * Developer: MERO:TG@QP4RM
 */
#ifndef MERO_IR_BUILDER_H
#define MERO_IR_BUILDER_H

#include "ast_nodes.h"
#include "ast_visitor.h"
#include "ir_nodes.h"
#include "symbol_table.h"
#include <memory>

extern "C" {
#include "mero/error.h"
}

namespace mero {

class IRBuilder : public ast::ASTVisitor {
public:
    IRBuilder(SymbolTable& symbols);
    ~IRBuilder();

    std::unique_ptr<ir::IRModule> build(ast::ModuleNode* module);
    bool has_errors() const { return error_ctx_.error_count > 0; }

private:
    SymbolTable& symbols_;
    MeroErrorContext error_ctx_;
    std::unique_ptr<ir::IRModule> module_;
    ir::IRNodeList* current_body_ = nullptr;
    std::vector<ir::IRNodeList*> body_stack_;

    void push_body(ir::IRNodeList* body);
    void pop_body();
    void emit(ir::IRNodePtr node);

    ir::IRNodePtr convert_expr(ast::Node* node);
    void convert_stmt(ast::Node* node);

    ir::IRNodePtr convert_constant(ast::ConstantExpr* node);
    ir::IRNodePtr convert_name(ast::NameExpr* node);
    ir::IRNodePtr convert_binop(ast::BinOpExpr* node);
    ir::IRNodePtr convert_unaryop(ast::UnaryOpExpr* node);
    ir::IRNodePtr convert_boolop(ast::BoolOpExpr* node);
    ir::IRNodePtr convert_compare(ast::CompareExpr* node);
    ir::IRNodePtr convert_call(ast::CallExpr* node);
    ir::IRNodePtr convert_attribute(ast::AttributeExpr* node);
    ir::IRNodePtr convert_subscript(ast::SubscriptExpr* node);
    ir::IRNodePtr convert_list(ast::ListExpr* node);
    ir::IRNodePtr convert_tuple(ast::TupleExpr* node);
    ir::IRNodePtr convert_dict(ast::DictExpr* node);
    ir::IRNodePtr convert_set(ast::SetExpr* node);
    ir::IRNodePtr convert_ifexpr(ast::IfExpr* node);
    ir::IRNodePtr convert_lambda(ast::LambdaExpr* node);
    ir::IRNodePtr convert_listcomp(ast::ListCompExpr* node);
    ir::IRNodePtr convert_joinedstr(ast::JoinedStrExpr* node);
    ir::IRNodePtr convert_slice(ast::SliceExpr* node);
    ir::IRNodePtr convert_starred(ast::StarredExpr* node);

    void convert_function(ast::FunctionDef* node);
    void convert_class(ast::ClassDef* node);
    void convert_if(ast::IfStmt* node);
    void convert_while(ast::WhileStmt* node);
    void convert_for(ast::ForStmt* node);
    void convert_try(ast::TryStmt* node);
    void convert_with(ast::WithStmt* node);
    void convert_assign(ast::AssignStmt* node);
    void convert_augassign(ast::AugAssignStmt* node);
    void convert_return(ast::ReturnStmt* node);
    void convert_import(ast::ImportStmt* node);
    void convert_importfrom(ast::ImportFromStmt* node);
    void convert_raise(ast::RaiseStmt* node);
    void convert_delete(ast::DeleteStmt* node);
    void convert_assert(ast::AssertStmt* node);
    void convert_global(ast::GlobalStmt* node);

    ir::OpCode map_binop(TokenType op);
    ir::OpCode map_cmpop(ast::CmpOp op);
    ir::OpCode map_augop(TokenType op);
    std::vector<ir::IRParam> convert_params(const std::vector<ast::Param>& params);
};

} // namespace mero

#endif // MERO_IR_BUILDER_H
