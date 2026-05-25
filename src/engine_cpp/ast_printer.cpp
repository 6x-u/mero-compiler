/**
 * MERO Compiler - AST Pretty Printer Implementation
 * Developer: MERO:TG@QP4RM
 */
#include "ast_printer.h"

namespace mero {

std::string ASTPrinter::print(ast::Node* node) {
    out_.str("");
    out_.clear();
    indent_ = 0;
    visit(node);
    return out_.str();
}

void ASTPrinter::emit(const std::string& text) {
    for (int i = 0; i < indent_; i++) out_ << "  ";
    out_ << text << "\n";
}

void ASTPrinter::indent() { indent_++; }
void ASTPrinter::dedent() { if (indent_ > 0) indent_--; }

void ASTPrinter::visit_module(ast::ModuleNode* node) {
    emit("Module");
    indent();
    visit_body(node->body);
    dedent();
}

void ASTPrinter::visit_function_def(ast::FunctionDef* node) {
    std::string async_str = node->is_async ? "async " : "";
    emit(async_str + "FunctionDef: " + node->name);
    indent();
    emit("params:");
    indent();
    for (auto& p : node->params) {
        std::string param_info = p.name;
        if (p.is_vararg) param_info = "*" + param_info;
        if (p.is_kwarg) param_info = "**" + param_info;
        emit(param_info);
    }
    dedent();
    emit("body:");
    indent();
    visit_body(node->body);
    dedent();
    dedent();
}

void ASTPrinter::visit_class_def(ast::ClassDef* node) {
    emit("ClassDef: " + node->name);
    indent();
    visit_body(node->body);
    dedent();
}

void ASTPrinter::visit_assign(ast::AssignStmt* node) {
    emit("Assign");
    indent();
    emit("targets:");
    indent();
    visit_body(node->targets);
    dedent();
    emit("value:");
    indent();
    visit(node->value.get());
    dedent();
    dedent();
}

void ASTPrinter::visit_return(ast::ReturnStmt* node) {
    emit("Return");
    if (node->value) {
        indent();
        visit(node->value.get());
        dedent();
    }
}

void ASTPrinter::visit_if(ast::IfStmt* node) {
    emit("If");
    indent();
    emit("test:");
    indent();
    visit(node->test.get());
    dedent();
    emit("body:");
    indent();
    visit_body(node->body);
    dedent();
    if (!node->orelse.empty()) {
        emit("else:");
        indent();
        visit_body(node->orelse);
        dedent();
    }
    dedent();
}

void ASTPrinter::visit_while(ast::WhileStmt* node) {
    emit("While");
    indent();
    visit(node->test.get());
    visit_body(node->body);
    dedent();
}

void ASTPrinter::visit_for(ast::ForStmt* node) {
    emit("For");
    indent();
    visit(node->target.get());
    visit(node->iter.get());
    visit_body(node->body);
    dedent();
}

void ASTPrinter::visit_bin_op(ast::BinOpExpr* node) {
    emit("BinOp: " + std::string(token_type_to_string(node->op)));
    indent();
    visit(node->left.get());
    visit(node->right.get());
    dedent();
}

void ASTPrinter::visit_unary_op(ast::UnaryOpExpr* node) {
    emit("UnaryOp");
    indent();
    visit(node->operand.get());
    dedent();
}

void ASTPrinter::visit_call(ast::CallExpr* node) {
    emit("Call");
    indent();
    emit("func:");
    indent();
    visit(node->func.get());
    dedent();
    if (!node->args.empty()) {
        emit("args:");
        indent();
        visit_body(node->args);
        dedent();
    }
    dedent();
}

void ASTPrinter::visit_constant(ast::ConstantExpr* node) {
    emit("Constant: " + node->value);
}

void ASTPrinter::visit_name(ast::NameExpr* node) {
    emit("Name: " + node->id);
}

void ASTPrinter::visit_attribute(ast::AttributeExpr* node) {
    emit("Attribute: ." + node->attr);
    indent();
    visit(node->value.get());
    dedent();
}

void ASTPrinter::visit_list(ast::ListExpr* node) {
    emit("List");
    indent();
    visit_body(node->elts);
    dedent();
}

void ASTPrinter::visit_dict(ast::DictExpr* node) {
    emit("Dict");
    indent();
    for (size_t i = 0; i < node->keys.size(); i++) {
        emit("key:");
        indent(); visit(node->keys[i].get()); dedent();
        emit("value:");
        indent(); visit(node->values[i].get()); dedent();
    }
    dedent();
}

void ASTPrinter::visit_expr_stmt(ast::ExprStmt* node) {
    visit(node->value.get());
}

} // namespace mero
