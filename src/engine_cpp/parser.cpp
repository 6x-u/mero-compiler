/**
 * MERO Compiler - Python Parser Implementation
 * Developer: MERO:TG@QP4RM
 */
#include "parser.h"
#include <cstring>

namespace mero {

Parser::Parser(Lexer& lexer) : lexer_(lexer) {
    mero_error_ctx_init(&error_ctx_);
    advance();
}

Parser::~Parser() {
    mero_error_ctx_destroy(&error_ctx_);
}

Token Parser::advance() {
    previous_ = current_;
    current_ = lexer_.next_token();
    while (current_.type == TokenType::NEWLINE && check(TokenType::NEWLINE)) {
        current_ = lexer_.next_token();
    }
    return previous_;
}

bool Parser::check(TokenType type) const {
    return current_.type == type;
}

bool Parser::match(TokenType type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

Token Parser::consume(TokenType type, const char* msg) {
    if (check(type)) return advance();
    error(msg);
    return current_;
}

void Parser::error(const char* msg) {
    error_at(current_, msg);
}

void Parser::error_at(const Token& tok, const char* msg) {
    MeroSourceLoc loc = {};
    loc.line = tok.start.line;
    loc.column = tok.start.column;
    mero_error(&error_ctx_, MERO_ERR_INVALID_SYNTAX, loc, "%s", msg);
}

void Parser::synchronize() {
    while (!check(TokenType::END_OF_FILE)) {
        if (previous_.type == TokenType::NEWLINE) return;
        switch (current_.type) {
            case TokenType::KW_DEF:
            case TokenType::KW_CLASS:
            case TokenType::KW_IF:
            case TokenType::KW_FOR:
            case TokenType::KW_WHILE:
            case TokenType::KW_RETURN:
            case TokenType::KW_IMPORT:
            case TokenType::KW_FROM:
            case TokenType::KW_TRY:
                return;
            default:
                advance();
        }
    }
}

bool Parser::peek_is_in() const {
    // Check if the next token after current is 'in' (for "not in" pattern)
    // We peek by checking lexer state - simple approach: 
    // current_ is KW_NOT, we need next to be KW_IN
    return lexer_.peek_type() == TokenType::KW_IN;
}

std::unique_ptr<ast::ModuleNode> Parser::parse_module() {
    auto module = std::make_unique<ast::ModuleNode>();

    while (!check(TokenType::END_OF_FILE)) {
        if (match(TokenType::NEWLINE)) continue;
        auto stmt = parse_statement();
        if (stmt) {
            module->body.push_back(std::move(stmt));
        } else {
            synchronize();
        }
    }

    return module;
}

ast::NodePtr Parser::parse_statement() {
    if (check(TokenType::KW_DEF) || check(TokenType::KW_ASYNC) ||
        check(TokenType::KW_CLASS) || check(TokenType::KW_IF) ||
        check(TokenType::KW_WHILE) || check(TokenType::KW_FOR) ||
        check(TokenType::KW_TRY) || check(TokenType::KW_WITH) ||
        check(TokenType::AT)) {
        return parse_compound_statement();
    }
    return parse_simple_statement();
}

ast::NodePtr Parser::parse_simple_statement() {
    ast::NodePtr stmt;

    if (check(TokenType::KW_RETURN)) stmt = parse_return_stmt();
    else if (check(TokenType::KW_RAISE)) stmt = parse_raise_stmt();
    else if (check(TokenType::KW_IMPORT)) stmt = parse_import_stmt();
    else if (check(TokenType::KW_FROM)) stmt = parse_from_import_stmt();
    else if (check(TokenType::KW_GLOBAL)) stmt = parse_global_stmt();
    else if (check(TokenType::KW_NONLOCAL)) stmt = parse_nonlocal_stmt();
    else if (check(TokenType::KW_ASSERT)) stmt = parse_assert_stmt();
    else if (check(TokenType::KW_DEL)) stmt = parse_del_stmt();
    else if (check(TokenType::KW_PASS)) { advance(); stmt = std::make_unique<ast::PassStmt>(); }
    else if (check(TokenType::KW_BREAK)) {
        advance();
        if (loop_depth_ == 0) error("'break' outside loop");
        stmt = std::make_unique<ast::BreakStmt>();
    }
    else if (check(TokenType::KW_CONTINUE)) {
        advance();
        if (loop_depth_ == 0) error("'continue' outside loop");
        stmt = std::make_unique<ast::ContinueStmt>();
    }
    else {
        stmt = parse_assign_or_expr();
    }

    match(TokenType::NEWLINE);
    return stmt;
}

ast::NodePtr Parser::parse_compound_statement() {
    if (check(TokenType::AT)) {
        auto decorators = parse_decorators();
        ast::NodePtr def;
        if (check(TokenType::KW_DEF)) def = parse_function_def();
        else if (check(TokenType::KW_ASYNC)) def = parse_function_def(true);
        else if (check(TokenType::KW_CLASS)) def = parse_class_def();
        else { error("expected function or class definition after decorator"); return nullptr; }

        if (def && def->kind == ast::NodeKind::FunctionDef) {
            static_cast<ast::FunctionDef*>(def.get())->decorators = std::move(decorators);
        } else if (def && def->kind == ast::NodeKind::ClassDef) {
            static_cast<ast::ClassDef*>(def.get())->decorators = std::move(decorators);
        }
        return def;
    }

    if (check(TokenType::KW_DEF)) return parse_function_def();
    if (check(TokenType::KW_ASYNC)) {
        advance();
        if (check(TokenType::KW_DEF)) return parse_function_def(true);
        if (check(TokenType::KW_FOR)) return parse_for_stmt(true);
        if (check(TokenType::KW_WITH)) return parse_with_stmt(true);
        error("expected 'def', 'for', or 'with' after 'async'");
        return nullptr;
    }
    if (check(TokenType::KW_CLASS)) return parse_class_def();
    if (check(TokenType::KW_IF)) return parse_if_stmt();
    if (check(TokenType::KW_WHILE)) return parse_while_stmt();
    if (check(TokenType::KW_FOR)) return parse_for_stmt();
    if (check(TokenType::KW_TRY)) return parse_try_stmt();
    if (check(TokenType::KW_WITH)) return parse_with_stmt();

    error("expected statement");
    return nullptr;
}

ast::NodeList Parser::parse_decorators() {
    ast::NodeList decorators;
    while (match(TokenType::AT)) {
        decorators.push_back(parse_expression());
        match(TokenType::NEWLINE);
    }
    return decorators;
}

ast::NodeList Parser::parse_block() {
    ast::NodeList body;
    consume(TokenType::COLON, "expected ':'");
    match(TokenType::NEWLINE);
    consume(TokenType::INDENT, "expected indented block");

    while (!check(TokenType::DEDENT) && !check(TokenType::END_OF_FILE)) {
        if (match(TokenType::NEWLINE)) continue;
        auto stmt = parse_statement();
        if (stmt) body.push_back(std::move(stmt));
        else synchronize();
    }

    match(TokenType::DEDENT);
    return body;
}

ast::NodePtr Parser::parse_function_def(bool is_async) {
    if (!is_async) advance(); // consume 'def'
    else advance(); // consume 'def' after 'async' already consumed

    auto func = std::make_unique<ast::FunctionDef>();
    func->is_async = is_async;
    func->name = current_.value;
    consume(TokenType::IDENTIFIER, "expected function name");

    consume(TokenType::LPAREN, "expected '('");
    func->params = parse_parameters();
    consume(TokenType::RPAREN, "expected ')'");

    if (match(TokenType::ARROW)) {
        func->return_annotation = parse_expression();
    }

    func_depth_++;
    func->body = parse_block();
    func_depth_--;

    return func;
}

std::vector<ast::Param> Parser::parse_parameters() {
    std::vector<ast::Param> params;
    bool seen_default = false;

    while (!check(TokenType::RPAREN) && !check(TokenType::END_OF_FILE)) {
        ast::Param param;

        if (match(TokenType::STAR)) {
            if (check(TokenType::COMMA)) {
                params.push_back(std::move(param));
                advance();
                continue;
            }
            param.is_vararg = true;
        } else if (match(TokenType::DOUBLE_STAR)) {
            param.is_kwarg = true;
        }

        param.name = current_.value;
        consume(TokenType::IDENTIFIER, "expected parameter name");

        if (match(TokenType::COLON)) {
            param.annotation = parse_expression();
        }

        if (match(TokenType::ASSIGN)) {
            param.default_value = parse_expression();
            seen_default = true;
        } else if (seen_default && !param.is_vararg && !param.is_kwarg) {
            // Non-default follows default - allowed only for *args/**kwargs
        }

        params.push_back(std::move(param));

        if (!match(TokenType::COMMA)) break;
    }

    return params;
}

ast::NodePtr Parser::parse_class_def() {
    advance(); // 'class'
    auto cls = std::make_unique<ast::ClassDef>();
    cls->name = current_.value;
    consume(TokenType::IDENTIFIER, "expected class name");

    if (match(TokenType::LPAREN)) {
        while (!check(TokenType::RPAREN) && !check(TokenType::END_OF_FILE)) {
            cls->bases.push_back(parse_expression());
            if (!match(TokenType::COMMA)) break;
        }
        consume(TokenType::RPAREN, "expected ')'");
    }

    cls->body = parse_block();
    return cls;
}

ast::NodePtr Parser::parse_if_stmt() {
    advance(); // 'if'
    auto node = std::make_unique<ast::IfStmt>();
    node->test = parse_expression();
    node->body = parse_block();

    while (match(TokenType::KW_ELIF)) {
        auto elif = std::make_unique<ast::IfStmt>();
        elif->test = parse_expression();
        elif->body = parse_block();
        node->orelse.push_back(std::move(elif));
    }

    if (match(TokenType::KW_ELSE)) {
        node->orelse = parse_block();
    }

    return node;
}

ast::NodePtr Parser::parse_while_stmt() {
    advance(); // 'while'
    auto node = std::make_unique<ast::WhileStmt>();
    node->test = parse_expression();

    loop_depth_++;
    node->body = parse_block();
    loop_depth_--;

    if (match(TokenType::KW_ELSE)) {
        node->orelse = parse_block();
    }
    return node;
}

ast::NodePtr Parser::parse_for_stmt(bool is_async) {
    advance(); // 'for'
    auto node = std::make_unique<ast::ForStmt>();
    node->is_async = is_async;
    node->target = parse_expression();
    consume(TokenType::KW_IN, "expected 'in'");
    node->iter = parse_expression();

    loop_depth_++;
    node->body = parse_block();
    loop_depth_--;

    if (match(TokenType::KW_ELSE)) {
        node->orelse = parse_block();
    }
    return node;
}

ast::NodePtr Parser::parse_try_stmt() {
    advance(); // 'try'
    auto node = std::make_unique<ast::TryStmt>();
    node->body = parse_block();

    while (check(TokenType::KW_EXCEPT)) {
        advance();
        auto handler = std::make_unique<ast::ExceptHandlerNode>();
        if (!check(TokenType::COLON)) {
            handler->type = parse_expression();
            if (match(TokenType::KW_AS)) {
                handler->name = current_.value;
                consume(TokenType::IDENTIFIER, "expected exception name");
            }
        }
        handler->body = parse_block();
        node->handlers.push_back(std::move(handler));
    }

    if (match(TokenType::KW_ELSE)) {
        node->orelse = parse_block();
    }

    if (match(TokenType::KW_FINALLY)) {
        node->finalbody = parse_block();
    }

    return node;
}

ast::NodePtr Parser::parse_with_stmt(bool is_async) {
    advance(); // 'with'
    auto node = std::make_unique<ast::WithStmt>();
    node->is_async = is_async;

    do {
        ast::WithItem item;
        item.context_expr = parse_expression();
        if (match(TokenType::KW_AS)) {
            item.optional_vars = parse_expression();
        }
        node->items.push_back(std::move(item));
    } while (match(TokenType::COMMA));

    node->body = parse_block();
    return node;
}

ast::NodePtr Parser::parse_return_stmt() {
    advance(); // 'return'
    auto node = std::make_unique<ast::ReturnStmt>();
    if (!check(TokenType::NEWLINE) && !check(TokenType::END_OF_FILE)) {
        node->value = parse_expression();
    }
    return node;
}

ast::NodePtr Parser::parse_raise_stmt() {
    advance(); // 'raise'
    auto node = std::make_unique<ast::RaiseStmt>();
    if (!check(TokenType::NEWLINE) && !check(TokenType::END_OF_FILE)) {
        node->exc = parse_expression();
        if (match(TokenType::KW_FROM)) {
            node->cause = parse_expression();
        }
    }
    return node;
}

ast::NodePtr Parser::parse_import_stmt() {
    advance(); // 'import'
    auto node = std::make_unique<ast::ImportStmt>();

    do {
        ast::AliasNode alias;
        alias.name = current_.value;
        consume(TokenType::IDENTIFIER, "expected module name");
        while (match(TokenType::DOT)) {
            alias.name += ".";
            alias.name += current_.value;
            consume(TokenType::IDENTIFIER, "expected module name");
        }
        if (match(TokenType::KW_AS)) {
            alias.asname = current_.value;
            consume(TokenType::IDENTIFIER, "expected alias");
        }
        node->names.push_back(std::move(alias));
    } while (match(TokenType::COMMA));

    return node;
}

ast::NodePtr Parser::parse_from_import_stmt() {
    advance(); // 'from'
    auto node = std::make_unique<ast::ImportFromStmt>();

    while (match(TokenType::DOT)) node->level++;

    if (check(TokenType::IDENTIFIER)) {
        node->module = current_.value;
        advance();
        while (match(TokenType::DOT)) {
            node->module += ".";
            node->module += current_.value;
            consume(TokenType::IDENTIFIER, "expected module name");
        }
    }

    consume(TokenType::KW_IMPORT, "expected 'import'");

    if (match(TokenType::STAR)) {
        ast::AliasNode alias;
        alias.name = "*";
        node->names.push_back(alias);
    } else {
        bool has_paren = match(TokenType::LPAREN);
        do {
            ast::AliasNode alias;
            alias.name = current_.value;
            consume(TokenType::IDENTIFIER, "expected name");
            if (match(TokenType::KW_AS)) {
                alias.asname = current_.value;
                consume(TokenType::IDENTIFIER, "expected alias");
            }
            node->names.push_back(std::move(alias));
        } while (match(TokenType::COMMA));
        if (has_paren) consume(TokenType::RPAREN, "expected ')'");
    }

    return node;
}

ast::NodePtr Parser::parse_global_stmt() {
    advance();
    auto node = std::make_unique<ast::GlobalStmt>();
    do {
        node->names.push_back(current_.value);
        consume(TokenType::IDENTIFIER, "expected variable name");
    } while (match(TokenType::COMMA));
    return node;
}

ast::NodePtr Parser::parse_nonlocal_stmt() {
    advance();
    auto node = std::make_unique<ast::NonlocalStmt>();
    do {
        node->names.push_back(current_.value);
        consume(TokenType::IDENTIFIER, "expected variable name");
    } while (match(TokenType::COMMA));
    return node;
}

ast::NodePtr Parser::parse_assert_stmt() {
    advance();
    auto node = std::make_unique<ast::AssertStmt>();
    node->test = parse_expression();
    if (match(TokenType::COMMA)) {
        node->msg = parse_expression();
    }
    return node;
}

ast::NodePtr Parser::parse_del_stmt() {
    advance();
    auto node = std::make_unique<ast::DeleteStmt>();
    do {
        node->targets.push_back(parse_expression());
    } while (match(TokenType::COMMA));
    return node;
}

ast::NodePtr Parser::parse_assign_or_expr() {
    auto expr = parse_expression();

    if (match(TokenType::ASSIGN)) {
        auto node = std::make_unique<ast::AssignStmt>();
        node->targets.push_back(std::move(expr));

        while (true) {
            auto value = parse_expression();
            if (match(TokenType::ASSIGN)) {
                node->targets.push_back(std::move(value));
            } else {
                node->value = std::move(value);
                break;
            }
        }
        return node;
    }

    if (current_.is_assignment() && current_.type != TokenType::ASSIGN &&
        current_.type != TokenType::WALRUS) {
        auto node = std::make_unique<ast::AugAssignStmt>();
        node->target = std::move(expr);
        node->op = current_.type;
        advance();
        node->value = parse_expression();
        return node;
    }

    if (match(TokenType::COLON)) {
        auto node = std::make_unique<ast::AnnAssignStmt>();
        node->target = std::move(expr);
        node->annotation = parse_expression();
        if (match(TokenType::ASSIGN)) {
            node->value = parse_expression();
        }
        return node;
    }

    auto stmt = std::make_unique<ast::ExprStmt>();
    stmt->value = std::move(expr);
    return stmt;
}

// --- Expression parsing (precedence climbing) ---

ast::NodePtr Parser::parse_expression() {
    if (check(TokenType::KW_LAMBDA)) return parse_lambda();
    return parse_assignment_expr();
}

ast::NodePtr Parser::parse_assignment_expr() {
    auto expr = parse_ternary();
    if (match(TokenType::WALRUS)) {
        auto node = std::make_unique<ast::NamedExpr>();
        node->target = std::move(expr);
        node->value = parse_ternary();
        return node;
    }
    return expr;
}

ast::NodePtr Parser::parse_ternary() {
    auto expr = parse_or_expr();
    if (match(TokenType::KW_IF)) {
        auto node = std::make_unique<ast::IfExpr>();
        node->body = std::move(expr);
        node->test = parse_or_expr();
        consume(TokenType::KW_ELSE, "expected 'else' in ternary");
        node->orelse = parse_ternary();
        return node;
    }
    return expr;
}

ast::NodePtr Parser::parse_or_expr() {
    auto left = parse_and_expr();
    while (match(TokenType::KW_OR)) {
        auto node = std::make_unique<ast::BoolOpExpr>();
        node->op = ast::BoolOpKind::Or;
        node->values.push_back(std::move(left));
        node->values.push_back(parse_and_expr());
        left = std::move(node);
    }
    return left;
}

ast::NodePtr Parser::parse_and_expr() {
    auto left = parse_not_expr();
    while (match(TokenType::KW_AND)) {
        auto node = std::make_unique<ast::BoolOpExpr>();
        node->op = ast::BoolOpKind::And;
        node->values.push_back(std::move(left));
        node->values.push_back(parse_not_expr());
        left = std::move(node);
    }
    return left;
}

ast::NodePtr Parser::parse_not_expr() {
    if (match(TokenType::KW_NOT)) {
        auto node = std::make_unique<ast::UnaryOpExpr>();
        node->op = ast::UnaryOpKind::Not;
        node->operand = parse_not_expr();
        return node;
    }
    return parse_comparison();
}

ast::NodePtr Parser::parse_comparison() {
    auto left = parse_bitor();

    std::vector<ast::CmpOp> ops;
    ast::NodeList comparators;

    while (true) {
        ast::CmpOp op;
        if (match(TokenType::EQ)) op = ast::CmpOp::Eq;
        else if (match(TokenType::NEQ)) op = ast::CmpOp::NotEq;
        else if (match(TokenType::LT)) op = ast::CmpOp::Lt;
        else if (match(TokenType::GT)) op = ast::CmpOp::Gt;
        else if (match(TokenType::LTE)) op = ast::CmpOp::LtE;
        else if (match(TokenType::GTE)) op = ast::CmpOp::GtE;
        else if (match(TokenType::KW_IN)) op = ast::CmpOp::In;
        else if (check(TokenType::KW_NOT) && peek_is_in()) {
            advance(); advance();
            op = ast::CmpOp::NotIn;
        }
        else if (match(TokenType::KW_IS)) {
            if (match(TokenType::KW_NOT)) op = ast::CmpOp::IsNot;
            else op = ast::CmpOp::Is;
        }
        else break;

        ops.push_back(op);
        comparators.push_back(parse_bitor());
    }

    if (ops.empty()) return left;

    auto cmp = std::make_unique<ast::CompareExpr>();
    cmp->left = std::move(left);
    cmp->ops = std::move(ops);
    cmp->comparators = std::move(comparators);
    return cmp;
}

ast::NodePtr Parser::parse_bitor() {
    auto left = parse_bitxor();
    while (match(TokenType::PIPE)) {
        auto node = std::make_unique<ast::BinOpExpr>();
        node->left = std::move(left);
        node->op = TokenType::PIPE;
        node->right = parse_bitxor();
        left = std::move(node);
    }
    return left;
}

ast::NodePtr Parser::parse_bitxor() {
    auto left = parse_bitand();
    while (match(TokenType::CARET)) {
        auto node = std::make_unique<ast::BinOpExpr>();
        node->left = std::move(left);
        node->op = TokenType::CARET;
        node->right = parse_bitand();
        left = std::move(node);
    }
    return left;
}

ast::NodePtr Parser::parse_bitand() {
    auto left = parse_shift();
    while (match(TokenType::AMPERSAND)) {
        auto node = std::make_unique<ast::BinOpExpr>();
        node->left = std::move(left);
        node->op = TokenType::AMPERSAND;
        node->right = parse_shift();
        left = std::move(node);
    }
    return left;
}

ast::NodePtr Parser::parse_shift() {
    auto left = parse_arith();
    while (check(TokenType::LSHIFT) || check(TokenType::RSHIFT)) {
        auto op = current_.type;
        advance();
        auto node = std::make_unique<ast::BinOpExpr>();
        node->left = std::move(left);
        node->op = op;
        node->right = parse_arith();
        left = std::move(node);
    }
    return left;
}

ast::NodePtr Parser::parse_arith() {
    auto left = parse_term();
    while (check(TokenType::PLUS) || check(TokenType::MINUS)) {
        auto op = current_.type;
        advance();
        auto node = std::make_unique<ast::BinOpExpr>();
        node->left = std::move(left);
        node->op = op;
        node->right = parse_term();
        left = std::move(node);
    }
    return left;
}

ast::NodePtr Parser::parse_term() {
    auto left = parse_factor();
    while (check(TokenType::STAR) || check(TokenType::SLASH) ||
           check(TokenType::DOUBLE_SLASH) || check(TokenType::PERCENT) ||
           check(TokenType::AT)) {
        auto op = current_.type;
        advance();
        auto node = std::make_unique<ast::BinOpExpr>();
        node->left = std::move(left);
        node->op = op;
        node->right = parse_factor();
        left = std::move(node);
    }
    return left;
}

ast::NodePtr Parser::parse_factor() {
    if (check(TokenType::PLUS) || check(TokenType::MINUS) || check(TokenType::TILDE)) {
        auto node = std::make_unique<ast::UnaryOpExpr>();
        if (current_.type == TokenType::PLUS) node->op = ast::UnaryOpKind::UAdd;
        else if (current_.type == TokenType::MINUS) node->op = ast::UnaryOpKind::USub;
        else node->op = ast::UnaryOpKind::Invert;
        advance();
        node->operand = parse_factor();
        return node;
    }
    return parse_power();
}

ast::NodePtr Parser::parse_power() {
    auto base = parse_await_expr();
    if (match(TokenType::DOUBLE_STAR)) {
        auto node = std::make_unique<ast::BinOpExpr>();
        node->left = std::move(base);
        node->op = TokenType::DOUBLE_STAR;
        node->right = parse_factor(); // right-associative
        return node;
    }
    return base;
}

ast::NodePtr Parser::parse_await_expr() {
    if (match(TokenType::KW_AWAIT)) {
        auto node = std::make_unique<ast::AwaitExpr>();
        node->value = parse_primary();
        return node;
    }
    return parse_primary();
}

ast::NodePtr Parser::parse_primary() {
    auto atom = parse_atom();

    while (true) {
        if (check(TokenType::LPAREN)) {
            atom = parse_call(std::move(atom));
        } else if (check(TokenType::LBRACKET)) {
            atom = parse_subscript(std::move(atom));
        } else if (match(TokenType::DOT)) {
            atom = parse_attribute(std::move(atom));
        } else {
            break;
        }
    }

    return atom;
}

ast::NodePtr Parser::parse_atom() {
    if (check(TokenType::INTEGER_LIT)) {
        auto node = std::make_unique<ast::ConstantExpr>();
        node->const_kind = ast::ConstantKind::Int;
        node->value = current_.value;
        advance();
        return node;
    }
    if (check(TokenType::FLOAT_LIT)) {
        auto node = std::make_unique<ast::ConstantExpr>();
        node->const_kind = ast::ConstantKind::Float;
        node->value = current_.value;
        advance();
        return node;
    }
    if (check(TokenType::STRING_LIT)) {
        auto node = std::make_unique<ast::ConstantExpr>();
        node->const_kind = ast::ConstantKind::Str;
        node->value = current_.value;
        advance();
        return node;
    }
    if (check(TokenType::FSTRING_LIT)) {
        auto node = std::make_unique<ast::ConstantExpr>();
        node->const_kind = ast::ConstantKind::Str;
        node->value = current_.value;
        advance();
        return node;
    }
    if (check(TokenType::BOOL_TRUE)) {
        auto node = std::make_unique<ast::ConstantExpr>();
        node->const_kind = ast::ConstantKind::Bool;
        node->value = "True";
        advance();
        return node;
    }
    if (check(TokenType::BOOL_FALSE)) {
        auto node = std::make_unique<ast::ConstantExpr>();
        node->const_kind = ast::ConstantKind::Bool;
        node->value = "False";
        advance();
        return node;
    }
    if (check(TokenType::NONE_LIT)) {
        auto node = std::make_unique<ast::ConstantExpr>();
        node->const_kind = ast::ConstantKind::NoneVal;
        node->value = "None";
        advance();
        return node;
    }
    if (check(TokenType::IDENTIFIER)) {
        auto node = std::make_unique<ast::NameExpr>();
        node->id = current_.value;
        advance();
        return node;
    }
    if (check(TokenType::LPAREN)) return parse_tuple_or_paren();
    if (check(TokenType::LBRACKET)) return parse_list_expr();
    if (check(TokenType::LBRACE)) return parse_dict_or_set();
    if (check(TokenType::STAR)) return parse_star_expr();
    if (check(TokenType::KW_YIELD)) return parse_yield_expr();
    if (check(TokenType::ELLIPSIS)) {
        advance();
        auto node = std::make_unique<ast::ConstantExpr>();
        node->const_kind = ast::ConstantKind::Ellipsis;
        node->value = "...";
        return node;
    }

    error("expected expression");
    advance();
    return std::make_unique<ast::ConstantExpr>();
}

ast::NodePtr Parser::parse_call(ast::NodePtr func) {
    advance(); // '('
    auto node = std::make_unique<ast::CallExpr>();
    node->func = std::move(func);

    while (!check(TokenType::RPAREN) && !check(TokenType::END_OF_FILE)) {
        if (check(TokenType::DOUBLE_STAR)) {
            advance();
            ast::KeywordArg kw;
            kw.value = parse_expression();
            node->keywords.push_back(std::move(kw));
        } else {
            auto expr = parse_expression();
            if (match(TokenType::ASSIGN) && expr->kind == ast::NodeKind::Name) {
                ast::KeywordArg kw;
                kw.arg = static_cast<ast::NameExpr*>(expr.get())->id;
                kw.value = parse_expression();
                node->keywords.push_back(std::move(kw));
            } else {
                node->args.push_back(std::move(expr));
            }
        }
        if (!match(TokenType::COMMA)) break;
    }

    consume(TokenType::RPAREN, "expected ')'");
    return node;
}

ast::NodePtr Parser::parse_subscript(ast::NodePtr value) {
    advance(); // '['
    auto node = std::make_unique<ast::SubscriptExpr>();
    node->value = std::move(value);

    if (check(TokenType::COLON)) {
        auto slice = std::make_unique<ast::SliceExpr>();
        advance();
        if (!check(TokenType::RBRACKET) && !check(TokenType::COLON)) {
            slice->upper = parse_expression();
        }
        if (match(TokenType::COLON)) {
            if (!check(TokenType::RBRACKET)) {
                slice->step = parse_expression();
            }
        }
        node->slice = std::move(slice);
    } else {
        auto idx = parse_expression();
        if (check(TokenType::COLON)) {
            auto slice = std::make_unique<ast::SliceExpr>();
            slice->lower = std::move(idx);
            advance();
            if (!check(TokenType::RBRACKET) && !check(TokenType::COLON)) {
                slice->upper = parse_expression();
            }
            if (match(TokenType::COLON)) {
                if (!check(TokenType::RBRACKET)) {
                    slice->step = parse_expression();
                }
            }
            node->slice = std::move(slice);
        } else {
            node->slice = std::move(idx);
        }
    }

    consume(TokenType::RBRACKET, "expected ']'");
    return node;
}

ast::NodePtr Parser::parse_attribute(ast::NodePtr value) {
    auto node = std::make_unique<ast::AttributeExpr>();
    node->value = std::move(value);
    node->attr = current_.value;
    consume(TokenType::IDENTIFIER, "expected attribute name");
    return node;
}

ast::NodePtr Parser::parse_list_expr() {
    advance(); // '['
    if (check(TokenType::RBRACKET)) {
        advance();
        return std::make_unique<ast::ListExpr>();
    }

    auto first = parse_expression();

    if (check(TokenType::KW_FOR)) {
        return parse_comprehension(std::move(first));
    }

    auto node = std::make_unique<ast::ListExpr>();
    node->elts.push_back(std::move(first));
    while (match(TokenType::COMMA)) {
        if (check(TokenType::RBRACKET)) break;
        node->elts.push_back(parse_expression());
    }
    consume(TokenType::RBRACKET, "expected ']'");
    return node;
}

ast::NodePtr Parser::parse_dict_or_set() {
    advance(); // '{'
    if (check(TokenType::RBRACE)) {
        advance();
        return std::make_unique<ast::DictExpr>();
    }

    auto first = parse_expression();

    if (match(TokenType::COLON)) {
        auto dict = std::make_unique<ast::DictExpr>();
        dict->keys.push_back(std::move(first));
        dict->values.push_back(parse_expression());
        while (match(TokenType::COMMA)) {
            if (check(TokenType::RBRACE)) break;
            dict->keys.push_back(parse_expression());
            consume(TokenType::COLON, "expected ':'");
            dict->values.push_back(parse_expression());
        }
        consume(TokenType::RBRACE, "expected '}'");
        return dict;
    }

    auto set_node = std::make_unique<ast::SetExpr>();
    set_node->elts.push_back(std::move(first));
    while (match(TokenType::COMMA)) {
        if (check(TokenType::RBRACE)) break;
        set_node->elts.push_back(parse_expression());
    }
    consume(TokenType::RBRACE, "expected '}'");
    return set_node;
}

ast::NodePtr Parser::parse_tuple_or_paren() {
    advance(); // '('
    if (check(TokenType::RPAREN)) {
        advance();
        return std::make_unique<ast::TupleExpr>();
    }

    auto first = parse_expression();

    if (match(TokenType::COMMA)) {
        auto tuple = std::make_unique<ast::TupleExpr>();
        tuple->elts.push_back(std::move(first));
        while (!check(TokenType::RPAREN) && !check(TokenType::END_OF_FILE)) {
            tuple->elts.push_back(parse_expression());
            if (!match(TokenType::COMMA)) break;
        }
        consume(TokenType::RPAREN, "expected ')'");
        return tuple;
    }

    consume(TokenType::RPAREN, "expected ')'");
    return first;
}

ast::NodePtr Parser::parse_lambda() {
    advance(); // 'lambda'
    auto node = std::make_unique<ast::LambdaExpr>();
    if (!check(TokenType::COLON)) {
        node->params = parse_parameters();
    }
    consume(TokenType::COLON, "expected ':'");
    node->body = parse_expression();
    return node;
}

ast::NodePtr Parser::parse_star_expr() {
    advance(); // '*'
    auto node = std::make_unique<ast::StarredExpr>();
    node->value = parse_expression();
    return node;
}

ast::NodePtr Parser::parse_yield_expr() {
    advance(); // 'yield'
    auto node = std::make_unique<ast::YieldExpr>();
    if (match(TokenType::KW_FROM)) {
        node->is_from = true;
        node->value = parse_expression();
    } else if (!check(TokenType::NEWLINE) && !check(TokenType::RPAREN)) {
        node->value = parse_expression();
    }
    return node;
}

ast::NodePtr Parser::parse_comprehension(ast::NodePtr first_expr) {
    auto node = std::make_unique<ast::ListCompExpr>();
    node->elt = std::move(first_expr);
    while (check(TokenType::KW_FOR)) {
        node->generators.push_back(parse_comp_for());
    }
    consume(TokenType::RBRACKET, "expected ']'");
    return node;
}

ast::ComprehensionNode Parser::parse_comp_for() {
    ast::ComprehensionNode comp;
    consume(TokenType::KW_FOR, "expected 'for'");
    comp.target = parse_expression();
    consume(TokenType::KW_IN, "expected 'in'");
    comp.iter = parse_or_expr();
    while (match(TokenType::KW_IF)) {
        comp.ifs.push_back(parse_or_expr());
    }
    return comp;
}

} // namespace mero
