/**
 * MERO Compiler - Intermediate Representation Nodes
 * Unified IR for code generation and optimization.
 * Developer: MERO:TG@QP4RM
 */
#ifndef MERO_IR_NODES_H
#define MERO_IR_NODES_H

#include <string>
#include <vector>
#include <memory>
#include <cstdint>

namespace mero {
namespace ir {

enum class OpCode : uint8_t {
    Add, Sub, Mul, Div, FloorDiv, Mod, Pow, Neg,
    Eq, Neq, Lt, Gt, Lte, Gte,
    And, Or, Not,
    BitAnd, BitOr, BitXor, BitNot, Shl, Shr,
    Concat, Length, Index, FieldGet, FieldSet
};

enum class TypeTag : uint8_t {
    Nil, Bool, Int, Float, String, Table, Array, Function, Any, Class
};

struct IRType {
    TypeTag tag = TypeTag::Any;
    std::string name;

    bool is_numeric() const { return tag == TypeTag::Int || tag == TypeTag::Float; }
    std::string luau_type() const {
        switch (tag) {
            case TypeTag::Nil: return "nil";
            case TypeTag::Bool: return "boolean";
            case TypeTag::Int: case TypeTag::Float: return "number";
            case TypeTag::String: return "string";
            case TypeTag::Table: return "{[any]: any}";
            case TypeTag::Array: return "{any}";
            case TypeTag::Function: return "(...any) -> any";
            default: return "any";
        }
    }
};

struct IRNode;
using IRNodePtr = std::unique_ptr<IRNode>;
using IRNodeList = std::vector<IRNodePtr>;

enum class IRKind : uint8_t {
    Literal, Nil, Ident, BinOp, UnaryOp,
    Call, MethodCall, Index, Field,
    Array, Table, Lambda, Ternary,
    Assign, LocalDecl, AugAssign,
    Function, Return, If, While,
    ForNumeric, ForIn, Break, Continue,
    Class, Module,
    Try, Throw, ExprStmt,
    Comment, Comprehension, FString,
    Slice, MultiAssign, Global
};

struct IRNode {
    IRKind kind;
    IRType type;
    uint32_t line = 0;

