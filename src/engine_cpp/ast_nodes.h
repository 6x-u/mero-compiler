/**
 * MERO Compiler - AST Node Definitions
 * Complete Python AST representation.
 * Developer: MERO:TG@QP4RM
 */
#ifndef MERO_AST_NODES_H
#define MERO_AST_NODES_H

#include "tokens.h"
#include <string>
#include <vector>
#include <memory>
#include <optional>

namespace mero {
namespace ast {

struct Node;
using NodePtr = std::unique_ptr<Node>;
using NodeList = std::vector<NodePtr>;

enum class NodeKind : uint8_t {
    // Module
    Module,
    // Statements
    FunctionDef,
    AsyncFunctionDef,
    ClassDef,
    Return,
    Delete,
    Assign,
    AugAssign,
    AnnAssign,
    For,
    AsyncFor,
    While,
    If,
    With,
    AsyncWith,
    Raise,
    Try,
    Assert,
    Import,
    ImportFrom,
    Global,
    Nonlocal,
    ExprStmt,
    Pass,
    Break,
    Continue,
    // Expressions
    BoolOp,
    NamedExpr,
    BinOp,
    UnaryOp,
    Lambda,
    IfExpr,
    Dict,
    Set,
    ListComp,
    SetComp,
    DictComp,
    GeneratorExp,
    Await,
    Yield,
    YieldFrom,
    Compare,
    Call,
    FormattedValue,
    JoinedStr,
    Constant,
    Attribute,
    Subscript,
    Starred,
    Name,
    List,
    Tuple,
    Slice,
    // Helpers
    ExceptHandler,
    Argument,
    Arguments,
    Keyword,
    Alias,
    WithItem,
    Decorator,
    Comprehension
};

struct SourceSpan {
    uint32_t start_line = 0;
    uint32_t start_col = 0;
    uint32_t end_line = 0;
    uint32_t end_col = 0;
};

struct Node {
    NodeKind kind;
    SourceSpan span;

    Node(NodeKind k) : kind(k) {}
    virtual ~Node() = default;
};

// --- Module ---
struct ModuleNode : Node {
    NodeList body;
    ModuleNode() : Node(NodeKind::Module) {}
};

// --- Statements ---
struct Param {
    std::string name;
    NodePtr annotation;
    NodePtr default_value;
    bool is_vararg = false;   // *args
    bool is_kwarg = false;    // **kwargs

    Param() = default;
    Param(Param&&) = default;
    Param& operator=(Param&&) = default;
    Param(const Param&) = delete;
    Param& operator=(const Param&) = delete;
};

struct FunctionDef : Node {
    std::string name;
    std::vector<Param> params;
    NodeList body;
    NodeList decorators;
    NodePtr return_annotation;
    bool is_async = false;

    FunctionDef() : Node(NodeKind::FunctionDef) {}
};

struct ClassDef : Node {
    std::string name;
    NodeList bases;
    NodeList keywords;
    NodeList body;
    NodeList decorators;

    ClassDef() : Node(NodeKind::ClassDef) {}
};

struct ReturnStmt : Node {
    NodePtr value;
    ReturnStmt() : Node(NodeKind::Return) {}
};

struct DeleteStmt : Node {
    NodeList targets;
    DeleteStmt() : Node(NodeKind::Delete) {}
};

struct AssignStmt : Node {
    NodeList targets;
    NodePtr value;
    AssignStmt() : Node(NodeKind::Assign) {}
};

struct AugAssignStmt : Node {
    NodePtr target;
    TokenType op;
    NodePtr value;
    AugAssignStmt() : Node(NodeKind::AugAssign) {}
};

struct AnnAssignStmt : Node {
    NodePtr target;
    NodePtr annotation;
    NodePtr value;
    AnnAssignStmt() : Node(NodeKind::AnnAssign) {}
};

struct ForStmt : Node {
    NodePtr target;
    NodePtr iter;
    NodeList body;
    NodeList orelse;
    bool is_async = false;

