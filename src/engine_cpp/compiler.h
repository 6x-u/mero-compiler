/**
 * MERO Compiler - Main Compiler Pipeline
 * Orchestrates the full compilation: Source -> Lexer -> Parser -> Semantic -> IR -> Optimize -> CodeGen
 * Developer: MERO:TG@QP4RM
 */
#ifndef MERO_COMPILER_H
#define MERO_COMPILER_H

#include "lexer.h"
#include "parser.h"
#include "semantic_analyzer.h"
#include "ir_builder.h"
#include "optimizer.h"
#include "codegen_luau.h"
#include <string>
#include <vector>

extern "C" {
#include "mero/error.h"
}

namespace mero {

struct CompilerConfig {
    OptLevel optimization = OptLevel::O2;
    CodegenOptions codegen;
    bool dump_ast = false;
    bool dump_ir = false;
    bool dump_tokens = false;
    bool strict_mode = false;
    std::string input_file = "<stdin>";
    std::string output_file;
};

struct CompileResult {
    std::string output;
    bool success = false;
    std::vector<std::string> errors;
    std::vector<std::string> warnings;

    struct {
        double lexer_ms = 0;
        double parser_ms = 0;
        double semantic_ms = 0;
        double ir_ms = 0;
        double optimize_ms = 0;
        double codegen_ms = 0;
        double total_ms = 0;
        int lines_in = 0;
        int lines_out = 0;
    } stats;
};

class Compiler {
public:
    explicit Compiler(const CompilerConfig& config = {});
    ~Compiler() = default;

    CompileResult compile(const std::string& source);
    CompileResult compile_file(const std::string& path);

    static std::string version() { return "1.0.0"; }

private:
    CompilerConfig config_;

    void collect_errors(const MeroErrorContext& ctx, CompileResult& result);
};

// C API for Python bindings
extern "C" {
    typedef struct MeroCompiler MeroCompiler;
    typedef struct MeroCompileResult {
        const char* output;
        int success;
        const char* error_message;
        int output_length;
    } MeroCompileResult;

    MeroCompiler* mero_compiler_create(int opt_level);
    void mero_compiler_destroy(MeroCompiler* compiler);
    MeroCompileResult mero_compiler_compile(MeroCompiler* compiler,
                                             const char* source, int source_len);
    void mero_compile_result_free(MeroCompileResult* result);
    const char* mero_compiler_version();
}

} // namespace mero

#endif // MERO_COMPILER_H
