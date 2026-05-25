/**
 * MERO Compiler - Symbol Table
 * Hierarchical scope-aware symbol management.
 * Developer: MERO:TG@QP4RM
 */
#ifndef MERO_SYMBOL_TABLE_H
#define MERO_SYMBOL_TABLE_H

#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <optional>

namespace mero {

enum class SymbolKind {
    Variable,
    Function,
    Class,
    Parameter,
    Module,
    Builtin
};

enum class ScopeKind {
    Global,
    Function,
    Class,
    Loop,
    Comprehension
};

struct SymbolInfo {
    std::string name;
    SymbolKind kind = SymbolKind::Variable;
    std::string type_annotation;
    bool is_const = false;
    bool is_global = false;
    bool is_nonlocal = false;
    bool is_assigned = false;
    bool is_used = false;
    int scope_depth = 0;
    uint32_t decl_line = 0;
};

class Scope {
public:
    explicit Scope(ScopeKind kind, Scope* parent = nullptr)
        : kind_(kind), parent_(parent) {
        depth_ = parent ? parent->depth_ + 1 : 0;
    }

    bool define(const std::string& name, SymbolInfo info) {
        if (symbols_.count(name)) return false;
        info.scope_depth = depth_;
        symbols_[name] = std::move(info);
        return true;
    }

    SymbolInfo* lookup_local(const std::string& name) {
        auto it = symbols_.find(name);
        return it != symbols_.end() ? &it->second : nullptr;
    }

    SymbolInfo* lookup(const std::string& name) {
        auto* sym = lookup_local(name);
        if (sym) return sym;
        if (parent_) return parent_->lookup(name);
        return nullptr;
    }

    void mark_used(const std::string& name) {
        auto* sym = lookup(name);
        if (sym) sym->is_used = true;
    }

    ScopeKind kind() const { return kind_; }
    Scope* parent() const { return parent_; }
    int depth() const { return depth_; }

    const std::unordered_map<std::string, SymbolInfo>& symbols() const { return symbols_; }

    std::vector<std::string> unused_symbols() const {
        std::vector<std::string> result;
        for (auto& [name, info] : symbols_) {
            if (!info.is_used && name[0] != '_') {
                result.push_back(name);
            }
        }
        return result;
    }

private:
    ScopeKind kind_;
    Scope* parent_;
    int depth_ = 0;
    std::unordered_map<std::string, SymbolInfo> symbols_;
};

class SymbolTable {
public:
    SymbolTable() {
        push_scope(ScopeKind::Global);
        register_builtins();
    }

    void push_scope(ScopeKind kind) {
        auto scope = std::make_unique<Scope>(kind, current_scope_);
        current_scope_ = scope.get();
        scopes_.push_back(std::move(scope));
    }

    void pop_scope() {
        if (current_scope_ && current_scope_->parent()) {
            current_scope_ = current_scope_->parent();
        }
    }

    bool define(const std::string& name, SymbolInfo info) {
        return current_scope_->define(name, std::move(info));
    }

    SymbolInfo* resolve(const std::string& name) {
        return current_scope_->lookup(name);
    }

    void mark_used(const std::string& name) {
        current_scope_->mark_used(name);
    }

    Scope* current() const { return current_scope_; }
    int depth() const { return current_scope_ ? current_scope_->depth() : 0; }

private:
    void register_builtins() {
        const char* builtins[] = {
            "print", "len", "range", "int", "float", "str", "bool",
            "list", "dict", "set", "tuple", "type", "isinstance",
            "issubclass", "hasattr", "getattr", "setattr", "delattr",
            "input", "open", "abs", "max", "min", "sum", "round",
            "enumerate", "zip", "map", "filter", "sorted", "reversed",
            "any", "all", "id", "hash", "repr", "format", "chr", "ord",
            "hex", "oct", "bin", "iter", "next", "super", "object",
            "staticmethod", "classmethod", "property", "Exception",
            "ValueError", "TypeError", "KeyError", "IndexError",
            "AttributeError", "RuntimeError", "StopIteration",
            "NotImplementedError", "IOError", "OSError"
        };
        for (const char* name : builtins) {
            SymbolInfo info;
            info.name = name;
            info.kind = SymbolKind::Builtin;
            info.is_const = true;
            current_scope_->define(name, info);
        }
    }

    Scope* current_scope_ = nullptr;
    std::vector<std::unique_ptr<Scope>> scopes_;
};

} // namespace mero

#endif // MERO_SYMBOL_TABLE_H
