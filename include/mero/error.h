/**
 * MERO Compiler - Error Handling System
 * Structured error reporting with source locations.
 * Developer: MERO:TG@QP4RM
 */
#ifndef MERO_ERROR_H
#define MERO_ERROR_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum MeroErrorLevel {
    MERO_ERROR_NOTE = 0,
    MERO_ERROR_WARNING = 1,
    MERO_ERROR_ERROR = 2,
    MERO_ERROR_FATAL = 3
} MeroErrorLevel;

typedef enum MeroErrorCode {
    MERO_OK = 0,
    /* Lexer errors */
    MERO_ERR_UNEXPECTED_CHAR = 100,
    MERO_ERR_UNTERMINATED_STRING = 101,
    MERO_ERR_INVALID_NUMBER = 102,
    MERO_ERR_INVALID_ESCAPE = 103,
    MERO_ERR_INDENT_ERROR = 104,
    /* Parser errors */
    MERO_ERR_UNEXPECTED_TOKEN = 200,
    MERO_ERR_EXPECTED_EXPR = 201,
    MERO_ERR_EXPECTED_STMT = 202,
    MERO_ERR_EXPECTED_IDENT = 203,
    MERO_ERR_EXPECTED_COLON = 204,
    MERO_ERR_UNMATCHED_PAREN = 205,
    MERO_ERR_UNMATCHED_BRACKET = 206,
    MERO_ERR_INVALID_ASSIGN = 207,
    MERO_ERR_INVALID_SYNTAX = 208,
    /* Semantic errors */
    MERO_ERR_UNDEFINED_VAR = 300,
    MERO_ERR_UNDEFINED_FUNC = 301,
    MERO_ERR_REDEFINITION = 302,
    MERO_ERR_TYPE_MISMATCH = 303,
    MERO_ERR_ARG_COUNT = 304,
    MERO_ERR_INVALID_BREAK = 305,
    MERO_ERR_INVALID_RETURN = 306,
    /* Code generation errors */
    MERO_ERR_UNSUPPORTED_FEATURE = 400,
    MERO_ERR_CODEGEN_OVERFLOW = 401,
    MERO_ERR_INVALID_TARGET = 402,
    /* Runtime errors */
    MERO_ERR_RUNTIME = 500,
    MERO_ERR_STACK_OVERFLOW = 501,
    MERO_ERR_DIVISION_ZERO = 502,
    MERO_ERR_INDEX_BOUNDS = 503,
    MERO_ERR_NULL_REFERENCE = 504
} MeroErrorCode;

typedef struct MeroSourceLoc {
    const char *file;
    uint32_t line;
    uint32_t column;
    uint32_t offset;
    uint32_t length;
} MeroSourceLoc;

typedef struct MeroError {
    MeroErrorCode code;
    MeroErrorLevel level;
    MeroSourceLoc location;
    char message[512];
    struct MeroError *next;
} MeroError;

typedef struct MeroErrorContext {
    MeroError *head;
    MeroError *tail;
    size_t count;
    size_t error_count;
    size_t warning_count;
    bool has_fatal;
} MeroErrorContext;

/* Initialize error context */
void mero_error_ctx_init(MeroErrorContext *ctx);

/* Destroy error context */
void mero_error_ctx_destroy(MeroErrorContext *ctx);

/* Report an error */
void mero_error_report(MeroErrorContext *ctx, MeroErrorCode code,
                       MeroErrorLevel level, MeroSourceLoc loc,
                       const char *fmt, ...);

/* Shorthand for error level */
void mero_error(MeroErrorContext *ctx, MeroErrorCode code,
                MeroSourceLoc loc, const char *fmt, ...);

/* Shorthand for warning level */
void mero_warning(MeroErrorContext *ctx, MeroErrorCode code,
                  MeroSourceLoc loc, const char *fmt, ...);

/* Check if context has errors */
bool mero_error_ctx_has_errors(const MeroErrorContext *ctx);

/* Print all errors to stderr */
void mero_error_ctx_print(const MeroErrorContext *ctx);

/* Get human-readable error level string */
const char *mero_error_level_str(MeroErrorLevel level);

/* Get human-readable error code string */
const char *mero_error_code_str(MeroErrorCode code);

/* Clear all errors */
void mero_error_ctx_clear(MeroErrorContext *ctx);

#ifdef __cplusplus
}
#endif

#endif /* MERO_ERROR_H */
