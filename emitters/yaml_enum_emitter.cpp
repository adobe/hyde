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
#include "yaml_enum_emitter.hpp"

// stdc++
#include <iostream>

/**************************************************************************************************/

namespace hyde {

/**************************************************************************************************/

bool yaml_enum_emitter::do_merge(const std::string& filepath,
                                 const json& have,
                                 const json& expected,
                                 json& out_merged) {
    bool failure{false};

    failure |= check_scalar(filepath, have, expected, "", out_merged, "defined_in_file");
    failure |= check_scalar_array(filepath, have, expected, "", out_merged, "annotation");
    failure |= check_scalar_array(filepath, have, expected, "", out_merged, "namespace");

    failure |= check_object_array(
        filepath, have, expected, "", out_merged, "values", "name",
        [this](const std::string& filepath, const json& have, const json& expected,
               const std::string& nodepath, json& out_merged) {
            bool failure{false};
            
            failure |= check_scalar(filepath, have, expected, nodepath, out_merged, "name");
            failure |= check_editable_scalar(filepath, have, expected, nodepath, out_merged, "description");
            failure |= check_scalar_array(filepath, have, expected, "", out_merged, "values");

            copy_inline_comments(expected, out_merged);

            return failure;
        });

    copy_inline_comments(expected, out_merged);

    return failure;
}

/**************************************************************************************************/

bool yaml_enum_emitter::emit(const json& j, json& out_emitted) {
    const std::string& name = j["name"];

    // Most likely an enum forward declaration. Nothing to document here.
    if (j["values"].empty()) return true;

    json base_node = base_emitter_node("enumeration", j["name"], "enumeration", has_json_flag(j, "implicit"));
    json& node = base_node["hyde"];

    node["defined_in_file"] = defined_in_file(j["defined_in_file"], _src_root);
    insert_annotations(j, node);
    insert_doxygen(j, node);

    std::string filename;
    for (const auto& ns : j["namespaces"]) {
        const std::string& namespace_str = ns;
        node["namespace"].push_back(namespace_str);
        filename += namespace_str + "::";
    }
    filename = filename_filter(std::move(filename) + name) + ".md";

    for (const auto& value : j["values"]) {
        json cur_value;
        cur_value["name"] = value["name"];
        cur_value["description"] = tag_value_missing_k;
        insert_doxygen(value, cur_value);
        node["values"].push_back(std::move(cur_value));
    }

    return reconcile(std::move(base_node), _dst_root, dst_path(j) / filename, out_emitted);
}

/**************************************************************************************************/

} // namespace hyde

/**************************************************************************************************/
