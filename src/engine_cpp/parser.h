/**
 * MERO Compiler - Python Parser
 * Recursive descent parser for Python source to AST.
 * Developer: MERO:TG@QP4RM
 */
#ifndef MERO_PARSER_H
#define MERO_PARSER_H

#include "lexer.h"
#include "ast_nodes.h"
#include <memory>
#include <vector>

extern "C" {
#include "mero/error.h"
}

namespace mero {

class Parser {
public:
    explicit Parser(Lexer& lexer);
    ~Parser();

    std::unique_ptr<ast::ModuleNode> parse_module();
    bool has_errors() const { return error_ctx_.error_count > 0; }
    const MeroErrorContext& errors() const { return error_ctx_; }

private:
    Lexer& lexer_;
    Token current_;
    Token previous_;
    MeroErrorContext error_ctx_;
    int loop_depth_ = 0;
    int func_depth_ = 0;

    // Token management
    Token advance();
    bool check(TokenType type) const;
    bool match(TokenType type);
    Token consume(TokenType type, const char* msg);
    void synchronize();

    void error(const char* msg);
    void error_at(const Token& tok, const char* msg);

    // Statements
    ast::NodePtr parse_statement();
    ast::NodePtr parse_simple_statement();
    ast::NodePtr parse_compound_statement();
    ast::NodePtr parse_function_def(bool is_async = false);
    ast::NodePtr parse_class_def();
    ast::NodePtr parse_if_stmt();
    ast::NodePtr parse_while_stmt();
    ast::NodePtr parse_for_stmt(bool is_async = false);
    ast::NodePtr parse_try_stmt();
    ast::NodePtr parse_with_stmt(bool is_async = false);
    ast::NodePtr parse_return_stmt();
    ast::NodePtr parse_raise_stmt();
    ast::NodePtr parse_import_stmt();
    ast::NodePtr parse_from_import_stmt();
    ast::NodePtr parse_global_stmt();
    ast::NodePtr parse_nonlocal_stmt();
    ast::NodePtr parse_assert_stmt();
    ast::NodePtr parse_del_stmt();
    ast::NodePtr parse_assign_or_expr();
    std::vector<ast::Param> parse_parameters();
    ast::NodeList parse_block();
    ast::NodeList parse_decorators();

    // Expressions (precedence climbing)
    ast::NodePtr parse_expression();
    ast::NodePtr parse_assignment_expr();
    ast::NodePtr parse_ternary();
    ast::NodePtr parse_or_expr();
    ast::NodePtr parse_and_expr();
    ast::NodePtr parse_not_expr();
    ast::NodePtr parse_comparison();
    ast::NodePtr parse_bitor();
    ast::NodePtr parse_bitxor();
    ast::NodePtr parse_bitand();
    ast::NodePtr parse_shift();
    ast::NodePtr parse_arith();
    ast::NodePtr parse_term();
    ast::NodePtr parse_factor();
    ast::NodePtr parse_power();
    ast::NodePtr parse_unary();
    ast::NodePtr parse_await_expr();
    ast::NodePtr parse_primary();
    ast::NodePtr parse_atom();
    ast::NodePtr parse_call(ast::NodePtr func);
    ast::NodePtr parse_subscript(ast::NodePtr value);
    ast::NodePtr parse_attribute(ast::NodePtr value);

    // Helpers
    bool peek_is_in() const;
    ast::NodePtr parse_list_expr();
    ast::NodePtr parse_dict_or_set();
    ast::NodePtr parse_tuple_or_paren();
    ast::NodePtr parse_lambda();
    ast::NodePtr parse_fstring();
    ast::NodePtr parse_star_expr();
    ast::NodePtr parse_yield_expr();
    ast::NodePtr parse_comprehension(ast::NodePtr first_expr);
    ast::ComprehensionNode parse_comp_for();
};

} // namespace mero

#endif // MERO_PARSER_H
