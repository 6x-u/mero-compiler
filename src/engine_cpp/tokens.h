/**
 * MERO Compiler - Token Definitions
 * Complete Python token set for lexical analysis.
 * Developer: MERO:TG@QP4RM
 */
#ifndef MERO_TOKENS_H
#define MERO_TOKENS_H

#include <string>
#include <cstdint>

namespace mero {

enum class TokenType : uint16_t {
    // Literals
    INTEGER_LIT,
    FLOAT_LIT,
    STRING_LIT,
    FSTRING_LIT,
    BYTES_LIT,
    BOOL_TRUE,
    BOOL_FALSE,
    NONE_LIT,

    // Identifier
    IDENTIFIER,

    // Operators
    PLUS,           // +
    MINUS,          // -
    STAR,           // *
    SLASH,          // /
    DOUBLE_SLASH,   // //
    PERCENT,        // %
    DOUBLE_STAR,    // **
    AMPERSAND,      // &
    PIPE,           // |
    CARET,          // ^
    TILDE,          // ~
    LSHIFT,         // <<
    RSHIFT,         // >>
    AT,             // @

    // Comparison
    EQ,             // ==
    NEQ,            // !=
    LT,             // <
    GT,             // >
    LTE,            // <=
    GTE,            // >=

    // Assignment
    ASSIGN,         // =
    PLUS_ASSIGN,    // +=
    MINUS_ASSIGN,   // -=
    STAR_ASSIGN,    // *=
    SLASH_ASSIGN,   // /=
    DSLASH_ASSIGN,  // //=
    PERCENT_ASSIGN, // %=
    DSTAR_ASSIGN,   // **=
    AMP_ASSIGN,     // &=
    PIPE_ASSIGN,    // |=
    CARET_ASSIGN,   // ^=
    LSHIFT_ASSIGN,  // <<=
    RSHIFT_ASSIGN,  // >>=
    AT_ASSIGN,      // @=
    WALRUS,         // :=

    // Delimiters
    LPAREN,         // (
    RPAREN,         // )
    LBRACKET,       // [
    RBRACKET,       // ]
    LBRACE,         // {
    RBRACE,         // }
    COMMA,          // ,
    COLON,          // :
    SEMICOLON,      // ;
    DOT,            // .
    ELLIPSIS,       // ...
    ARROW,          // ->

    // Keywords
    KW_AND,
    KW_AS,
    KW_ASSERT,
    KW_ASYNC,
    KW_AWAIT,
    KW_BREAK,
    KW_CLASS,
    KW_CONTINUE,
    KW_DEF,
    KW_DEL,
    KW_ELIF,
    KW_ELSE,
    KW_EXCEPT,
    KW_FINALLY,
    KW_FOR,
    KW_FROM,
    KW_GLOBAL,
    KW_IF,
    KW_IMPORT,
    KW_IN,
    KW_IS,
    KW_LAMBDA,
    KW_NONLOCAL,
    KW_NOT,
    KW_OR,
    KW_PASS,
    KW_RAISE,
    KW_RETURN,
    KW_TRY,
    KW_WHILE,
    KW_WITH,
    KW_YIELD,

    // Indentation
    INDENT,
    DEDENT,
    NEWLINE,

    // Special
    END_OF_FILE,
    ERROR_TOKEN
};

struct SourcePosition {
    uint32_t line = 1;
    uint32_t column = 1;
    uint32_t offset = 0;
};

struct Token {
    TokenType type = TokenType::ERROR_TOKEN;
    std::string value;
    SourcePosition start;
    SourcePosition end;

    bool is_keyword() const {
        return static_cast<uint16_t>(type) >= static_cast<uint16_t>(TokenType::KW_AND) &&
               static_cast<uint16_t>(type) <= static_cast<uint16_t>(TokenType::KW_YIELD);
    }

    bool is_literal() const {
        return static_cast<uint16_t>(type) <= static_cast<uint16_t>(TokenType::NONE_LIT);
    }

    bool is_operator() const {
        return static_cast<uint16_t>(type) >= static_cast<uint16_t>(TokenType::PLUS) &&
               static_cast<uint16_t>(type) <= static_cast<uint16_t>(TokenType::AT);
    }

    bool is_assignment() const {
        return static_cast<uint16_t>(type) >= static_cast<uint16_t>(TokenType::ASSIGN) &&
               static_cast<uint16_t>(type) <= static_cast<uint16_t>(TokenType::WALRUS);
    }
};

const char* token_type_to_string(TokenType type);

} // namespace mero

#endif // MERO_TOKENS_H
