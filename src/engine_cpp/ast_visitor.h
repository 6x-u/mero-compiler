/**
 * MERO Compiler - AST Visitor Pattern
 * Type-safe visitor for AST traversal and transformation.
 * Developer: MERO:TG@QP4RM
 */
#ifndef MERO_AST_VISITOR_H
#define MERO_AST_VISITOR_H

#include "ast_nodes.h"

namespace mero {
namespace ast {

class ASTVisitor {
public:
    virtual ~ASTVisitor() = default;

    void visit(Node* node) {
        if (!node) return;
        switch (node->kind) {
            case NodeKind::Module: visit_module(static_cast<ModuleNode*>(node)); break;
            case NodeKind::FunctionDef:
            case NodeKind::AsyncFunctionDef:
                visit_function_def(static_cast<FunctionDef*>(node)); break;
            case NodeKind::ClassDef: visit_class_def(static_cast<ClassDef*>(node)); break;
            case NodeKind::Return: visit_return(static_cast<ReturnStmt*>(node)); break;
            case NodeKind::Delete: visit_delete(static_cast<DeleteStmt*>(node)); break;
            case NodeKind::Assign: visit_assign(static_cast<AssignStmt*>(node)); break;
            case NodeKind::AugAssign: visit_aug_assign(static_cast<AugAssignStmt*>(node)); break;
            case NodeKind::AnnAssign: visit_ann_assign(static_cast<AnnAssignStmt*>(node)); break;
            case NodeKind::For:
            case NodeKind::AsyncFor:
                visit_for(static_cast<ForStmt*>(node)); break;
            case NodeKind::While: visit_while(static_cast<WhileStmt*>(node)); break;
            case NodeKind::If: visit_if(static_cast<IfStmt*>(node)); break;
            case NodeKind::With:
            case NodeKind::AsyncWith:
                visit_with(static_cast<WithStmt*>(node)); break;
            case NodeKind::Raise: visit_raise(static_cast<RaiseStmt*>(node)); break;
            case NodeKind::Try: visit_try(static_cast<TryStmt*>(node)); break;
            case NodeKind::Assert: visit_assert(static_cast<AssertStmt*>(node)); break;
            case NodeKind::Import: visit_import(static_cast<ImportStmt*>(node)); break;
            case NodeKind::ImportFrom: visit_import_from(static_cast<ImportFromStmt*>(node)); break;
            case NodeKind::Global: visit_global(static_cast<GlobalStmt*>(node)); break;
            case NodeKind::Nonlocal: visit_nonlocal(static_cast<NonlocalStmt*>(node)); break;
            case NodeKind::ExprStmt: visit_expr_stmt(static_cast<ExprStmt*>(node)); break;
            case NodeKind::Pass: visit_pass(static_cast<PassStmt*>(node)); break;
            case NodeKind::Break: visit_break(static_cast<BreakStmt*>(node)); break;
            case NodeKind::Continue: visit_continue(static_cast<ContinueStmt*>(node)); break;
            case NodeKind::BoolOp: visit_bool_op(static_cast<BoolOpExpr*>(node)); break;
            case NodeKind::BinOp: visit_bin_op(static_cast<BinOpExpr*>(node)); break;
            case NodeKind::UnaryOp: visit_unary_op(static_cast<UnaryOpExpr*>(node)); break;
            case NodeKind::Lambda: visit_lambda(static_cast<LambdaExpr*>(node)); break;
            case NodeKind::IfExpr: visit_if_expr(static_cast<IfExpr*>(node)); break;
            case NodeKind::Dict: visit_dict(static_cast<DictExpr*>(node)); break;
            case NodeKind::Set: visit_set(static_cast<SetExpr*>(node)); break;
            case NodeKind::ListComp: visit_list_comp(static_cast<ListCompExpr*>(node)); break;
            case NodeKind::Compare: visit_compare(static_cast<CompareExpr*>(node)); break;
            case NodeKind::Call: visit_call(static_cast<CallExpr*>(node)); break;
            case NodeKind::JoinedStr: visit_joined_str(static_cast<JoinedStrExpr*>(node)); break;
            case NodeKind::Constant: visit_constant(static_cast<ConstantExpr*>(node)); break;
            case NodeKind::Attribute: visit_attribute(static_cast<AttributeExpr*>(node)); break;
            case NodeKind::Subscript: visit_subscript(static_cast<SubscriptExpr*>(node)); break;
            case NodeKind::Name: visit_name(static_cast<NameExpr*>(node)); break;
            case NodeKind::List: visit_list(static_cast<ListExpr*>(node)); break;
            case NodeKind::Tuple: visit_tuple(static_cast<TupleExpr*>(node)); break;
            case NodeKind::Slice: visit_slice(static_cast<SliceExpr*>(node)); break;
            case NodeKind::Await: visit_await(static_cast<AwaitExpr*>(node)); break;
            case NodeKind::Yield: visit_yield(static_cast<YieldExpr*>(node)); break;
            default: visit_unknown(node); break;
        }
    }

