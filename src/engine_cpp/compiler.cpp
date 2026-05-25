/**
 * MERO Compiler - Main Compiler Pipeline Implementation
 * Developer: MERO:TG@QP4RM
 */
#include "compiler.h"
#include <chrono>
#include <fstream>
#include <sstream>
#include <cstring>

namespace mero {

Compiler::Compiler(const CompilerConfig& config) : config_(config) {}

CompileResult Compiler::compile(const std::string& source) {
    CompileResult result;
    auto total_start = std::chrono::high_resolution_clock::now();

    result.stats.lines_in = static_cast<int>(
        std::count(source.begin(), source.end(), '\n') + 1
    );

    // Phase 1: Lexical Analysis
    auto t0 = std::chrono::high_resolution_clock::now();
    Lexer lexer(source, config_.input_file);

    if (config_.dump_tokens) {
        auto tokens = lexer.tokenize_all();
        // Reset lexer
        lexer = Lexer(source, config_.input_file);
    }
    auto t1 = std::chrono::high_resolution_clock::now();
    result.stats.lexer_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

    // Phase 2: Parsing
    t0 = std::chrono::high_resolution_clock::now();
    Parser parser(lexer);
    auto ast = parser.parse_module();
    t1 = std::chrono::high_resolution_clock::now();
    result.stats.parser_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

    if (parser.has_errors()) {
        collect_errors(parser.errors(), result);
        return result;
    }

    // Phase 3: Semantic Analysis
    t0 = std::chrono::high_resolution_clock::now();
    SemanticAnalyzer analyzer;
    analyzer.analyze(ast.get());
    t1 = std::chrono::high_resolution_clock::now();
    result.stats.semantic_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

    if (analyzer.has_errors()) {
        collect_errors(analyzer.errors(), result);
        return result;
    }

    // Phase 4: IR Generation
    t0 = std::chrono::high_resolution_clock::now();
    IRBuilder ir_builder(analyzer.symbols());
    auto ir_module = ir_builder.build(ast.get());
    t1 = std::chrono::high_resolution_clock::now();
    result.stats.ir_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

    if (ir_builder.has_errors()) {
        result.errors.push_back("IR generation failed");
        return result;
    }

    // Phase 5: Optimization
    t0 = std::chrono::high_resolution_clock::now();
    IROptimizer optimizer(config_.optimization);
    optimizer.optimize(ir_module.get());
    t1 = std::chrono::high_resolution_clock::now();
    result.stats.optimize_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

    // Phase 6: Code Generation
    t0 = std::chrono::high_resolution_clock::now();
    LuauCodegen codegen(config_.codegen);
    result.output = codegen.generate(ir_module.get());
    t1 = std::chrono::high_resolution_clock::now();
    result.stats.codegen_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

    auto total_end = std::chrono::high_resolution_clock::now();
    result.stats.total_ms = std::chrono::duration<double, std::milli>(total_end - total_start).count();
    result.stats.lines_out = static_cast<int>(
        std::count(result.output.begin(), result.output.end(), '\n') + 1
    );

    result.success = true;
    return result;
}

CompileResult Compiler::compile_file(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        CompileResult result;
        result.errors.push_back("Cannot open file: " + path);
        return result;
    }

    std::stringstream ss;
    ss << file.rdbuf();
    config_.input_file = path;
    return compile(ss.str());
}

void Compiler::collect_errors(const MeroErrorContext& ctx, CompileResult& result) {
    MeroError* err = ctx.head;
    while (err) {
        std::string msg;
        if (err->location.file) {
            msg = std::string(err->location.file) + ":" +
                  std::to_string(err->location.line) + ":" +
                  std::to_string(err->location.column) + ": ";
        }
        msg += err->message;

        if (err->level == MERO_ERROR_WARNING) {
            result.warnings.push_back(msg);
        } else {
            result.errors.push_back(msg);
        }
        err = err->next;
    }
}

// --- C API Implementation ---

struct MeroCompiler {
    mero::Compiler* impl;
};

extern "C" {

MeroCompiler* mero_compiler_create(int opt_level) {
    CompilerConfig config;
    config.optimization = static_cast<OptLevel>(opt_level);
    auto* c = new MeroCompiler;
    c->impl = new Compiler(config);
    return c;
}

void mero_compiler_destroy(MeroCompiler* compiler) {
    if (compiler) {
        delete compiler->impl;
        delete compiler;
    }
}

MeroCompileResult mero_compiler_compile(MeroCompiler* compiler,
                                         const char* source, int source_len) {
    MeroCompileResult c_result = {};

    std::string src(source, source_len > 0 ? source_len : strlen(source));
    auto result = compiler->impl->compile(src);

    if (result.success) {
        char* output = new char[result.output.size() + 1];
        memcpy(output, result.output.c_str(), result.output.size() + 1);
        c_result.output = output;
        c_result.output_length = static_cast<int>(result.output.size());
        c_result.success = 1;
    } else {
        std::string errors;
        for (auto& e : result.errors) {
            errors += e + "\n";
        }
        char* err_msg = new char[errors.size() + 1];
        memcpy(err_msg, errors.c_str(), errors.size() + 1);
        c_result.error_message = err_msg;
        c_result.success = 0;
    }

    return c_result;
}

void mero_compile_result_free(MeroCompileResult* result) {
    if (result->output) delete[] result->output;
    if (result->error_message) delete[] result->error_message;
}

const char* mero_compiler_version() {
    return "1.0.0";
}

}

} // namespace mero
