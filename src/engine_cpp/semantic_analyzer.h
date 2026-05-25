/**
 * MERO Compiler - Semantic Analyzer
 * Performs semantic analysis: scope resolution, type checking, validation.
 * Developer: MERO:TG@QP4RM
 */
#ifndef MERO_SEMANTIC_ANALYZER_H
#define MERO_SEMANTIC_ANALYZER_H

#include "ast_nodes.h"
#include "ast_visitor.h"
#include "symbol_table.h"
#include <vector>
#include <string>

extern "C" {
#include "mero/error.h"
}

namespace mero {

class SemanticAnalyzer : public ast::ASTVisitor {
public:
    SemanticAnalyzer();
    ~SemanticAnalyzer();

    void analyze(ast::ModuleNode* module);
    bool has_errors() const { return error_ctx_.error_count > 0; }
    const MeroErrorContext& errors() const { return error_ctx_; }
    SymbolTable& symbols() { return symbols_; }

protected:
    void visit_module(ast::ModuleNode* node) override;
    void visit_function_def(ast::FunctionDef* node) override;
    void visit_class_def(ast::ClassDef* node) override;
    void visit_assign(ast::AssignStmt* node) override;
    void visit_aug_assign(ast::AugAssignStmt* node) override;
    void visit_name(ast::NameExpr* node) override;
    void visit_for(ast::ForStmt* node) override;
    void visit_while(ast::WhileStmt* node) override;
    void visit_return(ast::ReturnStmt* node) override;
    void visit_global(ast::GlobalStmt* node) override;
    void visit_nonlocal(ast::NonlocalStmt* node) override;
    void visit_import(ast::ImportStmt* node) override;
    void visit_import_from(ast::ImportFromStmt* node) override;
    void visit_call(ast::CallExpr* node) override;

private:
    SymbolTable symbols_;
    MeroErrorContext error_ctx_;
    bool in_function_ = false;
    bool in_class_ = false;
    std::string current_class_;

    void define_variable(const std::string& name, SymbolKind kind, uint32_t line);
    void resolve_name(const std::string& name, uint32_t line);
    void analyze_target(ast::Node* target);
    void report(MeroErrorCode code, uint32_t line, const char* fmt, ...);
};

} // namespace mero

#endif // MERO_SEMANTIC_ANALYZER_H
