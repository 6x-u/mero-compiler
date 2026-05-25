/**
 * MERO Compiler - Builtin Mapper Implementation
 * Developer: MERO:TG@QP4RM
 */
#include "builtin_mapper.h"

namespace mero {

BuiltinMapper::BuiltinMapper() {
    init_functions();
    init_string_methods();
    init_list_methods();
    init_dict_methods();
}

void BuiltinMapper::init_functions() {
    functions_["print"] = {"_RT.print", true, 0, -1};
    functions_["len"] = {"#", false, 1, 1};
    functions_["range"] = {"_RT.range", true, 1, 3};
    functions_["int"] = {"_RT.to_int", true, 1, 1};
    functions_["float"] = {"tonumber", false, 1, 1};
    functions_["str"] = {"tostring", false, 1, 1};
    functions_["bool"] = {"_RT.to_bool", true, 1, 1};
    functions_["type"] = {"typeof", false, 1, 1};
    functions_["abs"] = {"math.abs", false, 1, 1};
    functions_["max"] = {"math.max", false, 1, -1};
    functions_["min"] = {"math.min", false, 1, -1};
    functions_["round"] = {"math.round", false, 1, 2};
    functions_["sum"] = {"_RT.sum", true, 1, 1};
    functions_["sorted"] = {"_RT.sorted", true, 1, 1};
    functions_["reversed"] = {"_RT.reversed", true, 1, 1};
    functions_["enumerate"] = {"_RT.enumerate", true, 1, 2};
    functions_["zip"] = {"_RT.zip", true, 2, -1};
    functions_["map"] = {"_RT.map", true, 2, 2};
    functions_["filter"] = {"_RT.filter", true, 2, 2};
    functions_["any"] = {"_RT.any", true, 1, 1};
    functions_["all"] = {"_RT.all", true, 1, 1};
    functions_["isinstance"] = {"_RT.isinstance", true, 2, 2};
    functions_["hasattr"] = {"_RT.hasattr", true, 2, 2};
    functions_["getattr"] = {"_RT.getattr", true, 2, 3};
    functions_["setattr"] = {"_RT.setattr", true, 3, 3};
    functions_["input"] = {"_RT.input", true, 0, 1};
    functions_["chr"] = {"string.char", false, 1, 1};
    functions_["ord"] = {"string.byte", false, 1, 1};
    functions_["hex"] = {"_RT.hex", true, 1, 1};
    functions_["oct"] = {"_RT.oct", true, 1, 1};
    functions_["bin"] = {"_RT.bin", true, 1, 1};
}

void BuiltinMapper::init_string_methods() {
    string_methods_["upper"] = {"string.upper(%obj%)", true, false};
    string_methods_["lower"] = {"string.lower(%obj%)", true, false};
    string_methods_["strip"] = {"_RT.str_strip(%obj%)", false, false};
    string_methods_["lstrip"] = {"_RT.str_lstrip(%obj%)", false, false};
    string_methods_["rstrip"] = {"_RT.str_rstrip(%obj%)", false, false};
    string_methods_["split"] = {"string.split(%obj%, %args%)", true, false};
    string_methods_["join"] = {"table.concat(%args%, %obj%)", false, false};
    string_methods_["replace"] = {"string.gsub(%obj%, %args%)", true, false};
    string_methods_["find"] = {"string.find(%obj%, %args%)", true, false};
    string_methods_["startswith"] = {"_RT.str_startswith(%obj%, %args%)", false, false};
    string_methods_["endswith"] = {"_RT.str_endswith(%obj%, %args%)", false, false};
    string_methods_["format"] = {"string.format(%obj%, %args%)", true, false};
    string_methods_["count"] = {"_RT.str_count(%obj%, %args%)", false, false};
    string_methods_["isdigit"] = {"_RT.str_isdigit(%obj%)", false, false};
    string_methods_["isalpha"] = {"_RT.str_isalpha(%obj%)", false, false};
    string_methods_["capitalize"] = {"_RT.str_capitalize(%obj%)", false, false};
    string_methods_["title"] = {"_RT.str_title(%obj%)", false, false};
    string_methods_["encode"] = {"%obj%", false, false};
}

void BuiltinMapper::init_list_methods() {
    list_methods_["append"] = {"table.insert(%obj%, %args%)", true, false};
    list_methods_["insert"] = {"table.insert(%obj%, %args%)", true, false};
    list_methods_["pop"] = {"table.remove(%obj%, %args%)", true, false};
    list_methods_["remove"] = {"_RT.list_remove(%obj%, %args%)", false, false};
    list_methods_["sort"] = {"table.sort(%obj%)", true, false};
    list_methods_["reverse"] = {"_RT.list_reverse(%obj%)", false, false};
    list_methods_["extend"] = {"_RT.list_extend(%obj%, %args%)", false, false};
    list_methods_["index"] = {"_RT.list_index(%obj%, %args%)", false, false};
    list_methods_["count"] = {"_RT.list_count(%obj%, %args%)", false, false};
    list_methods_["copy"] = {"table.clone(%obj%)", true, false};
    list_methods_["clear"] = {"table.clear(%obj%)", true, false};
}

void BuiltinMapper::init_dict_methods() {
    dict_methods_["keys"] = {"_RT.dict_keys(%obj%)", false, false};
    dict_methods_["values"] = {"_RT.dict_values(%obj%)", false, false};
    dict_methods_["items"] = {"_RT.dict_items(%obj%)", false, false};
    dict_methods_["get"] = {"_RT.dict_get(%obj%, %args%)", false, false};
    dict_methods_["update"] = {"_RT.dict_update(%obj%, %args%)", false, false};
    dict_methods_["pop"] = {"_RT.dict_pop(%obj%, %args%)", false, false};
    dict_methods_["setdefault"] = {"_RT.dict_setdefault(%obj%, %args%)", false, false};
    dict_methods_["copy"] = {"table.clone(%obj%)", true, false};
    dict_methods_["clear"] = {"table.clear(%obj%)", true, false};
}

bool BuiltinMapper::has_function(const std::string& name) const {
    return functions_.count(name) > 0;
}

const BuiltinMapping& BuiltinMapper::get_function(const std::string& name) const {
    return functions_.at(name);
}

bool BuiltinMapper::has_method(const std::string& type_hint, const std::string& method) const {
    if (type_hint == "str") return string_methods_.count(method) > 0;
    if (type_hint == "list") return list_methods_.count(method) > 0;
    if (type_hint == "dict") return dict_methods_.count(method) > 0;
    return string_methods_.count(method) > 0 ||
           list_methods_.count(method) > 0 ||
           dict_methods_.count(method) > 0;
}

std::string BuiltinMapper::map_method_call(const std::string& obj, const std::string& method,
                                            const std::vector<std::string>& args) const {
    auto result = map_string_method(obj, method, args);
    if (!result.empty()) return result;
    result = map_list_method(obj, method, args);
    if (!result.empty()) return result;
    result = map_dict_method(obj, method, args);
    return result;
}

std::string BuiltinMapper::map_string_method(const std::string& obj, const std::string& method,
                                              const std::vector<std::string>& args) const {
    auto it = string_methods_.find(method);
    if (it == string_methods_.end()) return "";

    std::string tmpl = it->second.luau_call;
    std::string result = tmpl;

    // Replace %obj%
    auto pos = result.find("%obj%");
    if (pos != std::string::npos) result.replace(pos, 5, obj);

    // Replace %args%
    pos = result.find("%args%");
    if (pos != std::string::npos) {
        std::string joined_args;
        for (size_t i = 0; i < args.size(); i++) {
            if (i > 0) joined_args += ", ";
            joined_args += args[i];
        }
        result.replace(pos, 6, joined_args);
    }

    return result;
}

std::string BuiltinMapper::map_list_method(const std::string& obj, const std::string& method,
                                            const std::vector<std::string>& args) const {
    auto it = list_methods_.find(method);
    if (it == list_methods_.end()) return "";

    std::string tmpl = it->second.luau_call;
    std::string result = tmpl;

    auto pos = result.find("%obj%");
    if (pos != std::string::npos) result.replace(pos, 5, obj);

    pos = result.find("%args%");
    if (pos != std::string::npos) {
        std::string joined_args;
        for (size_t i = 0; i < args.size(); i++) {
            if (i > 0) joined_args += ", ";
            joined_args += args[i];
        }
        result.replace(pos, 6, joined_args);
    }

    return result;
}

std::string BuiltinMapper::map_dict_method(const std::string& obj, const std::string& method,
                                            const std::vector<std::string>& args) const {
    auto it = dict_methods_.find(method);
    if (it == dict_methods_.end()) return "";

    std::string tmpl = it->second.luau_call;
    std::string result = tmpl;

    auto pos = result.find("%obj%");
    if (pos != std::string::npos) result.replace(pos, 5, obj);

    pos = result.find("%args%");
    if (pos != std::string::npos) {
        std::string joined_args;
        for (size_t i = 0; i < args.size(); i++) {
            if (i > 0) joined_args += ", ";
            joined_args += args[i];
        }
        result.replace(pos, 6, joined_args);
    }

    return result;
}

} // namespace mero
