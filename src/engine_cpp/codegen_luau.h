/**
 * MERO Compiler - Luau Code Generator
 * Translates IR to production-ready Luau source code.
 * Developer: MERO:TG@QP4RM
 */
#ifndef MERO_CODEGEN_LUAU_H
#define MERO_CODEGEN_LUAU_H

#include "ir_nodes.h"
#include <string>
#include <sstream>
#include <unordered_map>
#include <unordered_set>

namespace mero {

struct CodegenOptions {
    bool emit_type_annotations = true;
    bool emit_comments = true;
    bool minify = false;
    bool emit_source_map = false;
    int indent_size = 4;
    bool use_continue = true;   // Luau supports continue
    std::string module_prefix = "MERO";
};

class LuauCodegen {
public:
    explicit LuauCodegen(const CodegenOptions& opts = {});

    std::string generate(ir::IRModule* module);
    const std::string& output() const { return output_; }

private:
    CodegenOptions opts_;
    std::string output_;
    int indent_level_ = 0;
    bool needs_runtime_ = false;
    std::unordered_set<std::string> used_builtins_;
    std::unordered_map<std::string, std::string> python_to_luau_;

    void init_builtin_map();

    // Output helpers
    void emit(const std::string& code);
    void emit_line(const std::string& code);
    void emit_indent();
    void emit_newline();
    void indent();
    void dedent();
    std::string get_indent() const;

    // Module structure
    void emit_header();
    void emit_runtime_require();
    void emit_body(const ir::IRNodeList& body);
    void emit_footer(ir::IRModule* module);

    // Statements
    void emit_node(ir::IRNode* node);
    void emit_function(ir::IRFunction* node);
    void emit_class(ir::IRClass* node);
    void emit_if(ir::IRIf* node);
    void emit_while(ir::IRWhile* node);
    void emit_for_numeric(ir::IRForNumeric* node);
    void emit_for_in(ir::IRForIn* node);
    void emit_assign(ir::IRAssign* node);
    void emit_aug_assign(ir::IRAugAssign* node);
    void emit_return(ir::IRReturn* node);
    void emit_try(ir::IRTry* node);
    void emit_throw(ir::IRThrow* node);
    void emit_expr_stmt(ir::IRExprStmt* node);

    // Expressions
    std::string emit_expr(ir::IRNode* node);
    std::string emit_literal(ir::IRLiteral* node);
    std::string emit_ident(ir::IRIdent* node);
    std::string emit_binop(ir::IRBinOp* node);
    std::string emit_unaryop(ir::IRUnaryOp* node);
    std::string emit_call(ir::IRCall* node);
    std::string emit_method_call(ir::IRMethodCall* node);
    std::string emit_index(ir::IRIndex* node);
    std::string emit_field(ir::IRField* node);
    std::string emit_array(ir::IRArray* node);
    std::string emit_table(ir::IRTable* node);
    std::string emit_lambda(ir::IRLambda* node);
    std::string emit_ternary(ir::IRTernary* node);
    std::string emit_fstring(ir::IRFString* node);
    std::string emit_comprehension(ir::IRComprehension* node);
    std::string emit_slice(ir::IRSlice* node);

    // Helpers
    std::string map_operator(ir::OpCode op);
    std::string map_builtin_call(const std::string& name, const ir::IRNodeList& args);
    std::string escape_string(const std::string& str);
    bool is_builtin_function(const std::string& name);
    std::string translate_method(const std::string& obj_expr, const std::string& method,
                                 const ir::IRNodeList& args);
};

} // namespace mero

#endif // MERO_CODEGEN_LUAU_H
