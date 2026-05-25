/**
 * MERO Compiler - Type Checker Implementation
 * Developer: MERO:TG@QP4RM
 */
#include "type_checker.h"

namespace mero {

TypeChecker::TypeChecker(SymbolTable& symbols) : symbols_(symbols) {}

void TypeChecker::check(ast::ModuleNode* module) {
    visit_body(module->body);
}

ir::TypeTag TypeChecker::infer_type(ast::Node* node) {
    if (!node) return ir::TypeTag::Any;

    switch (node->kind) {
        case ast::NodeKind::Constant: {
            auto* c = static_cast<ast::ConstantExpr*>(node);
            switch (c->const_kind) {
                case ast::ConstantKind::Int: return ir::TypeTag::Int;
                case ast::ConstantKind::Float: return ir::TypeTag::Float;
                case ast::ConstantKind::Str: return ir::TypeTag::String;
                case ast::ConstantKind::Bool: return ir::TypeTag::Bool;
                case ast::ConstantKind::NoneVal: return ir::TypeTag::Nil;
                default: return ir::TypeTag::Any;
            }
        }
        case ast::NodeKind::Name: {
            auto* name = static_cast<ast::NameExpr*>(node);
            auto it = var_types_.find(name->id);
            if (it != var_types_.end()) return it->second;
            return ir::TypeTag::Any;
        }
        case ast::NodeKind::List: return ir::TypeTag::Array;
        case ast::NodeKind::Dict: return ir::TypeTag::Table;
        case ast::NodeKind::BinOp: {
            auto* bin = static_cast<ast::BinOpExpr*>(node);
            auto lt = infer_type(bin->left.get());
            auto rt = infer_type(bin->right.get());
            return infer_binop_type(lt, rt, bin->op);
        }
        case ast::NodeKind::Call: {
            auto* call = static_cast<ast::CallExpr*>(node);
            if (call->func->kind == ast::NodeKind::Name) {
                auto* name = static_cast<ast::NameExpr*>(call->func.get());
                return infer_call_type(name->id);
            }
            return ir::TypeTag::Any;
        }
        default: return ir::TypeTag::Any;
    }
}

void TypeChecker::visit_assign(ast::AssignStmt* node) {
    auto type = infer_type(node->value.get());
    for (auto& target : node->targets) {
        if (target->kind == ast::NodeKind::Name) {
            auto* name = static_cast<ast::NameExpr*>(target.get());
            record_type(name->id, type);
        }
    }
    ast::ASTVisitor::visit_assign(node);
}

void TypeChecker::visit_function_def(ast::FunctionDef* node) {
    auto prev_ret = current_return_type_;
    current_return_type_ = ir::TypeTag::Any;

    if (node->return_annotation) {
        current_return_type_ = type_from_annotation(node->return_annotation.get());
    }

    for (auto& param : node->params) {
        if (param.annotation) {
            record_type(param.name, type_from_annotation(param.annotation.get()));
        } else {
            record_type(param.name, ir::TypeTag::Any);
        }
    }

    visit_body(node->body);
    current_return_type_ = prev_ret;
}

void TypeChecker::visit_return(ast::ReturnStmt* node) {
    if (node->value && current_return_type_ != ir::TypeTag::Any) {
        auto actual = infer_type(node->value.get());
        if (actual != ir::TypeTag::Any && actual != current_return_type_) {
            warn("return type mismatch", node->span.start_line);
        }
    }
}

void TypeChecker::visit_bin_op(ast::BinOpExpr* node) {
    ast::ASTVisitor::visit_bin_op(node);
}

void TypeChecker::visit_call(ast::CallExpr* node) {
    ast::ASTVisitor::visit_call(node);
}

ir::TypeTag TypeChecker::infer_binop_type(ir::TypeTag left, ir::TypeTag right, TokenType op) {
    if (left == ir::TypeTag::String && right == ir::TypeTag::String) {
        if (op == TokenType::PLUS) return ir::TypeTag::String;
    }
    if (left == ir::TypeTag::Int && right == ir::TypeTag::Int) {
        if (op == TokenType::SLASH) return ir::TypeTag::Float;
        return ir::TypeTag::Int;
    }
    if ((left == ir::TypeTag::Float || right == ir::TypeTag::Float) &&
        (left == ir::TypeTag::Int || left == ir::TypeTag::Float) &&
        (right == ir::TypeTag::Int || right == ir::TypeTag::Float)) {
        return ir::TypeTag::Float;
    }
    return ir::TypeTag::Any;
}

ir::TypeTag TypeChecker::infer_call_type(const std::string& func_name) {
    if (func_name == "int") return ir::TypeTag::Int;
    if (func_name == "float") return ir::TypeTag::Float;
    if (func_name == "str") return ir::TypeTag::String;
    if (func_name == "bool") return ir::TypeTag::Bool;
    if (func_name == "len") return ir::TypeTag::Int;
    if (func_name == "list") return ir::TypeTag::Array;
    if (func_name == "dict") return ir::TypeTag::Table;
    if (func_name == "abs") return ir::TypeTag::Int;
    if (func_name == "round") return ir::TypeTag::Int;
    return ir::TypeTag::Any;
}

ir::TypeTag TypeChecker::type_from_annotation(ast::Node* annotation) {
    if (!annotation) return ir::TypeTag::Any;
    if (annotation->kind == ast::NodeKind::Name) {
        auto* name = static_cast<ast::NameExpr*>(annotation);
        if (name->id == "int") return ir::TypeTag::Int;
        if (name->id == "float") return ir::TypeTag::Float;
        if (name->id == "str") return ir::TypeTag::String;
        if (name->id == "bool") return ir::TypeTag::Bool;
        if (name->id == "list") return ir::TypeTag::Array;
        if (name->id == "dict") return ir::TypeTag::Table;
        if (name->id == "None") return ir::TypeTag::Nil;
    }
    return ir::TypeTag::Any;
}

void TypeChecker::record_type(const std::string& name, ir::TypeTag type) {
    var_types_[name] = type;
}

void TypeChecker::warn(const std::string& msg, uint32_t line) {
    warnings_.push_back("line " + std::to_string(line) + ": " + msg);
}

} // namespace mero
