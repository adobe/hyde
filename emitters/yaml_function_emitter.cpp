/*
Copyright 2018 Adobe
All Rights Reserved.

NOTICE: Adobe permits you to use, modify, and distribute this file in
accordance with the terms of the Adobe license agreement accompanying
it. If you have received this file from a source other than Adobe,
then your use, modification, or distribution of it requires the prior
written permission of Adobe. 
*/

// identity
#include "yaml_function_emitter.hpp"

// stdc++
#include <iostream>

/**************************************************************************************************/

namespace hyde {

/**************************************************************************************************/

bool yaml_function_emitter::do_merge(const std::string& filepath,
                                     const json& have,
                                     const json& expected,
                                     json& out_merged) {
    bool failure{false};

    failure |= check_scalar(filepath, have, expected, "", out_merged, "defined_in_file");
    failure |= check_scalar_array(filepath, have, expected, "", out_merged, "namespace");
    failure |= check_scalar(filepath, have, expected, "", out_merged, "is_ctor");
    failure |= check_scalar(filepath, have, expected, "", out_merged, "is_dtor");
    failure |= check_map(
        filepath, have, expected, "", out_merged, "overloads",
        [this](const std::string& filepath, const json& have, const json& expected,
               const std::string& nodepath, json& out_merged) {
            bool failure{false};

            failure |= check_editable_scalar(filepath, have, expected, nodepath, out_merged, "description");
            failure |= check_scalar(filepath, have, expected, nodepath, out_merged, "signature_with_names");
            failure |= check_editable_scalar(filepath, have, expected, nodepath, out_merged, "return");
            if (_options._tested_by != hyde::attribute_category::disabled) {
                failure |= check_editable_scalar_array(filepath, have, expected, nodepath, out_merged, "tested_by");
            }
            
            failure |= check_scalar_array(filepath, have, expected, nodepath, out_merged, "annotation");

            failure |= check_object_array(
                filepath, have, expected, nodepath, out_merged, "arguments", "name",
                [this](const std::string& filepath, const json& have, const json& expected,
                       const std::string& nodepath, json& out_merged) {
                    bool failure{false};

                    failure |= check_scalar(filepath, have, expected, nodepath, out_merged, "name");
                    failure |= check_scalar(filepath, have, expected, nodepath, out_merged, "type");
                    failure |= check_editable_scalar(filepath, have, expected, nodepath, out_merged, "description");
                    failure |= check_scalar(filepath, have, expected, nodepath, out_merged, "unnamed");

                    return failure;
                });

            return failure;
        });

    return failure;
}

/**************************************************************************************************/

bool yaml_function_emitter::emit(const json& jsn, json& out_emitted) {
    std::filesystem::path dst;
    std::string name;
    std::string filename;
    std::string defined_path;
    std::size_t i{0};
    json overloads = json::object();
    bool is_ctor{false};
    bool is_dtor{false};
    std::size_t count(jsn.size());

    for (const auto& overload : jsn) {
        if (!i) {
            dst = dst_path(overload);
            // always the unqualified name, as free functions may be defined
            // over different namespaces.
            name = overload["short_name"];
            // prefix to keep free-function from colliding with class member (e.g., `swap`)
            filename = (_as_methods ? "m_" : "f_") + filename_filter(name);
            defined_path = defined_in_file(overload["defined_in_file"], _src_root);
            if (overload.count("is_ctor") && overload["is_ctor"]) is_ctor = true;
            if (overload.count("is_dtor") && overload["is_dtor"]) is_dtor = true;
        }

        const std::string& key = static_cast<const std::string&>(overload["signature"]);

        // If there are any in-source (a.k.a. Doxygen) comments, insert them into
        // the node *first* so we can use them to decide if subsequent Hyde fields
        // can be deferred.
        insert_doxygen(overload, overloads[key]);

        overloads[key]["signature_with_names"] = overload["signature_with_names"];
        // description is now optional when there is a singular variant.
        overloads[key]["description"] = count > 1 ? tag_value_missing_k : tag_value_optional_k;
        overloads[key]["return"] = tag_value_optional_k;
        if (_options._tested_by != hyde::attribute_category::disabled) {
            overloads[key]["tested_by"] = hyde::get_tag(_options._tested_by);
        }
        maybe_annotate(overload, overloads[key]);

        if (!overload["arguments"].empty()) {
            std::size_t j{0};
            auto& args = overloads[key]["arguments"];
            for (const auto& arg : overload["arguments"]) {
                auto& cur_arg = args[j];
                const std::string& name = arg["name"];
                const bool unnamed = name.empty();
                cur_arg["name"] = unnamed ? "unnamed-" + std::to_string(j) : name;
                cur_arg["type"] = arg["type"];
                cur_arg["description"] = tag_value_optional_k;
                if (unnamed) cur_arg["unnamed"] = true;
                ++j;
            }
        }

        ++i;
    }

    json node = base_emitter_node(_as_methods ? "method" : "function", name,
                                  _as_methods ? "method" : "function");

    node["hyde"]["defined_in_file"] = defined_path;
    
    if (!_as_methods && jsn.size() > 0) {
        // All overloads will have the same namespace
        for (const auto& ns : jsn.front()["namespaces"])
            node["hyde"]["namespace"].push_back(static_cast<const std::string&>(ns));
    }
    
    node["hyde"]["overloads"] = std::move(overloads);
    if (is_ctor) node["hyde"]["is_ctor"] = true;
    if (is_dtor) node["hyde"]["is_dtor"] = true;

    return reconcile(std::move(node), _dst_root, dst / (filename + ".md"), out_emitted);
}

/**************************************************************************************************/

} // namespace hyde

/**************************************************************************************************/
