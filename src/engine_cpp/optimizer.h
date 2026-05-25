/**
 * MERO Compiler - IR Optimizer
 * Multi-pass optimization pipeline for IR reduction.
 * Developer: MERO:TG@QP4RM
 */
#ifndef MERO_OPTIMIZER_H
#define MERO_OPTIMIZER_H

#include "ir_nodes.h"
#include <functional>
#include <vector>

namespace mero {

enum class OptLevel : uint8_t {
    O0 = 0,  // No optimization
    O1 = 1,  // Basic optimizations
    O2 = 2,  // Standard optimizations
    O3 = 3   // Aggressive optimizations
};

class IROptimizer {
public:
    explicit IROptimizer(OptLevel level = OptLevel::O2);

    void optimize(ir::IRModule* module);

    struct Stats {
        int dead_code_removed = 0;
        int constants_folded = 0;
        int functions_inlined = 0;
        int redundant_removed = 0;
        int loops_optimized = 0;
    };

    const Stats& stats() const { return stats_; }

private:
    OptLevel level_;
    Stats stats_;

    // Optimization passes
    void constant_fold(ir::IRNodeList& body);
    void dead_code_eliminate(ir::IRNodeList& body);
    void simplify_expressions(ir::IRNodeList& body);
    void remove_redundant_assigns(ir::IRNodeList& body);
    void optimize_loops(ir::IRNodeList& body);
    void inline_small_functions(ir::IRModule* module);

    // Expression-level optimization
    ir::IRNodePtr fold_binop(ir::IRBinOp* node);
    ir::IRNodePtr fold_unaryop(ir::IRUnaryOp* node);
    ir::IRNodePtr simplify_expr(ir::IRNodePtr node);

    bool is_constant(ir::IRNode* node);
    bool is_pure(ir::IRNode* node);
    bool is_small_function(ir::IRFunction* func);

    void optimize_body(ir::IRNodeList& body);
};

} // namespace mero

#endif // MERO_OPTIMIZER_H
