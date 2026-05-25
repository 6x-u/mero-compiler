/**
 * MERO Compiler - Error Handling System Implementation
 * Developer: MERO:TG@QP4RM
 */
#include "mero/error.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

void mero_error_ctx_init(MeroErrorContext *ctx) {
    ctx->head = NULL;
    ctx->tail = NULL;
    ctx->count = 0;
    ctx->error_count = 0;
    ctx->warning_count = 0;
    ctx->has_fatal = false;
}

void mero_error_ctx_destroy(MeroErrorContext *ctx) {
    MeroError *err = ctx->head;
    while (err) {
        MeroError *next = err->next;
        free(err);
        err = next;
    }
    ctx->head = NULL;
    ctx->tail = NULL;
    ctx->count = 0;
}

void mero_error_report(MeroErrorContext *ctx, MeroErrorCode code,
                       MeroErrorLevel level, MeroSourceLoc loc,
                       const char *fmt, ...) {
    MeroError *err = (MeroError *)malloc(sizeof(MeroError));
    if (!err) return;

    err->code = code;
    err->level = level;
    err->location = loc;
    err->next = NULL;

    va_list args;
    va_start(args, fmt);
    vsnprintf(err->message, sizeof(err->message), fmt, args);
    va_end(args);

    if (ctx->tail) {
        ctx->tail->next = err;
    } else {
        ctx->head = err;
    }
    ctx->tail = err;
    ctx->count++;

    switch (level) {
        case MERO_ERROR_ERROR:
            ctx->error_count++;
            break;
        case MERO_ERROR_WARNING:
            ctx->warning_count++;
            break;
        case MERO_ERROR_FATAL:
            ctx->error_count++;
            ctx->has_fatal = true;
            break;
        default:
            break;
    }
}

void mero_error(MeroErrorContext *ctx, MeroErrorCode code,
                MeroSourceLoc loc, const char *fmt, ...) {
    MeroError *err = (MeroError *)malloc(sizeof(MeroError));
    if (!err) return;

    err->code = code;
    err->level = MERO_ERROR_ERROR;
    err->location = loc;
    err->next = NULL;

    va_list args;
    va_start(args, fmt);
    vsnprintf(err->message, sizeof(err->message), fmt, args);
    va_end(args);

    if (ctx->tail) {
        ctx->tail->next = err;
    } else {
        ctx->head = err;
    }
    ctx->tail = err;
    ctx->count++;
    ctx->error_count++;
}

void mero_warning(MeroErrorContext *ctx, MeroErrorCode code,
                  MeroSourceLoc loc, const char *fmt, ...) {
    MeroError *err = (MeroError *)malloc(sizeof(MeroError));
    if (!err) return;

    err->code = code;
    err->level = MERO_ERROR_WARNING;
    err->location = loc;
    err->next = NULL;

    va_list args;
    va_start(args, fmt);
    vsnprintf(err->message, sizeof(err->message), fmt, args);
    va_end(args);

    if (ctx->tail) {
        ctx->tail->next = err;
    } else {
        ctx->head = err;
    }
    ctx->tail = err;
    ctx->count++;
    ctx->warning_count++;
}

bool mero_error_ctx_has_errors(const MeroErrorContext *ctx) {
    return ctx->error_count > 0;
}

void mero_error_ctx_print(const MeroErrorContext *ctx) {
    MeroError *err = ctx->head;
    while (err) {
        const char *level_str = mero_error_level_str(err->level);
        if (err->location.file) {
            fprintf(stderr, "%s:%u:%u: %s [E%04d]: %s\n",
                    err->location.file, err->location.line,
                    err->location.column, level_str,
                    (int)err->code, err->message);
        } else {
            fprintf(stderr, "%s [E%04d]: %s\n",
                    level_str, (int)err->code, err->message);
        }
        err = err->next;
    }

    if (ctx->error_count > 0 || ctx->warning_count > 0) {
        fprintf(stderr, "\n%zu error(s), %zu warning(s)\n",
                ctx->error_count, ctx->warning_count);
    }
}

const char *mero_error_level_str(MeroErrorLevel level) {
    switch (level) {
        case MERO_ERROR_NOTE:    return "note";
        case MERO_ERROR_WARNING: return "warning";
        case MERO_ERROR_ERROR:   return "error";
        case MERO_ERROR_FATAL:   return "fatal";
        default:                 return "unknown";
    }
}

const char *mero_error_code_str(MeroErrorCode code) {
    switch (code) {
        case MERO_OK: return "ok";
        case MERO_ERR_UNEXPECTED_CHAR: return "unexpected character";
        case MERO_ERR_UNTERMINATED_STRING: return "unterminated string";
        case MERO_ERR_INVALID_NUMBER: return "invalid number literal";
        case MERO_ERR_INVALID_ESCAPE: return "invalid escape sequence";
        case MERO_ERR_INDENT_ERROR: return "indentation error";
        case MERO_ERR_UNEXPECTED_TOKEN: return "unexpected token";
        case MERO_ERR_EXPECTED_EXPR: return "expected expression";
        case MERO_ERR_EXPECTED_STMT: return "expected statement";
        case MERO_ERR_EXPECTED_IDENT: return "expected identifier";
        case MERO_ERR_EXPECTED_COLON: return "expected colon";
        case MERO_ERR_UNMATCHED_PAREN: return "unmatched parenthesis";
        case MERO_ERR_UNMATCHED_BRACKET: return "unmatched bracket";
        case MERO_ERR_INVALID_ASSIGN: return "invalid assignment target";
        case MERO_ERR_INVALID_SYNTAX: return "invalid syntax";
        case MERO_ERR_UNDEFINED_VAR: return "undefined variable";
        case MERO_ERR_UNDEFINED_FUNC: return "undefined function";
        case MERO_ERR_REDEFINITION: return "redefinition";
        case MERO_ERR_TYPE_MISMATCH: return "type mismatch";
        case MERO_ERR_ARG_COUNT: return "argument count mismatch";
        case MERO_ERR_INVALID_BREAK: return "break outside loop";
        case MERO_ERR_INVALID_RETURN: return "return outside function";
        case MERO_ERR_UNSUPPORTED_FEATURE: return "unsupported feature";
        case MERO_ERR_CODEGEN_OVERFLOW: return "codegen overflow";
        case MERO_ERR_INVALID_TARGET: return "invalid target";
        case MERO_ERR_RUNTIME: return "runtime error";
        case MERO_ERR_STACK_OVERFLOW: return "stack overflow";
        case MERO_ERR_DIVISION_ZERO: return "division by zero";
        case MERO_ERR_INDEX_BOUNDS: return "index out of bounds";
        case MERO_ERR_NULL_REFERENCE: return "null reference";
        default: return "unknown error";
    }
}

void mero_error_ctx_clear(MeroErrorContext *ctx) {
    mero_error_ctx_destroy(ctx);
    mero_error_ctx_init(ctx);
}
