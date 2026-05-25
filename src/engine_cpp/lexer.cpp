/**
 * MERO Compiler - Python Lexer Implementation
 * Developer: MERO:TG@QP4RM
 */
#include "lexer.h"
#include <cctype>
#include <algorithm>
#include <cstring>

namespace mero {

Lexer::Lexer(const std::string& source, const std::string& filename)
    : source_(source), filename_(filename) {
    mero_error_ctx_init(&error_ctx_);
    indent_stack_.push(0);
}

char Lexer::current() const {
    if (pos_ >= source_.size()) return '\0';
    return source_[pos_];
}

char Lexer::peek(size_t ahead) const {
    size_t idx = pos_ + ahead;
    if (idx >= source_.size()) return '\0';
    return source_[idx];
}

char Lexer::advance() {
    char c = current();
    pos_++;
    if (c == '\n') {
        line_++;
        column_ = 1;
    } else {
        column_++;
    }
    return c;
}

bool Lexer::match(char expected) {
    if (current() == expected) {
        advance();
        return true;
    }
    return false;
}

bool Lexer::is_at_end() const {
    return pos_ >= source_.size();
}

SourcePosition Lexer::current_pos() const {
    SourcePosition p;
    p.line = static_cast<uint32_t>(line_);
    p.column = static_cast<uint32_t>(column_);
    p.offset = static_cast<uint32_t>(pos_);
    return p;
}

void Lexer::report_error(MeroErrorCode code, const char* msg) {
    MeroSourceLoc loc = {};
    loc.file = filename_.c_str();
    loc.line = static_cast<uint32_t>(line_);
    loc.column = static_cast<uint32_t>(column_);
    mero_error(&error_ctx_, code, loc, "%s", msg);
}

Token Lexer::make_token(TokenType type, const std::string& value, SourcePosition start) {
    Token tok;
    tok.type = type;
    tok.value = value;
    tok.start = start;
    tok.end = current_pos();
    return tok;
}

void Lexer::skip_comment() {
    while (!is_at_end() && current() != '\n') {
        advance();
    }
}

void Lexer::skip_whitespace_inline() {
    while (!is_at_end() && (current() == ' ' || current() == '\t') && current() != '\n') {
        advance();
    }
}

void Lexer::handle_indentation() {
    int indent = 0;
    while (!is_at_end()) {
        if (current() == ' ') {
            indent++;
            advance();
        } else if (current() == '\t') {
            indent += 4;
            advance();
        } else {
            break;
        }
    }

    if (is_at_end() || current() == '\n' || current() == '#') {
        return;
    }

    int current_indent = indent_stack_.top();

    if (indent > current_indent) {
        indent_stack_.push(indent);
        Token tok;
        tok.type = TokenType::INDENT;
        tok.start = current_pos();
        tok.end = current_pos();
        pending_tokens_.push_back(tok);
    } else {
        while (indent < indent_stack_.top()) {
            indent_stack_.pop();
            Token tok;
            tok.type = TokenType::DEDENT;
            tok.start = current_pos();
            tok.end = current_pos();
            pending_tokens_.push_back(tok);
        }
        if (indent != indent_stack_.top()) {
            report_error(MERO_ERR_INDENT_ERROR, "inconsistent indentation");
        }
    }
}

TokenType Lexer::check_keyword(const std::string& word) {
    static const struct { const char* str; TokenType type; } keywords[] = {
        {"and", TokenType::KW_AND}, {"as", TokenType::KW_AS},
        {"assert", TokenType::KW_ASSERT}, {"async", TokenType::KW_ASYNC},
        {"await", TokenType::KW_AWAIT}, {"break", TokenType::KW_BREAK},
        {"class", TokenType::KW_CLASS}, {"continue", TokenType::KW_CONTINUE},
        {"def", TokenType::KW_DEF}, {"del", TokenType::KW_DEL},
        {"elif", TokenType::KW_ELIF}, {"else", TokenType::KW_ELSE},
        {"except", TokenType::KW_EXCEPT}, {"finally", TokenType::KW_FINALLY},
        {"for", TokenType::KW_FOR}, {"from", TokenType::KW_FROM},
        {"global", TokenType::KW_GLOBAL}, {"if", TokenType::KW_IF},
        {"import", TokenType::KW_IMPORT}, {"in", TokenType::KW_IN},
        {"is", TokenType::KW_IS}, {"lambda", TokenType::KW_LAMBDA},
        {"nonlocal", TokenType::KW_NONLOCAL}, {"not", TokenType::KW_NOT},
        {"or", TokenType::KW_OR}, {"pass", TokenType::KW_PASS},
        {"raise", TokenType::KW_RAISE}, {"return", TokenType::KW_RETURN},
        {"try", TokenType::KW_TRY}, {"while", TokenType::KW_WHILE},
        {"with", TokenType::KW_WITH}, {"yield", TokenType::KW_YIELD},
        {"True", TokenType::BOOL_TRUE}, {"False", TokenType::BOOL_FALSE},
        {"None", TokenType::NONE_LIT},
    };

    for (const auto& kw : keywords) {
        if (word == kw.str) return kw.type;
    }
    return TokenType::IDENTIFIER;
}

Token Lexer::scan_number() {
    SourcePosition start = current_pos();
    std::string num;
    bool is_float = false;

    if (current() == '0' && (peek() == 'x' || peek() == 'X')) {
        num += advance(); num += advance();
        while (std::isxdigit(current()) || current() == '_') {
            if (current() != '_') num += current();
            advance();
        }
        return make_token(TokenType::INTEGER_LIT, num, start);
    }

    if (current() == '0' && (peek() == 'o' || peek() == 'O')) {
        num += advance(); num += advance();
        while ((current() >= '0' && current() <= '7') || current() == '_') {
            if (current() != '_') num += current();
            advance();
        }
        return make_token(TokenType::INTEGER_LIT, num, start);
    }

    if (current() == '0' && (peek() == 'b' || peek() == 'B')) {
        num += advance(); num += advance();
        while (current() == '0' || current() == '1' || current() == '_') {
            if (current() != '_') num += current();
            advance();
        }
        return make_token(TokenType::INTEGER_LIT, num, start);
    }

    while (std::isdigit(current()) || current() == '_') {
        if (current() != '_') num += current();
        advance();
    }

    if (current() == '.' && std::isdigit(peek())) {
        is_float = true;
        num += advance();
        while (std::isdigit(current()) || current() == '_') {
            if (current() != '_') num += current();
            advance();
        }
    }

    if (current() == 'e' || current() == 'E') {
        is_float = true;
        num += advance();
        if (current() == '+' || current() == '-') num += advance();
        while (std::isdigit(current())) {
            num += advance();
        }
    }

    return make_token(is_float ? TokenType::FLOAT_LIT : TokenType::INTEGER_LIT, num, start);
}

std::string Lexer::scan_escape_sequence() {
    advance(); // backslash
    char c = advance();
    switch (c) {
        case 'n': return "\n";
        case 't': return "\t";
        case 'r': return "\r";
        case '\\': return "\\";
        case '\'': return "'";
        case '"': return "\"";
        case '0': return std::string(1, '\0');
        case 'a': return "\a";
        case 'b': return "\b";
        case 'f': return "\f";
        case 'v': return "\v";
        case 'x': {
            std::string hex;
            for (int i = 0; i < 2 && std::isxdigit(current()); i++) {
                hex += advance();
            }
            return std::string(1, static_cast<char>(std::stoi(hex, nullptr, 16)));
        }
        default:
            return std::string(1, c);
    }
}

bool Lexer::is_triple_quote(char quote) {
    return pos_ + 2 < source_.size() &&
           source_[pos_] == quote &&
           source_[pos_ + 1] == quote &&
           source_[pos_ + 2] == quote;
}

Token Lexer::scan_string(char quote) {
    SourcePosition start = current_pos();
    std::string result;
    bool triple = false;

    advance(); // first quote

    if (current() == quote && peek() == quote) {
        triple = true;
        advance();
        advance();
    }

    while (!is_at_end()) {
        if (triple) {
            if (current() == quote && peek() == quote && peek(2) == quote) {
                advance(); advance(); advance();
                return make_token(TokenType::STRING_LIT, result, start);
            }
        } else {
            if (current() == quote) {
                advance();
                return make_token(TokenType::STRING_LIT, result, start);
            }
            if (current() == '\n') {
                report_error(MERO_ERR_UNTERMINATED_STRING, "unterminated string literal");
                return make_token(TokenType::ERROR_TOKEN, result, start);
            }
        }

        if (current() == '\\') {
            result += scan_escape_sequence();
        } else {
            result += advance();
        }
    }

    report_error(MERO_ERR_UNTERMINATED_STRING, "unterminated string literal");
    return make_token(TokenType::ERROR_TOKEN, result, start);
}

Token Lexer::scan_fstring(char quote) {
    SourcePosition start = current_pos();
    advance(); // 'f'
    std::string result;
    advance(); // opening quote

    int brace_depth = 0;
    while (!is_at_end() && (current() != quote || brace_depth > 0)) {
        if (current() == '{') {
            if (peek() == '{') {
                result += '{';
                advance(); advance();
            } else {
                brace_depth++;
                result += advance();
            }
        } else if (current() == '}') {
            if (peek() == '}') {
                result += '}';
                advance(); advance();
            } else {
                brace_depth--;
                result += advance();
            }
        } else if (current() == '\\') {
            result += scan_escape_sequence();
        } else {
            result += advance();
        }
    }

    if (!is_at_end()) advance(); // closing quote
    return make_token(TokenType::FSTRING_LIT, result, start);
}

Token Lexer::scan_identifier() {
    SourcePosition start = current_pos();
    std::string word;

    while (!is_at_end() && (std::isalnum(current()) || current() == '_')) {
        word += advance();
    }

    TokenType type = check_keyword(word);
    return make_token(type, word, start);
}

Token Lexer::scan_token() {
    if (at_line_start_ && paren_depth_ == 0) {
        at_line_start_ = false;
        handle_indentation();
        if (!pending_tokens_.empty()) {
            Token tok = pending_tokens_.front();
            pending_tokens_.erase(pending_tokens_.begin());
            return tok;
        }
    }

    skip_whitespace_inline();

    if (is_at_end()) {
        while (indent_stack_.size() > 1) {
            indent_stack_.pop();
            Token tok;
            tok.type = TokenType::DEDENT;
            tok.start = current_pos();
            tok.end = current_pos();
            pending_tokens_.push_back(tok);
        }
        if (!pending_tokens_.empty()) {
            Token tok = pending_tokens_.front();
            pending_tokens_.erase(pending_tokens_.begin());
            return tok;
        }
        return make_token(TokenType::END_OF_FILE, "", current_pos());
    }

    if (current() == '#') {
        skip_comment();
    }

    if (current() == '\n') {
        SourcePosition start = current_pos();
        advance();
        at_line_start_ = true;
        if (paren_depth_ > 0) {
            return scan_token();
        }
        return make_token(TokenType::NEWLINE, "\\n", start);
    }

    if (current() == '\\' && peek() == '\n') {
        advance(); advance();
        return scan_token();
    }

    SourcePosition start = current_pos();
    char c = current();

    // Numbers
    if (std::isdigit(c)) {
        return scan_number();
    }

    // Strings
    if (c == 'f' && (peek() == '\'' || peek() == '"')) {
        return scan_fstring(peek());
    }
    if (c == 'r' && (peek() == '\'' || peek() == '"')) {
        advance();
        return scan_string(current());
    }
    if (c == 'b' && (peek() == '\'' || peek() == '"')) {
        advance();
        Token tok = scan_string(current());
        tok.type = TokenType::BYTES_LIT;
        return tok;
    }
    if (c == '\'' || c == '"') {
        return scan_string(c);
    }

    // Identifiers and keywords
    if (std::isalpha(c) || c == '_') {
        return scan_identifier();
    }

    // Operators and delimiters
    advance();
    switch (c) {
        case '(':
            paren_depth_++;
            return make_token(TokenType::LPAREN, "(", start);
        case ')':
            paren_depth_--;
            return make_token(TokenType::RPAREN, ")", start);
        case '[':
            paren_depth_++;
            return make_token(TokenType::LBRACKET, "[", start);
        case ']':
            paren_depth_--;
            return make_token(TokenType::RBRACKET, "]", start);
        case '{':
            paren_depth_++;
            return make_token(TokenType::LBRACE, "{", start);
        case '}':
            paren_depth_--;
            return make_token(TokenType::RBRACE, "}", start);
        case ',':
            return make_token(TokenType::COMMA, ",", start);
        case ';':
            return make_token(TokenType::SEMICOLON, ";", start);
        case '~':
            return make_token(TokenType::TILDE, "~", start);
        case ':':
            if (match('=')) return make_token(TokenType::WALRUS, ":=", start);
            return make_token(TokenType::COLON, ":", start);
        case '.':
            if (current() == '.' && peek() == '.') {
                advance(); advance();
                return make_token(TokenType::ELLIPSIS, "...", start);
            }
            return make_token(TokenType::DOT, ".", start);
        case '+':
            if (match('=')) return make_token(TokenType::PLUS_ASSIGN, "+=", start);
            return make_token(TokenType::PLUS, "+", start);
        case '-':
            if (match('=')) return make_token(TokenType::MINUS_ASSIGN, "-=", start);
            if (match('>')) return make_token(TokenType::ARROW, "->", start);
            return make_token(TokenType::MINUS, "-", start);
        case '*':
            if (match('*')) {
                if (match('=')) return make_token(TokenType::DSTAR_ASSIGN, "**=", start);
                return make_token(TokenType::DOUBLE_STAR, "**", start);
            }
            if (match('=')) return make_token(TokenType::STAR_ASSIGN, "*=", start);
            return make_token(TokenType::STAR, "*", start);
        case '/':
            if (match('/')) {
                if (match('=')) return make_token(TokenType::DSLASH_ASSIGN, "//=", start);
                return make_token(TokenType::DOUBLE_SLASH, "//", start);
            }
            if (match('=')) return make_token(TokenType::SLASH_ASSIGN, "/=", start);
            return make_token(TokenType::SLASH, "/", start);
        case '%':
            if (match('=')) return make_token(TokenType::PERCENT_ASSIGN, "%=", start);
            return make_token(TokenType::PERCENT, "%", start);
        case '&':
            if (match('=')) return make_token(TokenType::AMP_ASSIGN, "&=", start);
            return make_token(TokenType::AMPERSAND, "&", start);
        case '|':
            if (match('=')) return make_token(TokenType::PIPE_ASSIGN, "|=", start);
            return make_token(TokenType::PIPE, "|", start);
        case '^':
            if (match('=')) return make_token(TokenType::CARET_ASSIGN, "^=", start);
            return make_token(TokenType::CARET, "^", start);
        case '@':
            if (match('=')) return make_token(TokenType::AT_ASSIGN, "@=", start);
            return make_token(TokenType::AT, "@", start);
        case '=':
            if (match('=')) return make_token(TokenType::EQ, "==", start);
            return make_token(TokenType::ASSIGN, "=", start);
        case '!':
            if (match('=')) return make_token(TokenType::NEQ, "!=", start);
            report_error(MERO_ERR_UNEXPECTED_CHAR, "unexpected '!'");
            return make_token(TokenType::ERROR_TOKEN, "!", start);
        case '<':
            if (match('<')) {
                if (match('=')) return make_token(TokenType::LSHIFT_ASSIGN, "<<=", start);
                return make_token(TokenType::LSHIFT, "<<", start);
            }
            if (match('=')) return make_token(TokenType::LTE, "<=", start);
            return make_token(TokenType::LT, "<", start);
        case '>':
            if (match('>')) {
                if (match('=')) return make_token(TokenType::RSHIFT_ASSIGN, ">>=", start);
                return make_token(TokenType::RSHIFT, ">>", start);
            }
            if (match('=')) return make_token(TokenType::GTE, ">=", start);
            return make_token(TokenType::GT, ">", start);
        default:
            report_error(MERO_ERR_UNEXPECTED_CHAR, "unexpected character");
            return make_token(TokenType::ERROR_TOKEN, std::string(1, c), start);
    }
}

Token Lexer::next_token() {
    if (!pending_tokens_.empty()) {
        Token tok = pending_tokens_.front();
        pending_tokens_.erase(pending_tokens_.begin());
        return tok;
    }
    return scan_token();
}

Token Lexer::peek_token() {
    if (!pending_tokens_.empty()) {
        return pending_tokens_.front();
    }

    size_t saved_pos = pos_;
    size_t saved_line = line_;
    size_t saved_col = column_;
    bool saved_at_line_start = at_line_start_;

    Token tok = scan_token();

    pos_ = saved_pos;
    line_ = saved_line;
    column_ = saved_col;
    at_line_start_ = saved_at_line_start;

    return tok;
}

TokenType Lexer::peek_type() {
    return peek_token().type;
}

std::vector<Token> Lexer::tokenize_all() {
    std::vector<Token> tokens;
    while (true) {
        Token tok = next_token();
        tokens.push_back(tok);
        if (tok.type == TokenType::END_OF_FILE) break;
    }
    return tokens;
}

const char* token_type_to_string(TokenType type) {
    switch (type) {
        case TokenType::INTEGER_LIT: return "INTEGER";
        case TokenType::FLOAT_LIT: return "FLOAT";
        case TokenType::STRING_LIT: return "STRING";
        case TokenType::FSTRING_LIT: return "FSTRING";
        case TokenType::BYTES_LIT: return "BYTES";
        case TokenType::BOOL_TRUE: return "TRUE";
        case TokenType::BOOL_FALSE: return "FALSE";
        case TokenType::NONE_LIT: return "NONE";
        case TokenType::IDENTIFIER: return "IDENTIFIER";
        case TokenType::PLUS: return "PLUS";
        case TokenType::MINUS: return "MINUS";
        case TokenType::STAR: return "STAR";
        case TokenType::SLASH: return "SLASH";
        case TokenType::DOUBLE_SLASH: return "DOUBLE_SLASH";
        case TokenType::PERCENT: return "PERCENT";
        case TokenType::DOUBLE_STAR: return "DOUBLE_STAR";
        case TokenType::EQ: return "EQ";
        case TokenType::NEQ: return "NEQ";
        case TokenType::LT: return "LT";
        case TokenType::GT: return "GT";
        case TokenType::LTE: return "LTE";
        case TokenType::GTE: return "GTE";
        case TokenType::ASSIGN: return "ASSIGN";
        case TokenType::LPAREN: return "LPAREN";
        case TokenType::RPAREN: return "RPAREN";
        case TokenType::LBRACKET: return "LBRACKET";
        case TokenType::RBRACKET: return "RBRACKET";
        case TokenType::LBRACE: return "LBRACE";
        case TokenType::RBRACE: return "RBRACE";
        case TokenType::COMMA: return "COMMA";
        case TokenType::COLON: return "COLON";
        case TokenType::DOT: return "DOT";
        case TokenType::ARROW: return "ARROW";
        case TokenType::INDENT: return "INDENT";
        case TokenType::DEDENT: return "DEDENT";
        case TokenType::NEWLINE: return "NEWLINE";
        case TokenType::END_OF_FILE: return "EOF";
        case TokenType::KW_DEF: return "DEF";
        case TokenType::KW_CLASS: return "CLASS";
        case TokenType::KW_IF: return "IF";
        case TokenType::KW_ELSE: return "ELSE";
        case TokenType::KW_ELIF: return "ELIF";
        case TokenType::KW_FOR: return "FOR";
        case TokenType::KW_WHILE: return "WHILE";
        case TokenType::KW_RETURN: return "RETURN";
        case TokenType::KW_IMPORT: return "IMPORT";
        case TokenType::KW_FROM: return "FROM";
        default: return "UNKNOWN";
    }
}

} // namespace mero
