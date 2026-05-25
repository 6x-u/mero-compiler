/**
 * MERO Compiler - IR Builder Implementation
 * Developer: MERO:TG@QP4RM
 */
#include "ir_builder.h"

namespace mero {

IRBuilder::IRBuilder(SymbolTable& symbols) : symbols_(symbols) {
    mero_error_ctx_init(&error_ctx_);
}

IRBuilder::~IRBuilder() {
    mero_error_ctx_destroy(&error_ctx_);
}

void IRBuilder::push_body(ir::IRNodeList* body) {
    body_stack_.push_back(current_body_);
    current_body_ = body;
}

void IRBuilder::pop_body() {
    current_body_ = body_stack_.back();
    body_stack_.pop_back();
}

void IRBuilder::emit(ir::IRNodePtr node) {
    if (current_body_) current_body_->push_back(std::move(node));
}

std::unique_ptr<ir::IRModule> IRBuilder::build(ast::ModuleNode* ast_module) {
    module_ = std::make_unique<ir::IRModule>();
    module_->name = "main";
    push_body(&module_->body);

    for (auto& stmt : ast_module->body) {
        convert_stmt(stmt.get());
    }

    pop_body();
    return std::move(module_);
}

void IRBuilder::convert_stmt(ast::Node* node) {
    if (!node) return;
    switch (node->kind) {
        case ast::NodeKind::FunctionDef:
        case ast::NodeKind::AsyncFunctionDef:
            convert_function(static_cast<ast::FunctionDef*>(node)); break;
        case ast::NodeKind::ClassDef:
            convert_class(static_cast<ast::ClassDef*>(node)); break;
        case ast::NodeKind::If:
            convert_if(static_cast<ast::IfStmt*>(node)); break;
        case ast::NodeKind::While:
            convert_while(static_cast<ast::WhileStmt*>(node)); break;
        case ast::NodeKind::For: case ast::NodeKind::AsyncFor:
            convert_for(static_cast<ast::ForStmt*>(node)); break;
        case ast::NodeKind::Try:
            convert_try(static_cast<ast::TryStmt*>(node)); break;
        case ast::NodeKind::With: case ast::NodeKind::AsyncWith:
            convert_with(static_cast<ast::WithStmt*>(node)); break;
        case ast::NodeKind::Assign:
            convert_assign(static_cast<ast::AssignStmt*>(node)); break;
        case ast::NodeKind::AugAssign:
            convert_augassign(static_cast<ast::AugAssignStmt*>(node)); break;
        case ast::NodeKind::Return:
            convert_return(static_cast<ast::ReturnStmt*>(node)); break;
        case ast::NodeKind::Import:
            convert_import(static_cast<ast::ImportStmt*>(node)); break;
        case ast::NodeKind::ImportFrom:
            convert_importfrom(static_cast<ast::ImportFromStmt*>(node)); break;
        case ast::NodeKind::Raise:
            convert_raise(static_cast<ast::RaiseStmt*>(node)); break;
        case ast::NodeKind::Delete:
            convert_delete(static_cast<ast::DeleteStmt*>(node)); break;
        case ast::NodeKind::Assert:
            convert_assert(static_cast<ast::AssertStmt*>(node)); break;
        case ast::NodeKind::Global:
            convert_global(static_cast<ast::GlobalStmt*>(node)); break;
        case ast::NodeKind::Pass: break;
        case ast::NodeKind::Break:
            emit(std::make_unique<ir::IRBreak>()); break;
        case ast::NodeKind::Continue:
            emit(std::make_unique<ir::IRContinue>()); break;
        case ast::NodeKind::ExprStmt: {
            auto* es = static_cast<ast::ExprStmt*>(node);
            auto stmt = std::make_unique<ir::IRExprStmt>();
            stmt->expr = convert_expr(es->value.get());
            emit(std::move(stmt));
            break;
        }
        default: break;
    }
}

ir::IRNodePtr IRBuilder::convert_expr(ast::Node* node) {
    if (!node) return nullptr;
    switch (node->kind) {
        case ast::NodeKind::Constant: return convert_constant(static_cast<ast::ConstantExpr*>(node));
        case ast::NodeKind::Name: return convert_name(static_cast<ast::NameExpr*>(node));
        case ast::NodeKind::BinOp: return convert_binop(static_cast<ast::BinOpExpr*>(node));
        case ast::NodeKind::UnaryOp: return convert_unaryop(static_cast<ast::UnaryOpExpr*>(node));
        case ast::NodeKind::BoolOp: return convert_boolop(static_cast<ast::BoolOpExpr*>(node));
        case ast::NodeKind::Compare: return convert_compare(static_cast<ast::CompareExpr*>(node));
        case ast::NodeKind::Call: return convert_call(static_cast<ast::CallExpr*>(node));
        case ast::NodeKind::Attribute: return convert_attribute(static_cast<ast::AttributeExpr*>(node));
        case ast::NodeKind::Subscript: return convert_subscript(static_cast<ast::SubscriptExpr*>(node));
        case ast::NodeKind::List: return convert_list(static_cast<ast::ListExpr*>(node));
        case ast::NodeKind::Tuple: return convert_tuple(static_cast<ast::TupleExpr*>(node));
        case ast::NodeKind::Dict: return convert_dict(static_cast<ast::DictExpr*>(node));
        case ast::NodeKind::Set: return convert_set(static_cast<ast::SetExpr*>(node));
        case ast::NodeKind::IfExpr: return convert_ifexpr(static_cast<ast::IfExpr*>(node));
        case ast::NodeKind::Lambda: return convert_lambda(static_cast<ast::LambdaExpr*>(node));
        case ast::NodeKind::ListComp: return convert_listcomp(static_cast<ast::ListCompExpr*>(node));
        case ast::NodeKind::JoinedStr: return convert_joinedstr(static_cast<ast::JoinedStrExpr*>(node));
        case ast::NodeKind::Slice: return convert_slice(static_cast<ast::SliceExpr*>(node));
        case ast::NodeKind::Starred: return convert_starred(static_cast<ast::StarredExpr*>(node));
        default: return nullptr;
    }
}

ir::IRNodePtr IRBuilder::convert_constant(ast::ConstantExpr* node) {
    auto lit = std::make_unique<ir::IRLiteral>();
    lit->value = node->value;
    lit->line = node->span.start_line;
    switch (node->const_kind) {
        case ast::ConstantKind::Int:
            lit->lit_kind = ir::IRLiteral::LInt;
            lit->type = {ir::TypeTag::Int, ""};
            break;
        case ast::ConstantKind::Float:
            lit->lit_kind = ir::IRLiteral::LFloat;
            lit->type = {ir::TypeTag::Float, ""};
            break;
        case ast::ConstantKind::Str:
            lit->lit_kind = ir::IRLiteral::LString;
            lit->type = {ir::TypeTag::String, ""};
            break;
        case ast::ConstantKind::Bool:
            lit->lit_kind = ir::IRLiteral::LBool;
            lit->type = {ir::TypeTag::Bool, ""};
            break;
        case ast::ConstantKind::NoneVal:
            lit->lit_kind = ir::IRLiteral::LNil;
            lit->type = {ir::TypeTag::Nil, ""};
            break;
        default:
            lit->lit_kind = ir::IRLiteral::LNil;
            break;
    }
    return lit;
}

ir::IRNodePtr IRBuilder::convert_name(ast::NameExpr* node) {
    auto ident = std::make_unique<ir::IRIdent>();
    ident->name = node->id;
    ident->line = node->span.start_line;
    return ident;
}

ir::IRNodePtr IRBuilder::convert_binop(ast::BinOpExpr* node) {
    auto bin = std::make_unique<ir::IRBinOp>();
    bin->op = map_binop(node->op);
    bin->left = convert_expr(node->left.get());
    bin->right = convert_expr(node->right.get());
    bin->line = node->span.start_line;
    return bin;
}

ir::IRNodePtr IRBuilder::convert_unaryop(ast::UnaryOpExpr* node) {
    auto un = std::make_unique<ir::IRUnaryOp>();
    switch (node->op) {
        case ast::UnaryOpKind::USub: un->op = ir::OpCode::Neg; break;
        case ast::UnaryOpKind::UAdd: return convert_expr(node->operand.get());
        case ast::UnaryOpKind::Not: un->op = ir::OpCode::Not; break;
        case ast::UnaryOpKind::Invert: un->op = ir::OpCode::BitNot; break;
    }
    un->operand = convert_expr(node->operand.get());
    un->line = node->span.start_line;
    return un;
}

ir::IRNodePtr IRBuilder::convert_boolop(ast::BoolOpExpr* node) {
    ir::OpCode op = (node->op == ast::BoolOpKind::And) ? ir::OpCode::And : ir::OpCode::Or;
    auto result = convert_expr(node->values[0].get());
    for (size_t i = 1; i < node->values.size(); i++) {
        auto bin = std::make_unique<ir::IRBinOp>();
        bin->op = op;
        bin->left = std::move(result);
        bin->right = convert_expr(node->values[i].get());
        result = std::move(bin);
    }
    return result;
}

ir::IRNodePtr IRBuilder::convert_compare(ast::CompareExpr* node) {
    auto left = convert_expr(node->left.get());
    if (node->ops.size() == 1) {
        auto cmp = std::make_unique<ir::IRBinOp>();
        cmp->op = map_cmpop(node->ops[0]);
        cmp->left = std::move(left);
        cmp->right = convert_expr(node->comparators[0].get());
        return cmp;
    }
    // Chain comparisons: a < b < c -> a < b and b < c
    auto result = std::make_unique<ir::IRBinOp>();
    result->op = ir::OpCode::And;
    auto first = std::make_unique<ir::IRBinOp>();
    first->op = map_cmpop(node->ops[0]);
    first->left = std::move(left);
    first->right = convert_expr(node->comparators[0].get());
    result->left = std::move(first);

    auto second = std::make_unique<ir::IRBinOp>();
    second->op = map_cmpop(node->ops[1]);
    second->left = convert_expr(node->comparators[0].get());
    second->right = convert_expr(node->comparators[1].get());
    result->right = std::move(second);
    return result;
}

ir::IRNodePtr IRBuilder::convert_call(ast::CallExpr* node) {
    auto call = std::make_unique<ir::IRCall>();
    call->callee = convert_expr(node->func.get());
    for (auto& arg : node->args) {
        call->args.push_back(convert_expr(arg.get()));
    }
    for (auto& kw : node->keywords) {
        call->kwargs.emplace_back(kw.arg, convert_expr(kw.value.get()));
    }
    call->line = node->span.start_line;
    return call;
}

ir::IRNodePtr IRBuilder::convert_attribute(ast::AttributeExpr* node) {
    auto field = std::make_unique<ir::IRField>();
    field->object = convert_expr(node->value.get());
    field->field = node->attr;
    field->line = node->span.start_line;
    return field;
}

ir::IRNodePtr IRBuilder::convert_subscript(ast::SubscriptExpr* node) {
    auto idx = std::make_unique<ir::IRIndex>();
    idx->object = convert_expr(node->value.get());
    idx->index = convert_expr(node->slice.get());
    idx->line = node->span.start_line;
    return idx;
}

ir::IRNodePtr IRBuilder::convert_list(ast::ListExpr* node) {
    auto arr = std::make_unique<ir::IRArray>();
    for (auto& e : node->elts) arr->elements.push_back(convert_expr(e.get()));
    arr->type = {ir::TypeTag::Array, ""};
    return arr;
}

ir::IRNodePtr IRBuilder::convert_tuple(ast::TupleExpr* node) {
    auto arr = std::make_unique<ir::IRArray>();
    for (auto& e : node->elts) arr->elements.push_back(convert_expr(e.get()));
    arr->type = {ir::TypeTag::Array, ""};
    return arr;
}

ir::IRNodePtr IRBuilder::convert_dict(ast::DictExpr* node) {
    auto tbl = std::make_unique<ir::IRTable>();
    for (size_t i = 0; i < node->keys.size(); i++) {
        tbl->pairs.emplace_back(convert_expr(node->keys[i].get()),
                                 convert_expr(node->values[i].get()));
    }
    tbl->type = {ir::TypeTag::Table, ""};
    return tbl;
}

ir::IRNodePtr IRBuilder::convert_set(ast::SetExpr* node) {
    auto tbl = std::make_unique<ir::IRTable>();
    for (auto& e : node->elts) {
        auto lit = std::make_unique<ir::IRLiteral>();
        lit->lit_kind = ir::IRLiteral::LBool;
        lit->value = "true";
        tbl->pairs.emplace_back(convert_expr(e.get()), std::move(lit));
    }
    return tbl;
}

ir::IRNodePtr IRBuilder::convert_ifexpr(ast::IfExpr* node) {
    auto ternary = std::make_unique<ir::IRTernary>();
    ternary->condition = convert_expr(node->test.get());
    ternary->true_val = convert_expr(node->body.get());
    ternary->false_val = convert_expr(node->orelse.get());
    return ternary;
}

ir::IRNodePtr IRBuilder::convert_lambda(ast::LambdaExpr* node) {
    auto lam = std::make_unique<ir::IRLambda>();
    lam->params = convert_params(node->params);
    auto ret = std::make_unique<ir::IRReturn>();
    ret->values.push_back(convert_expr(node->body.get()));
    lam->body.push_back(std::move(ret));
    return lam;
}

ir::IRNodePtr IRBuilder::convert_listcomp(ast::ListCompExpr* node) {
    auto comp = std::make_unique<ir::IRComprehension>();
    comp->element = convert_expr(node->elt.get());
    if (!node->generators.empty()) {
        auto& gen = node->generators[0];
        if (gen.target && gen.target->kind == ast::NodeKind::Name) {
            comp->variable = static_cast<ast::NameExpr*>(gen.target.get())->id;
        }
        comp->iterator = convert_expr(gen.iter.get());
        for (auto& cond : gen.ifs) {
            comp->conditions.push_back(convert_expr(cond.get()));
        }
    }
    return comp;
}

ir::IRNodePtr IRBuilder::convert_joinedstr(ast::JoinedStrExpr* node) {
    auto fs = std::make_unique<ir::IRFString>();
    for (auto& part : node->values) {
        fs->parts.push_back(convert_expr(part.get()));
    }
    return fs;
}

ir::IRNodePtr IRBuilder::convert_slice(ast::SliceExpr* node) {
    auto s = std::make_unique<ir::IRSlice>();
    s->lower = node->lower ? convert_expr(node->lower.get()) : nullptr;
    s->upper = node->upper ? convert_expr(node->upper.get()) : nullptr;
    s->step = node->step ? convert_expr(node->step.get()) : nullptr;
    return s;
}

ir::IRNodePtr IRBuilder::convert_starred(ast::StarredExpr* node) {
    return convert_expr(node->value.get());
}

void IRBuilder::convert_function(ast::FunctionDef* node) {
    auto func = std::make_unique<ir::IRFunction>();
    func->name = node->name;
    func->is_local = true;
    func->params = convert_params(node->params);
    func->line = node->span.start_line;

    push_body(&func->body);
    for (auto& stmt : node->body) convert_stmt(stmt.get());
    pop_body();

    emit(std::move(func));
}

void IRBuilder::convert_class(ast::ClassDef* node) {
    auto cls = std::make_unique<ir::IRClass>();
    cls->name = node->name;
    if (!node->bases.empty() && node->bases[0]->kind == ast::NodeKind::Name) {
        cls->base = static_cast<ast::NameExpr*>(node->bases[0].get())->id;
    }
    cls->line = node->span.start_line;

    for (auto& stmt : node->body) {
        if (stmt->kind == ast::NodeKind::FunctionDef) {
            auto* fdef = static_cast<ast::FunctionDef*>(stmt.get());
            auto method = std::make_unique<ir::IRFunction>();
            method->name = fdef->name;
            method->is_method = true;
            method->params = convert_params(fdef->params);
            push_body(&method->body);
            for (auto& s : fdef->body) convert_stmt(s.get());
            pop_body();
            cls->methods.push_back(std::move(method));
        }
    }

    emit(std::move(cls));
}

void IRBuilder::convert_if(ast::IfStmt* node) {
    auto ir_if = std::make_unique<ir::IRIf>();
    ir_if->condition = convert_expr(node->test.get());
    push_body(&ir_if->then_body);
    for (auto& s : node->body) convert_stmt(s.get());
    pop_body();

    if (!node->orelse.empty()) {
        if (node->orelse.size() == 1 && node->orelse[0]->kind == ast::NodeKind::If) {
            auto* elif = static_cast<ast::IfStmt*>(node->orelse[0].get());
            ir::IRNodeList elif_body;
            push_body(&elif_body);
            for (auto& s : elif->body) convert_stmt(s.get());
            pop_body();
            ir_if->elifs.emplace_back(convert_expr(elif->test.get()), std::move(elif_body));

            if (!elif->orelse.empty()) {
                push_body(&ir_if->else_body);
                for (auto& s : elif->orelse) convert_stmt(s.get());
                pop_body();
            }
        } else {
            push_body(&ir_if->else_body);
            for (auto& s : node->orelse) convert_stmt(s.get());
            pop_body();
        }
    }
    emit(std::move(ir_if));
}

void IRBuilder::convert_while(ast::WhileStmt* node) {
    auto w = std::make_unique<ir::IRWhile>();
    w->condition = convert_expr(node->test.get());
    push_body(&w->body);
    for (auto& s : node->body) convert_stmt(s.get());
    pop_body();
    emit(std::move(w));
}

void IRBuilder::convert_for(ast::ForStmt* node) {
    auto f = std::make_unique<ir::IRForIn>();
    if (node->target->kind == ast::NodeKind::Name) {
        f->vars.push_back(static_cast<ast::NameExpr*>(node->target.get())->id);
    } else if (node->target->kind == ast::NodeKind::Tuple) {
        for (auto& e : static_cast<ast::TupleExpr*>(node->target.get())->elts) {
            if (e->kind == ast::NodeKind::Name)
                f->vars.push_back(static_cast<ast::NameExpr*>(e.get())->id);
        }
    }
    f->iterator = convert_expr(node->iter.get());
    push_body(&f->body);
    for (auto& s : node->body) convert_stmt(s.get());
    pop_body();
    emit(std::move(f));
}

void IRBuilder::convert_try(ast::TryStmt* node) {
    auto t = std::make_unique<ir::IRTry>();
    push_body(&t->body);
    for (auto& s : node->body) convert_stmt(s.get());
    pop_body();

    for (auto& h : node->handlers) {
        ir::IRExceptHandler handler;
        if (h->type && h->type->kind == ast::NodeKind::Name)
            handler.type = static_cast<ast::NameExpr*>(h->type.get())->id;
        handler.name = h->name;
        push_body(&handler.body);
        for (auto& s : h->body) convert_stmt(s.get());
        pop_body();
        t->handlers.push_back(std::move(handler));
    }

    if (!node->finalbody.empty()) {
        push_body(&t->finally_body);
        for (auto& s : node->finalbody) convert_stmt(s.get());
        pop_body();
    }
    emit(std::move(t));
}

void IRBuilder::convert_with(ast::WithStmt* node) {
    // Convert `with` to try/finally pattern
    for (auto& s : node->body) convert_stmt(s.get());
}

void IRBuilder::convert_assign(ast::AssignStmt* node) {
    auto assign = std::make_unique<ir::IRAssign>();
    assign->value = convert_expr(node->value.get());
    if (!node->targets.empty()) {
        assign->target = convert_expr(node->targets[0].get());
    }
    assign->is_local = true;
    assign->line = node->span.start_line;
    emit(std::move(assign));
}

void IRBuilder::convert_augassign(ast::AugAssignStmt* node) {
    auto aug = std::make_unique<ir::IRAugAssign>();
    aug->target = convert_expr(node->target.get());
    aug->op = map_augop(node->op);
    aug->value = convert_expr(node->value.get());
    emit(std::move(aug));
}

void IRBuilder::convert_return(ast::ReturnStmt* node) {
    auto ret = std::make_unique<ir::IRReturn>();
    if (node->value) ret->values.push_back(convert_expr(node->value.get()));
    emit(std::move(ret));
}

void IRBuilder::convert_import(ast::ImportStmt* node) {
    for (auto& alias : node->names) {
        module_->imports.push_back(alias.name);
    }
}

void IRBuilder::convert_importfrom(ast::ImportFromStmt* node) {
    module_->imports.push_back(node->module);
}

void IRBuilder::convert_raise(ast::RaiseStmt* node) {
    auto t = std::make_unique<ir::IRThrow>();
    if (node->exc) t->exception = convert_expr(node->exc.get());
    emit(std::move(t));
}

void IRBuilder::convert_delete(ast::DeleteStmt*) {
    // Delete maps to setting nil in Luau
}

void IRBuilder::convert_assert(ast::AssertStmt* node) {
    auto call = std::make_unique<ir::IRCall>();
    auto callee = std::make_unique<ir::IRIdent>();
    callee->name = "assert";
    call->callee = std::move(callee);
    call->args.push_back(convert_expr(node->test.get()));
    if (node->msg) call->args.push_back(convert_expr(node->msg.get()));
    auto stmt = std::make_unique<ir::IRExprStmt>();
    stmt->expr = std::move(call);
    emit(std::move(stmt));
}

void IRBuilder::convert_global(ast::GlobalStmt* node) {
    auto g = std::make_unique<ir::IRGlobal>();
    g->names = node->names;
    emit(std::move(g));
}

ir::OpCode IRBuilder::map_binop(TokenType op) {
    switch (op) {
        case TokenType::PLUS: return ir::OpCode::Add;
        case TokenType::MINUS: return ir::OpCode::Sub;
        case TokenType::STAR: return ir::OpCode::Mul;
        case TokenType::SLASH: return ir::OpCode::Div;
        case TokenType::DOUBLE_SLASH: return ir::OpCode::FloorDiv;
        case TokenType::PERCENT: return ir::OpCode::Mod;
        case TokenType::DOUBLE_STAR: return ir::OpCode::Pow;
        case TokenType::AMPERSAND: return ir::OpCode::BitAnd;
        case TokenType::PIPE: return ir::OpCode::BitOr;
        case TokenType::CARET: return ir::OpCode::BitXor;
        case TokenType::LSHIFT: return ir::OpCode::Shl;
        case TokenType::RSHIFT: return ir::OpCode::Shr;
        default: return ir::OpCode::Add;
    }
}

ir::OpCode IRBuilder::map_cmpop(ast::CmpOp op) {
    switch (op) {
        case ast::CmpOp::Eq: return ir::OpCode::Eq;
        case ast::CmpOp::NotEq: return ir::OpCode::Neq;
        case ast::CmpOp::Lt: return ir::OpCode::Lt;
        case ast::CmpOp::Gt: return ir::OpCode::Gt;
        case ast::CmpOp::LtE: return ir::OpCode::Lte;
        case ast::CmpOp::GtE: return ir::OpCode::Gte;
        default: return ir::OpCode::Eq;
    }
}

ir::OpCode IRBuilder::map_augop(TokenType op) {
    switch (op) {
        case TokenType::PLUS_ASSIGN: return ir::OpCode::Add;
        case TokenType::MINUS_ASSIGN: return ir::OpCode::Sub;
        case TokenType::STAR_ASSIGN: return ir::OpCode::Mul;
        case TokenType::SLASH_ASSIGN: return ir::OpCode::Div;
        case TokenType::DSLASH_ASSIGN: return ir::OpCode::FloorDiv;
        case TokenType::PERCENT_ASSIGN: return ir::OpCode::Mod;
        case TokenType::DSTAR_ASSIGN: return ir::OpCode::Pow;
        default: return ir::OpCode::Add;
    }
}

std::vector<ir::IRParam> IRBuilder::convert_params(const std::vector<ast::Param>& params) {
    std::vector<ir::IRParam> result;
    for (auto& p : params) {
        ir::IRParam ip;
        ip.name = p.name;
        ip.variadic = p.is_vararg || p.is_kwarg;
        if (p.default_value) ip.default_val = convert_expr(p.default_value.get());
        result.push_back(std::move(ip));
    }
    return result;
}

} // namespace mero
