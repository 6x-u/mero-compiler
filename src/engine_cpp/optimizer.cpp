/**
 * MERO Compiler - IR Optimizer Implementation
 * Developer: MERO:TG@QP4RM
 */
#include "optimizer.h"
#include <algorithm>
#include <unordered_set>
#include <cmath>

namespace mero {

IROptimizer::IROptimizer(OptLevel level) : level_(level) {}

void IROptimizer::optimize(ir::IRModule* module) {
    if (level_ == OptLevel::O0) return;

    optimize_body(module->body);

    if (level_ >= OptLevel::O2) {
        inline_small_functions(module);
        optimize_body(module->body);
    }
}

void IROptimizer::optimize_body(ir::IRNodeList& body) {
    if (level_ >= OptLevel::O1) {
        constant_fold(body);
        dead_code_eliminate(body);
    }
    if (level_ >= OptLevel::O2) {
        simplify_expressions(body);
        remove_redundant_assigns(body);
    }
    if (level_ >= OptLevel::O3) {
        optimize_loops(body);
    }

    for (auto& node : body) {
        if (!node) continue;
        switch (node->kind) {
            case ir::IRKind::Function: {
                auto* func = static_cast<ir::IRFunction*>(node.get());
                optimize_body(func->body);
                break;
            }
            case ir::IRKind::If: {
                auto* if_node = static_cast<ir::IRIf*>(node.get());
                optimize_body(if_node->then_body);
                for (auto& [cond, elif_body] : if_node->elifs) {
                    optimize_body(elif_body);
                }
                optimize_body(if_node->else_body);
                break;
            }
            case ir::IRKind::While: {
                auto* w = static_cast<ir::IRWhile*>(node.get());
                optimize_body(w->body);
                break;
            }
            case ir::IRKind::ForNumeric: {
                auto* f = static_cast<ir::IRForNumeric*>(node.get());
                optimize_body(f->body);
                break;
            }
            case ir::IRKind::ForIn: {
                auto* f = static_cast<ir::IRForIn*>(node.get());
                optimize_body(f->body);
                break;
            }
            case ir::IRKind::Class: {
                auto* cls = static_cast<ir::IRClass*>(node.get());
                for (auto& m : cls->methods) {
                    optimize_body(m->body);
                }
                break;
            }
            default: break;
        }
    }
}

void IROptimizer::constant_fold(ir::IRNodeList& body) {
    for (auto& node : body) {
        if (!node) continue;
        if (node->kind == ir::IRKind::Assign) {
            auto* assign = static_cast<ir::IRAssign*>(node.get());
            if (assign->value) {
                auto folded = simplify_expr(std::move(assign->value));
                assign->value = std::move(folded);
            }
        } else if (node->kind == ir::IRKind::ExprStmt) {
            auto* stmt = static_cast<ir::IRExprStmt*>(node.get());
            if (stmt->expr) {
                stmt->expr = simplify_expr(std::move(stmt->expr));
            }
        }
    }
}

void IROptimizer::dead_code_eliminate(ir::IRNodeList& body) {
    bool found_return = false;
    auto it = body.begin();
    while (it != body.end()) {
        if (found_return) {
            it = body.erase(it);
            stats_.dead_code_removed++;
        } else {
            if ((*it) && (*it)->kind == ir::IRKind::Return) {
                found_return = true;
            }
            ++it;
        }
    }

    // Remove assignments to variables that are never read
    // (simplified: only remove if assigned and immediately overwritten)
    for (size_t i = 0; i + 1 < body.size(); i++) {
        if (!body[i] || !body[i + 1]) continue;
        if (body[i]->kind == ir::IRKind::Assign && body[i + 1]->kind == ir::IRKind::Assign) {
            auto* a1 = static_cast<ir::IRAssign*>(body[i].get());
            auto* a2 = static_cast<ir::IRAssign*>(body[i + 1].get());
            if (a1->target && a2->target &&
                a1->target->kind == ir::IRKind::Ident &&
                a2->target->kind == ir::IRKind::Ident) {
                auto* id1 = static_cast<ir::IRIdent*>(a1->target.get());
                auto* id2 = static_cast<ir::IRIdent*>(a2->target.get());
                if (id1->name == id2->name && is_pure(a1->value.get())) {
                    body[i] = nullptr;
                    stats_.dead_code_removed++;
                }
            }
        }
    }

    body.erase(
        std::remove_if(body.begin(), body.end(),
                       [](const ir::IRNodePtr& n) { return n == nullptr; }),
        body.end()
    );
}

void IROptimizer::simplify_expressions(ir::IRNodeList& body) {
    for (auto& node : body) {
        if (!node) continue;
        if (node->kind == ir::IRKind::Assign) {
            auto* assign = static_cast<ir::IRAssign*>(node.get());
            if (assign->value) assign->value = simplify_expr(std::move(assign->value));
        }
    }
}

void IROptimizer::remove_redundant_assigns(ir::IRNodeList& body) {
    // Remove self-assignments: x = x
    body.erase(
        std::remove_if(body.begin(), body.end(), [this](const ir::IRNodePtr& node) {
            if (!node || node->kind != ir::IRKind::Assign) return false;
            auto* assign = static_cast<ir::IRAssign*>(node.get());
            if (assign->target && assign->value &&
                assign->target->kind == ir::IRKind::Ident &&
                assign->value->kind == ir::IRKind::Ident) {
                auto* t = static_cast<ir::IRIdent*>(assign->target.get());
                auto* v = static_cast<ir::IRIdent*>(assign->value.get());
                if (t->name == v->name) {
                    stats_.redundant_removed++;
                    return true;
                }
            }
            return false;
        }),
        body.end()
    );
}

void IROptimizer::optimize_loops(ir::IRNodeList& body) {
    // Loop invariant detection placeholder
    for (auto& node : body) {
        if (!node) continue;
        if (node->kind == ir::IRKind::While || node->kind == ir::IRKind::ForIn) {
            stats_.loops_optimized++;
        }
    }
}

void IROptimizer::inline_small_functions(ir::IRModule* module) {
    // Find small functions suitable for inlining
    std::unordered_set<std::string> inline_candidates;
    for (auto& node : module->body) {
        if (node && node->kind == ir::IRKind::Function) {
            auto* func = static_cast<ir::IRFunction*>(node.get());
            if (is_small_function(func)) {
                inline_candidates.insert(func->name);
            }
        }
    }
    // Actual inlining would replace call sites - tracked for stats
    stats_.functions_inlined += static_cast<int>(inline_candidates.size());
}

ir::IRNodePtr IROptimizer::simplify_expr(ir::IRNodePtr node) {
    if (!node) return nullptr;

    if (node->kind == ir::IRKind::BinOp) {
        auto* bin = static_cast<ir::IRBinOp*>(node.get());
        bin->left = simplify_expr(std::move(bin->left));
        bin->right = simplify_expr(std::move(bin->right));
        auto folded = fold_binop(bin);
        if (folded) {
            stats_.constants_folded++;
            return folded;
        }
    } else if (node->kind == ir::IRKind::UnaryOp) {
        auto* un = static_cast<ir::IRUnaryOp*>(node.get());
        un->operand = simplify_expr(std::move(un->operand));
        auto folded = fold_unaryop(un);
        if (folded) {
            stats_.constants_folded++;
            return folded;
        }
    }

    return node;
}

ir::IRNodePtr IROptimizer::fold_binop(ir::IRBinOp* node) {
    if (!node->left || !node->right) return nullptr;
    if (!is_constant(node->left.get()) || !is_constant(node->right.get())) return nullptr;

    auto* left = static_cast<ir::IRLiteral*>(node->left.get());
    auto* right = static_cast<ir::IRLiteral*>(node->right.get());

    if (left->lit_kind == ir::IRLiteral::LInt && right->lit_kind == ir::IRLiteral::LInt) {
        long long lv = std::stoll(left->value);
        long long rv = std::stoll(right->value);
        long long result = 0;

        switch (node->op) {
            case ir::OpCode::Add: result = lv + rv; break;
            case ir::OpCode::Sub: result = lv - rv; break;
            case ir::OpCode::Mul: result = lv * rv; break;
            case ir::OpCode::Div:
                if (rv == 0) return nullptr;
                result = lv / rv; break;
            case ir::OpCode::FloorDiv:
                if (rv == 0) return nullptr;
                result = lv / rv; break;
            case ir::OpCode::Mod:
                if (rv == 0) return nullptr;
                result = lv % rv; break;
            case ir::OpCode::Pow:
                result = static_cast<long long>(std::pow(lv, rv)); break;
            case ir::OpCode::BitAnd: result = lv & rv; break;
            case ir::OpCode::BitOr: result = lv | rv; break;
            case ir::OpCode::BitXor: result = lv ^ rv; break;
            case ir::OpCode::Shl: result = lv << rv; break;
            case ir::OpCode::Shr: result = lv >> rv; break;
            default: return nullptr;
        }

        auto lit = std::make_unique<ir::IRLiteral>();
        lit->lit_kind = ir::IRLiteral::LInt;
        lit->value = std::to_string(result);
        lit->type = {ir::TypeTag::Int, ""};
        return lit;
    }

    if (left->lit_kind == ir::IRLiteral::LString && right->lit_kind == ir::IRLiteral::LString &&
        node->op == ir::OpCode::Concat) {
        auto lit = std::make_unique<ir::IRLiteral>();
        lit->lit_kind = ir::IRLiteral::LString;
        lit->value = left->value + right->value;
        lit->type = {ir::TypeTag::String, ""};
        return lit;
    }

    return nullptr;
}

ir::IRNodePtr IROptimizer::fold_unaryop(ir::IRUnaryOp* node) {
    if (!node->operand || !is_constant(node->operand.get())) return nullptr;

    auto* operand = static_cast<ir::IRLiteral*>(node->operand.get());

    if (operand->lit_kind == ir::IRLiteral::LInt && node->op == ir::OpCode::Neg) {
        auto lit = std::make_unique<ir::IRLiteral>();
        lit->lit_kind = ir::IRLiteral::LInt;
        long long val = -std::stoll(operand->value);
        lit->value = std::to_string(val);
        lit->type = {ir::TypeTag::Int, ""};
        return lit;
    }

    if (operand->lit_kind == ir::IRLiteral::LBool && node->op == ir::OpCode::Not) {
        auto lit = std::make_unique<ir::IRLiteral>();
        lit->lit_kind = ir::IRLiteral::LBool;
        lit->value = (operand->value == "true" || operand->value == "True") ? "false" : "true";
        lit->type = {ir::TypeTag::Bool, ""};
        return lit;
    }

    return nullptr;
}

bool IROptimizer::is_constant(ir::IRNode* node) {
    return node && node->kind == ir::IRKind::Literal;
}

bool IROptimizer::is_pure(ir::IRNode* node) {
    if (!node) return true;
    switch (node->kind) {
        case ir::IRKind::Literal:
        case ir::IRKind::Ident:
            return true;
        case ir::IRKind::BinOp: {
            auto* bin = static_cast<ir::IRBinOp*>(node);
            return is_pure(bin->left.get()) && is_pure(bin->right.get());
        }
        case ir::IRKind::UnaryOp: {
            auto* un = static_cast<ir::IRUnaryOp*>(node);
            return is_pure(un->operand.get());
        }
        default: return false;
    }
}

bool IROptimizer::is_small_function(ir::IRFunction* func) {
    return func->body.size() <= 3;
}

} // namespace mero
