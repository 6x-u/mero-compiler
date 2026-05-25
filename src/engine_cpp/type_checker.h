/**
 * MERO Compiler - Type Checker
 * Static type inference and checking for Python code.
 * Developer: MERO:TG@QP4RM
 */
#ifndef MERO_TYPE_CHECKER_H
#define MERO_TYPE_CHECKER_H

#include "ast_nodes.h"
#include "ast_visitor.h"
#include "symbol_table.h"
#include "ir_nodes.h"
#include <unordered_map>
#include <string>

namespace mero {

class TypeChecker : public ast::ASTVisitor {
public:
    explicit TypeChecker(SymbolTable& symbols);

    void check(ast::ModuleNode* module);
    ir::TypeTag infer_type(ast::Node* node);
    bool has_warnings() const { return !warnings_.empty(); }
    const std::vector<std::string>& warnings() const { return warnings_; }

protected:
    void visit_assign(ast::AssignStmt* node) override;
    void visit_function_def(ast::FunctionDef* node) override;
    void visit_return(ast::ReturnStmt* node) override;
    void visit_bin_op(ast::BinOpExpr* node) override;
    void visit_call(ast::CallExpr* node) override;

private:
    SymbolTable& symbols_;
    std::vector<std::string> warnings_;
    std::unordered_map<std::string, ir::TypeTag> var_types_;
    ir::TypeTag current_return_type_ = ir::TypeTag::Any;

    ir::TypeTag infer_binop_type(ir::TypeTag left, ir::TypeTag right, TokenType op);
    ir::TypeTag infer_call_type(const std::string& func_name);
    ir::TypeTag type_from_annotation(ast::Node* annotation);
    void record_type(const std::string& name, ir::TypeTag type);
    void warn(const std::string& msg, uint32_t line);
};

} // namespace mero

#endif // MERO_TYPE_CHECKER_H