    void visit_body(const NodeList& nodes) {
        for (auto& n : nodes) visit(n.get());
    }

protected:
    virtual void visit_module(ModuleNode* node) { visit_body(node->body); }
    virtual void visit_function_def(FunctionDef* node) { visit_body(node->body); }
    virtual void visit_class_def(ClassDef* node) { visit_body(node->body); }
    virtual void visit_return(ReturnStmt* node) { if (node->value) visit(node->value.get()); }
    virtual void visit_delete(DeleteStmt* node) { visit_body(node->targets); }
    virtual void visit_assign(AssignStmt* node) {
        visit_body(node->targets);
        visit(node->value.get());
    }
    virtual void visit_aug_assign(AugAssignStmt* node) {
        visit(node->target.get());
        visit(node->value.get());
    }
    virtual void visit_ann_assign(AnnAssignStmt* node) {
        visit(node->target.get());
        if (node->value) visit(node->value.get());
    }
    virtual void visit_for(ForStmt* node) {
        visit(node->target.get());
        visit(node->iter.get());
        visit_body(node->body);
        visit_body(node->orelse);
    }
    virtual void visit_while(WhileStmt* node) {
        visit(node->test.get());
        visit_body(node->body);
        visit_body(node->orelse);
    }
    virtual void visit_if(IfStmt* node) {
        visit(node->test.get());
        visit_body(node->body);
        visit_body(node->orelse);
    }
    virtual void visit_with(WithStmt* node) { visit_body(node->body); }
    virtual void visit_raise(RaiseStmt* node) {
        if (node->exc) visit(node->exc.get());
    }
    virtual void visit_try(TryStmt* node) {
        visit_body(node->body);
        visit_body(node->orelse);
        visit_body(node->finalbody);
    }
    virtual void visit_assert(AssertStmt* node) {
        visit(node->test.get());
        if (node->msg) visit(node->msg.get());
    }
    virtual void visit_import(ImportStmt*) {}
    virtual void visit_import_from(ImportFromStmt*) {}
    virtual void visit_global(GlobalStmt*) {}
    virtual void visit_nonlocal(NonlocalStmt*) {}
    virtual void visit_expr_stmt(ExprStmt* node) { visit(node->value.get()); }
    virtual void visit_pass(PassStmt*) {}
    virtual void visit_break(BreakStmt*) {}
    virtual void visit_continue(ContinueStmt*) {}
    virtual void visit_bool_op(BoolOpExpr* node) { visit_body(node->values); }
    virtual void visit_bin_op(BinOpExpr* node) {
        visit(node->left.get());
        visit(node->right.get());
    }
    virtual void visit_unary_op(UnaryOpExpr* node) { visit(node->operand.get()); }
    virtual void visit_lambda(LambdaExpr* node) { visit(node->body.get()); }
    virtual void visit_if_expr(IfExpr* node) {
        visit(node->test.get());
        visit(node->body.get());
        visit(node->orelse.get());
    }
    virtual void visit_dict(DictExpr* node) {
        visit_body(node->keys);
        visit_body(node->values);
    }
    virtual void visit_set(SetExpr* node) { visit_body(node->elts); }
    virtual void visit_list_comp(ListCompExpr* node) { visit(node->elt.get()); }
    virtual void visit_compare(CompareExpr* node) {
        visit(node->left.get());
        visit_body(node->comparators);
    }
    virtual void visit_call(CallExpr* node) {
        visit(node->func.get());
        visit_body(node->args);
    }
    virtual void visit_joined_str(JoinedStrExpr* node) { visit_body(node->values); }
    virtual void visit_constant(ConstantExpr*) {}
    virtual void visit_attribute(AttributeExpr* node) { visit(node->value.get()); }
    virtual void visit_subscript(SubscriptExpr* node) {
        visit(node->value.get());
        visit(node->slice.get());
    }
    virtual void visit_name(NameExpr*) {}
    virtual void visit_list(ListExpr* node) { visit_body(node->elts); }
    virtual void visit_tuple(TupleExpr* node) { visit_body(node->elts); }
    virtual void visit_slice(SliceExpr* node) {
        if (node->lower) visit(node->lower.get());
        if (node->upper) visit(node->upper.get());
        if (node->step) visit(node->step.get());
    }
    virtual void visit_await(AwaitExpr* node) { visit(node->value.get()); }
    virtual void visit_yield(YieldExpr* node) { if (node->value) visit(node->value.get()); }
    virtual void visit_unknown(Node*) {}
};

} // namespace ast
} // namespace mero

#endif // MERO_AST_VISITOR_H
