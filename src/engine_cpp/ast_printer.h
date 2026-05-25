/**
 * MERO Compiler - AST Pretty Printer
 * Debug visualization of AST structure.
 * Developer: MERO:TG@QP4RM
 */
#ifndef MERO_AST_PRINTER_H
#define MERO_AST_PRINTER_H

#include "ast_nodes.h"
#include "ast_visitor.h"
#include <string>
#include <sstream>

namespace mero {

class ASTPrinter : public ast::ASTVisitor {
public:
    std::string print(ast::Node* node);

protected:
    void visit_module(ast::ModuleNode* node) override;
    void visit_function_def(ast::FunctionDef* node) override;
    void visit_class_def(ast::ClassDef* node) override;
    void visit_assign(ast::AssignStmt* node) override;
    void visit_return(ast::ReturnStmt* node) override;
    void visit_if(ast::IfStmt* node) override;
    void visit_while(ast::WhileStmt* node) override;
    void visit_for(ast::ForStmt* node) override;
    void visit_bin_op(ast::BinOpExpr* node) override;
    void visit_unary_op(ast::UnaryOpExpr* node) override;
    void visit_call(ast::CallExpr* node) override;
    void visit_constant(ast::ConstantExpr* node) override;
    void visit_name(ast::NameExpr* node) override;
    void visit_attribute(ast::AttributeExpr* node) override;
    void visit_list(ast::ListExpr* node) override;
    void visit_dict(ast::DictExpr* node) override;
    void visit_expr_stmt(ast::ExprStmt* node) override;

private:
    std::ostringstream out_;
    int indent_ = 0;

    void emit(const std::string& text);
    void indent();
    void dedent();
};

} // namespace mero

#endif // MERO_AST_PRINTER_H
