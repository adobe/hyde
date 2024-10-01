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
#include "yaml_class_emitter.hpp"

// stdc++
#include <iostream>

// application
#include "emitters/yaml_function_emitter.hpp"
#include "matchers/utilities.hpp"

/**************************************************************************************************/

namespace hyde {

/**************************************************************************************************/

bool yaml_class_emitter::do_merge(const std::string& filepath,
                                  const json& have,
                                  const json& expected,
                                  json& out_merged) {
    bool failure{false};

    failure |= check_scalar(filepath, have, expected, "", out_merged, "defined_in_file");
    failure |= check_scalar_array(filepath, have, expected, "", out_merged, "annotation");
    failure |= check_scalar(filepath, have, expected, "", out_merged, "declaration");
    failure |= check_scalar_array(filepath, have, expected, "", out_merged, "namespace");
    failure |= check_scalar(filepath, have, expected, "", out_merged, "ctor");
    failure |= check_scalar(filepath, have, expected, "", out_merged, "dtor");
    failure |= check_map(
        filepath, have, expected, "", out_merged, "fields",
        [this](const std::string& filepath, const json& have, const json& expected,
               const std::string& nodepath, json& out_merged) {
            bool failure{false};

            failure |= check_scalar(filepath, have, expected, nodepath, out_merged, "type");
            failure |= check_editable_scalar(filepath, have, expected, nodepath, out_merged,
                                             "description");
            failure |=
                check_scalar_array(filepath, have, expected, nodepath, out_merged, "annotation");

            check_inline_comments(expected, out_merged);

            return failure;
        });

    failure |= check_typedefs(filepath, have, expected, "", out_merged);

    failure |= check_object_array(
        filepath, have, expected, "", out_merged, "methods", "title",
        [this](const std::string& filepath, const json& have, const json& expected,
               const std::string& nodepath, json& out_merged) {
            yaml_function_emitter function_emitter(_src_root, _dst_root, _mode, _options, true);
            return function_emitter.do_merge(filepath, have, expected, out_merged);
        });

    check_inline_comments(expected, out_merged);

    return failure;
}

/**************************************************************************************************/

bool yaml_class_emitter::emit(const json& j, json& out_emitted, const json& inherited) {
    json node = base_emitter_node("class", j["name"], "class", has_json_flag(j, "implicit"));
    node["hyde"]["defined_in_file"] = defined_in_file(j["defined_in_file"], _src_root);

    insert_inherited(inherited, node["hyde"]);
    insert_annotations(j, node["hyde"]);
    insert_doxygen(j, node["hyde"]);

    std::string declaration = format_template_parameters(j, true) + '\n' +
                              static_cast<const std::string&>(j["kind"]) + " " +
                              static_cast<const std::string&>(j["qualified_name"]) + ";";
    node["hyde"]["declaration"] = std::move(declaration);

    for (const auto& ns : j["namespaces"])
        node["hyde"]["namespace"].push_back(static_cast<const std::string&>(ns));

    if (j.count("ctor")) node["hyde"]["ctor"] = static_cast<const std::string&>(j["ctor"]);
    if (j.count("dtor")) node["hyde"]["dtor"] = static_cast<const std::string&>(j["dtor"]);

    if (j.count("fields")) {
        for (const auto& field : j["fields"]) {
            const std::string& key = field["name"];
            auto& field_node = node["hyde"]["fields"][key];
            insert_annotations(field, field_node);
            insert_doxygen(field, field_node);

            field_node["type"] = static_cast<const std::string&>(field["type"]);
            const bool inline_description_exists =
                field_node.count("inline") && field_node["inline"].count("description");
            field_node["description"] =
                inline_description_exists ? tag_value_inlined_k : tag_value_missing_k;
        }
    }

    insert_typedefs(j, node, inherited);

    auto dst = dst_path(j, static_cast<const std::string&>(j["name"]));

    if (_mode == yaml_mode::transcribe && !exists(dst)) {
        // In this case the symbol name has changed, which has caused a change to the directory name
        // we are now trying to load and reconcile with what we've created. In this case, we can
        // assume the "shape" of the documentation is the same, which means that within the parent
        // folder of `dst` is the actual source folder that holds the old documentation, just under
        // a different name. Find that folder and rename it.

        std::filesystem::rename(derive_transcription_src_path(dst, node["title"]), dst);
    }

    bool failure =
        reconcile(std::move(node), _dst_root, std::move(dst) / index_filename_k, out_emitted);

    yaml_function_emitter function_emitter(_src_root, _dst_root, _mode, _options, true);
    auto emitted_methods = hyde::json::array();
    const auto& methods = j["methods"];

    for (auto it = methods.begin(); it != methods.end(); ++it) {
        function_emitter.set_key(it.key());
        auto function_emitted = hyde::json::object();
        failure |= function_emitter.emit(it.value(), function_emitted, out_emitted.at("hyde"));
        emitted_methods.push_back(std::move(function_emitted));
    }

    out_emitted["methods"] = std::move(emitted_methods);

    return failure;
}

/**************************************************************************************************/

} // namespace hyde

/**************************************************************************************************/
