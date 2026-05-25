/**
 * MERO Compiler - Python Lexer
 * Tokenizes Python source code with indentation tracking.
 * Developer: MERO:TG@QP4RM
 */
#ifndef MERO_LEXER_H
#define MERO_LEXER_H

#include "tokens.h"
#include <string>
#include <vector>
#include <stack>

extern "C" {
#include "mero/error.h"
}

namespace mero {

class Lexer {
public:
    explicit Lexer(const std::string& source, const std::string& filename = "<stdin>");
    ~Lexer() = default;

    Token next_token();
    Token peek_token();
    TokenType peek_type();
    std::vector<Token> tokenize_all();

    bool has_errors() const { return error_ctx_.error_count > 0; }
    const MeroErrorContext& errors() const { return error_ctx_; }

private:
    std::string source_;
    std::string filename_;
    size_t pos_ = 0;
    size_t line_ = 1;
    size_t column_ = 1;
    MeroErrorContext error_ctx_;

    std::stack<int> indent_stack_;
    std::vector<Token> pending_tokens_;
    bool at_line_start_ = true;
    int paren_depth_ = 0;

    char current() const;
    char peek(size_t ahead = 1) const;
    char advance();
    bool match(char expected);
    bool is_at_end() const;
    void skip_comment();
    void skip_whitespace_inline();

    SourcePosition current_pos() const;
    void report_error(MeroErrorCode code, const char* msg);

    Token make_token(TokenType type, const std::string& value, SourcePosition start);

    Token scan_token();
    Token scan_number();
    Token scan_string(char quote);
    Token scan_fstring(char quote);
    Token scan_identifier();
    void handle_indentation();

    TokenType check_keyword(const std::string& word);
    bool is_triple_quote(char quote);
    std::string scan_escape_sequence();
};

} // namespace mero

#endif // MERO_LEXER_H