    ForStmt() : Node(NodeKind::For) {}
};

struct WhileStmt : Node {
    NodePtr test;
    NodeList body;
    NodeList orelse;
    WhileStmt() : Node(NodeKind::While) {}
};

struct IfStmt : Node {
    NodePtr test;
    NodeList body;
    NodeList orelse;
    IfStmt() : Node(NodeKind::If) {}
};

struct WithItem {
    NodePtr context_expr;
    NodePtr optional_vars;
};

struct WithStmt : Node {
    std::vector<WithItem> items;
    NodeList body;
    bool is_async = false;
    WithStmt() : Node(NodeKind::With) {}
};

struct RaiseStmt : Node {
    NodePtr exc;
    NodePtr cause;
    RaiseStmt() : Node(NodeKind::Raise) {}
};

struct ExceptHandlerNode : Node {
    NodePtr type;
    std::string name;
    NodeList body;
    ExceptHandlerNode() : Node(NodeKind::ExceptHandler) {}
};

struct TryStmt : Node {
    NodeList body;
    std::vector<std::unique_ptr<ExceptHandlerNode>> handlers;
    NodeList orelse;
    NodeList finalbody;
    TryStmt() : Node(NodeKind::Try) {}
};

struct AssertStmt : Node {
    NodePtr test;
    NodePtr msg;
    AssertStmt() : Node(NodeKind::Assert) {}
};

struct AliasNode {
    std::string name;
    std::string asname;
};

struct ImportStmt : Node {
    std::vector<AliasNode> names;
    ImportStmt() : Node(NodeKind::Import) {}
};

struct ImportFromStmt : Node {
    std::string module;
    std::vector<AliasNode> names;
    int level = 0;
    ImportFromStmt() : Node(NodeKind::ImportFrom) {}
};

struct GlobalStmt : Node {
    std::vector<std::string> names;
    GlobalStmt() : Node(NodeKind::Global) {}
};

struct NonlocalStmt : Node {
    std::vector<std::string> names;
    NonlocalStmt() : Node(NodeKind::Nonlocal) {}
};

struct ExprStmt : Node {
    NodePtr value;
    ExprStmt() : Node(NodeKind::ExprStmt) {}
};

struct PassStmt : Node { PassStmt() : Node(NodeKind::Pass) {} };
struct BreakStmt : Node { BreakStmt() : Node(NodeKind::Break) {} };
struct ContinueStmt : Node { ContinueStmt() : Node(NodeKind::Continue) {} };

// --- Expressions ---
enum class BoolOpKind { And, Or };
struct BoolOpExpr : Node {
    BoolOpKind op;
    NodeList values;
    BoolOpExpr() : Node(NodeKind::BoolOp) {}
};

struct NamedExpr : Node {
    NodePtr target;
    NodePtr value;
    NamedExpr() : Node(NodeKind::NamedExpr) {}
};

struct BinOpExpr : Node {
    NodePtr left;
    TokenType op;
    NodePtr right;
    BinOpExpr() : Node(NodeKind::BinOp) {}
};

enum class UnaryOpKind { Invert, Not, UAdd, USub };
struct UnaryOpExpr : Node {
    UnaryOpKind op;
    NodePtr operand;
    UnaryOpExpr() : Node(NodeKind::UnaryOp) {}
};

struct LambdaExpr : Node {
    std::vector<Param> params;
    NodePtr body;
    LambdaExpr() : Node(NodeKind::Lambda) {}
};

struct IfExpr : Node {
    NodePtr test;
    NodePtr body;
    NodePtr orelse;
    IfExpr() : Node(NodeKind::IfExpr) {}
};

struct DictExpr : Node {
    NodeList keys;
    NodeList values;
    DictExpr() : Node(NodeKind::Dict) {}
};

struct SetExpr : Node {
    NodeList elts;
    SetExpr() : Node(NodeKind::Set) {}
};

struct ComprehensionNode {
    NodePtr target;
    NodePtr iter;
    NodeList ifs;
    bool is_async = false;
};

struct ListCompExpr : Node {
    NodePtr elt;
    std::vector<ComprehensionNode> generators;
    ListCompExpr() : Node(NodeKind::ListComp) {}
};

struct SetCompExpr : Node {
    NodePtr elt;
    std::vector<ComprehensionNode> generators;
    SetCompExpr() : Node(NodeKind::SetComp) {}
};

struct DictCompExpr : Node {
    NodePtr key;
    NodePtr value;
    std::vector<ComprehensionNode> generators;
    DictCompExpr() : Node(NodeKind::DictComp) {}
};

struct GeneratorExpr : Node {
    NodePtr elt;
    std::vector<ComprehensionNode> generators;
    GeneratorExpr() : Node(NodeKind::GeneratorExp) {}
};

struct AwaitExpr : Node {
    NodePtr value;
    AwaitExpr() : Node(NodeKind::Await) {}
};

struct YieldExpr : Node {
    NodePtr value;
    bool is_from = false;
    YieldExpr() : Node(NodeKind::Yield) {}
};

enum class CmpOp { Eq, NotEq, Lt, LtE, Gt, GtE, Is, IsNot, In, NotIn };
struct CompareExpr : Node {
    NodePtr left;
    std::vector<CmpOp> ops;
    NodeList comparators;
    CompareExpr() : Node(NodeKind::Compare) {}
};

struct KeywordArg {
    std::string arg;  // empty for **kwargs
    NodePtr value;
};

struct CallExpr : Node {
    NodePtr func;
    NodeList args;
    std::vector<KeywordArg> keywords;
    CallExpr() : Node(NodeKind::Call) {}
};

struct FormattedValueExpr : Node {
    NodePtr value;
    int conversion = -1;  // -1, 's', 'r', 'a'
    NodePtr format_spec;
    FormattedValueExpr() : Node(NodeKind::FormattedValue) {}
};

struct JoinedStrExpr : Node {
    NodeList values;
    JoinedStrExpr() : Node(NodeKind::JoinedStr) {}
};

enum class ConstantKind { Int, Float, Str, Bool, NoneVal, Bytes, Ellipsis };
struct ConstantExpr : Node {
    ConstantKind const_kind;
    std::string value;
    ConstantExpr() : Node(NodeKind::Constant) {}
};

struct AttributeExpr : Node {
    NodePtr value;
    std::string attr;
    AttributeExpr() : Node(NodeKind::Attribute) {}
};

struct SubscriptExpr : Node {
    NodePtr value;
    NodePtr slice;
    SubscriptExpr() : Node(NodeKind::Subscript) {}
};

struct StarredExpr : Node {
    NodePtr value;
    StarredExpr() : Node(NodeKind::Starred) {}
};

struct NameExpr : Node {
    std::string id;
    NameExpr() : Node(NodeKind::Name) {}
};

struct ListExpr : Node {
    NodeList elts;
    ListExpr() : Node(NodeKind::List) {}
};

struct TupleExpr : Node {
    NodeList elts;
    TupleExpr() : Node(NodeKind::Tuple) {}
};

struct SliceExpr : Node {
    NodePtr lower;
    NodePtr upper;
    NodePtr step;
    SliceExpr() : Node(NodeKind::Slice) {}
};

} // namespace ast
} // namespace mero

#endif // MERO_AST_NODES_H