    IRNode(IRKind k) : kind(k) {}
    virtual ~IRNode() = default;
};

struct IRLiteral : IRNode {
    enum LitKind { LInt, LFloat, LString, LBool, LNil };
    LitKind lit_kind;
    std::string value;
    IRLiteral() : IRNode(IRKind::Literal) {}
};

struct IRIdent : IRNode {
    std::string name;
    int scope_depth = 0;
    IRIdent() : IRNode(IRKind::Ident) {}
};

struct IRBinOp : IRNode {
    OpCode op;
    IRNodePtr left;
    IRNodePtr right;
    IRBinOp() : IRNode(IRKind::BinOp) {}
};

struct IRUnaryOp : IRNode {
    OpCode op;
    IRNodePtr operand;
    IRUnaryOp() : IRNode(IRKind::UnaryOp) {}
};

struct IRCall : IRNode {
    IRNodePtr callee;
    IRNodeList args;
    std::vector<std::pair<std::string, IRNodePtr>> kwargs;
    IRCall() : IRNode(IRKind::Call) {}
};

struct IRMethodCall : IRNode {
    IRNodePtr object;
    std::string method;
    IRNodeList args;
    IRMethodCall() : IRNode(IRKind::MethodCall) {}
};

struct IRIndex : IRNode {
    IRNodePtr object;
    IRNodePtr index;
    IRIndex() : IRNode(IRKind::Index) {}
};

struct IRField : IRNode {
    IRNodePtr object;
    std::string field;
    IRField() : IRNode(IRKind::Field) {}
};

struct IRArray : IRNode {
    IRNodeList elements;
    IRArray() : IRNode(IRKind::Array) {}
};

struct IRTable : IRNode {
    std::vector<std::pair<IRNodePtr, IRNodePtr>> pairs;
    IRTable() : IRNode(IRKind::Table) {}
};

struct IRParam {
    std::string name;
    IRType type;
    IRNodePtr default_val;
    bool variadic = false;
};

struct IRLambda : IRNode {
    std::vector<IRParam> params;
    IRNodeList body;
    IRLambda() : IRNode(IRKind::Lambda) {}
};

struct IRTernary : IRNode {
    IRNodePtr condition;
    IRNodePtr true_val;
    IRNodePtr false_val;
    IRTernary() : IRNode(IRKind::Ternary) {}
};

struct IRAssign : IRNode {
    IRNodePtr target;
    IRNodePtr value;
    bool is_local = false;
    IRAssign() : IRNode(IRKind::Assign) {}
};

struct IRLocalDecl : IRNode {
    std::string name;
    IRNodePtr value;
    IRLocalDecl() : IRNode(IRKind::LocalDecl) {}
};

struct IRAugAssign : IRNode {
    IRNodePtr target;
    OpCode op;
    IRNodePtr value;
    IRAugAssign() : IRNode(IRKind::AugAssign) {}
};

struct IRFunction : IRNode {
    std::string name;
    std::vector<IRParam> params;
    IRNodeList body;
    IRType return_type;
    bool is_local = true;
    bool is_method = false;
    std::vector<std::string> decorators;
    IRFunction() : IRNode(IRKind::Function) {}
};

struct IRReturn : IRNode {
    IRNodeList values;
    IRReturn() : IRNode(IRKind::Return) {}
};

struct IRIf : IRNode {
    IRNodePtr condition;
    IRNodeList then_body;
    std::vector<std::pair<IRNodePtr, IRNodeList>> elifs;
    IRNodeList else_body;
    IRIf() : IRNode(IRKind::If) {}
};

struct IRWhile : IRNode {
    IRNodePtr condition;
    IRNodeList body;
    IRWhile() : IRNode(IRKind::While) {}
};

struct IRForNumeric : IRNode {
    std::string var;
    IRNodePtr start;
    IRNodePtr stop;
    IRNodePtr step;
    IRNodeList body;
    IRForNumeric() : IRNode(IRKind::ForNumeric) {}
};

struct IRForIn : IRNode {
    std::vector<std::string> vars;
    IRNodePtr iterator;
    IRNodeList body;
    IRForIn() : IRNode(IRKind::ForIn) {}
};

struct IRBreak : IRNode { IRBreak() : IRNode(IRKind::Break) {} };
struct IRContinue : IRNode { IRContinue() : IRNode(IRKind::Continue) {} };

struct IRClass : IRNode {
    std::string name;
    std::string base;
    std::vector<std::unique_ptr<IRFunction>> methods;
    std::vector<std::unique_ptr<IRAssign>> fields;
    IRClass() : IRNode(IRKind::Class) {}
};

struct IRModule : IRNode {
    std::string name;
    IRNodeList body;
    std::vector<std::string> imports;
    std::vector<std::string> exports;
    IRModule() : IRNode(IRKind::Module) {}
};

struct IRExceptHandler {
    std::string type;
    std::string name;
    IRNodeList body;
};

struct IRTry : IRNode {
    IRNodeList body;
    std::vector<IRExceptHandler> handlers;
    IRNodeList else_body;
    IRNodeList finally_body;
    IRTry() : IRNode(IRKind::Try) {}
};

struct IRThrow : IRNode {
    IRNodePtr exception;
    IRThrow() : IRNode(IRKind::Throw) {}
};

struct IRExprStmt : IRNode {
    IRNodePtr expr;
    IRExprStmt() : IRNode(IRKind::ExprStmt) {}
};

struct IRComment : IRNode {
    std::string text;
    IRComment() : IRNode(IRKind::Comment) {}
};

struct IRSlice : IRNode {
    IRNodePtr object;
    IRNodePtr lower;
    IRNodePtr upper;
    IRNodePtr step;
    IRSlice() : IRNode(IRKind::Slice) {}
};

struct IRFString : IRNode {
    IRNodeList parts;
    IRFString() : IRNode(IRKind::FString) {}
};

struct IRComprehension : IRNode {
    IRNodePtr element;
    std::string variable;
    IRNodePtr iterator;
    IRNodeList conditions;
    IRComprehension() : IRNode(IRKind::Comprehension) {}
};

struct IRMultiAssign : IRNode {
    IRNodeList targets;
    IRNodeList values;
    bool is_local = false;
    IRMultiAssign() : IRNode(IRKind::MultiAssign) {}
};

struct IRGlobal : IRNode {
    std::vector<std::string> names;
    IRGlobal() : IRNode(IRKind::Global) {}
};

} // namespace ir
} // namespace mero

#endif // MERO_IR_NODES_H
