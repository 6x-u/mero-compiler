/**
 * MERO Compiler - CLI Entry Point (C++)
 * Standalone command-line compiler executable.
 * Developer: MERO:TG@QP4RM
 */
#include "compiler.h"
#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>

static void print_usage() {
    fprintf(stderr,
        "Usage: meroc [options] <input.py>\n"
        "Options:\n"
        "  -o <file>      Output file (default: stdout)\n"
        "  -O<level>      Optimization level (0-3, default: 2)\n"
        "  --strict       Enable strict type checking\n"
        "  --minify       Minify output\n"
        "  --no-types     Disable type annotations\n"
        "  --dump-ast     Print AST\n"
        "  --dump-ir      Print IR\n"
        "  --version      Print version\n"
        "  -h, --help     Show this help\n"
    );
}

static std::string read_file(const char* path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        fprintf(stderr, "error: cannot open file: %s\n", path);
        return "";
    }
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

int main(int argc, char** argv) {
    if (argc < 2) {
        print_usage();
        return 1;
    }

    const char* input_file = nullptr;
    const char* output_file = nullptr;
    int opt_level = 2;
    bool strict = false;
    bool minify = false;
    bool no_types = false;
    bool dump_ast = false;
    bool dump_ir = false;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage();
            return 0;
        }
        if (strcmp(argv[i], "--version") == 0) {
            printf("meroc %s\n", mero_compiler_version());
            return 0;
        }
        if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            output_file = argv[++i];
            continue;
        }
        if (strncmp(argv[i], "-O", 2) == 0) {
            opt_level = argv[i][2] - '0';
            if (opt_level < 0 || opt_level > 3) opt_level = 2;
            continue;
        }
        if (strcmp(argv[i], "--strict") == 0) { strict = true; continue; }
        if (strcmp(argv[i], "--minify") == 0) { minify = true; continue; }
        if (strcmp(argv[i], "--no-types") == 0) { no_types = true; continue; }
        if (strcmp(argv[i], "--dump-ast") == 0) { dump_ast = true; continue; }
        if (strcmp(argv[i], "--dump-ir") == 0) { dump_ir = true; continue; }

        if (argv[i][0] != '-') {
            input_file = argv[i];
        } else {
            fprintf(stderr, "error: unknown option: %s\n", argv[i]);
            return 1;
        }
    }

    if (!input_file) {
        fprintf(stderr, "error: no input file specified\n");
        return 1;
    }

    std::string source = read_file(input_file);
    if (source.empty()) return 1;

    void* compiler = mero_compiler_create(opt_level);
    MeroCompileResult result = mero_compiler_compile(compiler, source.c_str(), (int)source.size());

    if (result.success) {
        if (output_file) {
            std::ofstream out(output_file);
            if (!out.is_open()) {
                fprintf(stderr, "error: cannot write to: %s\n", output_file);
                mero_compile_result_free(&result);
                mero_compiler_destroy(compiler);
                return 1;
            }
            out.write(result.output, result.output_length);
        } else {
            fwrite(result.output, 1, result.output_length, stdout);
        }
    } else {
        fprintf(stderr, "%s\n", result.error_message ? result.error_message : "Compilation failed");
        mero_compile_result_free(&result);
        mero_compiler_destroy(compiler);
        return 1;
    }

    mero_compile_result_free(&result);
    mero_compiler_destroy(compiler);
    return 0;
}
