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
            failure |= check_editable_scalar(filepath, have, expected, nodepath, out_merged,
                                             "description");
            failure |= check_scalar_array(filepath, have, expected, "", out_merged, "values");

            check_inline_comments(expected, out_merged);

            return failure;
        });

    check_inline_comments(expected, out_merged);

    return failure;
}

/**************************************************************************************************/

bool yaml_enum_emitter::emit(const json& j, json& out_emitted, const json& inherited) {
    const std::string& name = j["name"];

    // Most likely an enum forward declaration. Nothing to document here.
    if (j["values"].empty()) return true;

    json base_node =
        base_emitter_node("enumeration", j["name"], "enumeration", has_json_flag(j, "implicit"));
    json& node = base_node["hyde"];

    insert_inherited(inherited, node);
    insert_annotations(j, node);
    insert_doxygen(j, node);

    if (has_inline_field(node, "owner")) {
        node["owner"] = tag_value_inlined_k;
    }

    if (has_inline_field(node, "brief")) {
        node["brief"] = tag_value_inlined_k;
    }

    node["defined_in_file"] = defined_in_file(j["defined_in_file"], _src_root);

    std::string filename;
    for (const auto& ns : j["namespaces"]) {
        const std::string& namespace_str = ns;
        node["namespace"].push_back(namespace_str);
        filename += namespace_str + "::";
    }
    filename = filename_filter(std::move(filename) + name) + ".md";

    for (const auto& value : j["values"]) {
        json cur_value;
        insert_doxygen(value, cur_value);

        cur_value["name"] = value["name"];
        cur_value["description"] =
            has_inline_field(cur_value, "description") ? tag_value_inlined_k : tag_value_missing_k;
        node["values"].push_back(std::move(cur_value));
    }

    return reconcile(std::move(base_node), _dst_root, dst_path(j) / filename, out_emitted);
}

/**************************************************************************************************/

} // namespace hyde

/**************************************************************************************************/
