/**
 * MERO Compiler - Python to Luau Builtin Mapper
 * Maps Python standard library functions to Luau equivalents.
 * Developer: MERO:TG@QP4RM
 */
#ifndef MERO_BUILTIN_MAPPER_H
#define MERO_BUILTIN_MAPPER_H

#include <string>
#include <unordered_map>
#include <vector>
#include <functional>

namespace mero {

struct BuiltinMapping {
    std::string luau_equivalent;
    bool needs_runtime = false;
    int min_args = 0;
    int max_args = -1;  // -1 = variadic
};

struct MethodMapping {
    std::string luau_call;
    bool is_table_method = false;
    bool is_string_method = false;
};

class BuiltinMapper {
public:
    BuiltinMapper();

    bool has_function(const std::string& name) const;
    const BuiltinMapping& get_function(const std::string& name) const;

    bool has_method(const std::string& type_hint, const std::string& method) const;
    std::string map_method_call(const std::string& obj, const std::string& method,
                                 const std::vector<std::string>& args) const;

    std::string map_string_method(const std::string& obj, const std::string& method,
                                   const std::vector<std::string>& args) const;
    std::string map_list_method(const std::string& obj, const std::string& method,
                                 const std::vector<std::string>& args) const;
    std::string map_dict_method(const std::string& obj, const std::string& method,
                                 const std::vector<std::string>& args) const;

private:
    std::unordered_map<std::string, BuiltinMapping> functions_;
    std::unordered_map<std::string, MethodMapping> string_methods_;
    std::unordered_map<std::string, MethodMapping> list_methods_;
    std::unordered_map<std::string, MethodMapping> dict_methods_;

    void init_functions();
    void init_string_methods();
    void init_list_methods();
    void init_dict_methods();
};

} // namespace mero

#endif // MERO_BUILTIN_MAPPER_H
