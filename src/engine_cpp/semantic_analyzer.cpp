/**
 * MERO Compiler - Semantic Analyzer Implementation
 * Developer: MERO:TG@QP4RM
 */
#include "semantic_analyzer.h"
#include <cstdarg>
#include <cstdio>

namespace mero {

SemanticAnalyzer::SemanticAnalyzer() {
    mero_error_ctx_init(&error_ctx_);
}

SemanticAnalyzer::~SemanticAnalyzer() {
    mero_error_ctx_destroy(&error_ctx_);
}

void SemanticAnalyzer::analyze(ast::ModuleNode* module) {
    visit_module(module);
}

void SemanticAnalyzer::report(MeroErrorCode code, uint32_t line, const char* fmt, ...) {
    MeroSourceLoc loc = {};
    loc.line = line;
    char buf[512];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    mero_error(&error_ctx_, code, loc, "%s", buf);
}

void SemanticAnalyzer::visit_module(ast::ModuleNode* node) {
    visit_body(node->body);
}

void SemanticAnalyzer::visit_function_def(ast::FunctionDef* node) {
    define_variable(node->name, SymbolKind::Function, node->span.start_line);

    symbols_.push_scope(ScopeKind::Function);
    bool prev_in_func = in_function_;
    in_function_ = true;

    for (auto& param : node->params) {
        SymbolInfo info;
        info.name = param.name;
        info.kind = SymbolKind::Parameter;
        info.is_assigned = true;
        info.decl_line = node->span.start_line;
        symbols_.define(param.name, info);
    }

    visit_body(node->body);

    in_function_ = prev_in_func;
    symbols_.pop_scope();
}

void SemanticAnalyzer::visit_class_def(ast::ClassDef* node) {
    define_variable(node->name, SymbolKind::Class, node->span.start_line);

    symbols_.push_scope(ScopeKind::Class);
    bool prev_in_class = in_class_;
    std::string prev_class = current_class_;
    in_class_ = true;
    current_class_ = node->name;

    visit_body(node->body);

    in_class_ = prev_in_class;
    current_class_ = prev_class;
    symbols_.pop_scope();
}

void SemanticAnalyzer::visit_assign(ast::AssignStmt* node) {
    visit(node->value.get());
    for (auto& target : node->targets) {
        analyze_target(target.get());
    }
}

void SemanticAnalyzer::visit_aug_assign(ast::AugAssignStmt* node) {
    visit(node->value.get());
    visit(node->target.get());
}

void SemanticAnalyzer::visit_name(ast::NameExpr* node) {
    resolve_name(node->id, node->span.start_line);
}

void SemanticAnalyzer::visit_for(ast::ForStmt* node) {
    visit(node->iter.get());
    analyze_target(node->target.get());

    symbols_.push_scope(ScopeKind::Loop);
    visit_body(node->body);
    symbols_.pop_scope();

    visit_body(node->orelse);
}

void SemanticAnalyzer::visit_while(ast::WhileStmt* node) {
    visit(node->test.get());

    symbols_.push_scope(ScopeKind::Loop);
    visit_body(node->body);
    symbols_.pop_scope();

    visit_body(node->orelse);
}

void SemanticAnalyzer::visit_return(ast::ReturnStmt* node) {
    if (!in_function_) {
        report(MERO_ERR_INVALID_RETURN, node->span.start_line,
               "'return' outside function");
    }
    if (node->value) visit(node->value.get());
}

void SemanticAnalyzer::visit_global(ast::GlobalStmt* node) {
    for (auto& name : node->names) {
        auto* sym = symbols_.resolve(name);
        if (sym) {
            sym->is_global = true;
        } else {
            SymbolInfo info;
            info.name = name;
            info.kind = SymbolKind::Variable;
            info.is_global = true;
            symbols_.define(name, info);
        }
    }
}

void SemanticAnalyzer::visit_nonlocal(ast::NonlocalStmt* node) {
    if (!in_function_) {
        report(MERO_ERR_INVALID_SYNTAX, node->span.start_line,
               "'nonlocal' outside function");
    }
    for (auto& name : node->names) {
        auto* sym = symbols_.resolve(name);
        if (sym) {
            sym->is_nonlocal = true;
        }
    }
}

void SemanticAnalyzer::visit_import(ast::ImportStmt* node) {
    for (auto& alias : node->names) {
        std::string name = alias.asname.empty() ? alias.name : alias.asname;
        auto dot_pos = name.find('.');
        if (dot_pos != std::string::npos) name = name.substr(0, dot_pos);
        define_variable(name, SymbolKind::Module, node->span.start_line);
    }
}

void SemanticAnalyzer::visit_import_from(ast::ImportFromStmt* node) {
    for (auto& alias : node->names) {
        if (alias.name == "*") continue;
        std::string name = alias.asname.empty() ? alias.name : alias.asname;
        define_variable(name, SymbolKind::Variable, node->span.start_line);
    }
}

void SemanticAnalyzer::visit_call(ast::CallExpr* node) {
    visit(node->func.get());
    visit_body(node->args);
    for (auto& kw : node->keywords) {
        visit(kw.value.get());
    }
}

void SemanticAnalyzer::define_variable(const std::string& name, SymbolKind kind, uint32_t line) {
    SymbolInfo info;
    info.name = name;
    info.kind = kind;
    info.is_assigned = true;
    info.decl_line = line;
    symbols_.define(name, info);
}

void SemanticAnalyzer::resolve_name(const std::string& name, uint32_t line) {
    auto* sym = symbols_.resolve(name);
    if (!sym) {
        // Only warn, not error - Python is dynamic
        return;
    }
    sym->is_used = true;
}

void SemanticAnalyzer::analyze_target(ast::Node* target) {
    if (!target) return;
    switch (target->kind) {
        case ast::NodeKind::Name: {
            auto* name_node = static_cast<ast::NameExpr*>(target);
            define_variable(name_node->id, SymbolKind::Variable, target->span.start_line);
            break;
        }
        case ast::NodeKind::Tuple:
        case ast::NodeKind::List: {
            auto* container = static_cast<ast::TupleExpr*>(target);
            for (auto& elt : container->elts) {
                analyze_target(elt.get());
            }
            break;
        }
        case ast::NodeKind::Starred: {
            auto* starred = static_cast<ast::StarredExpr*>(target);
            analyze_target(starred->value.get());
            break;
        }
        default:
            visit(target);
            break;
    }
}

} // namespace mero
