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

/**************************************************************************************************/

namespace hyde {

/**************************************************************************************************/

bool yaml_class_emitter::do_merge(const std::string& filepath,
                                  const json& have,
                                  const json& expected,
                                  json& out_merged) {
    bool failure{false};

    failure |= check_scalar(filepath, have, expected, "", out_merged, "defined-in-file");
    // failure |= check_scalar(filepath, have, expected, "", out_merged, "annotation");
    failure |= check_scalar(filepath, have, expected, "", out_merged, "declaration");
    // failure |= check_array(filepath, have, expected, "", out_merged, "namespace");
    if (expected.count("ctor"))
        failure |= check_scalar(filepath, have, expected, "", out_merged, "ctor");
    if (expected.count("dtor"))
        failure |= check_scalar(filepath, have, expected, "", out_merged, "dtor");

    if (expected.count("fields")) {
        failure |= check_map(
            filepath, have, expected, "", out_merged, "fields",
            [this](const std::string& filepath, const json& have, const json& expected,
                   const std::string& nodepath, json& out_merged) {
                bool failure{false};

                failure |= check_scalar(filepath, have, expected, nodepath, out_merged, "type");
                failure |=
                    check_scalar(filepath, have, expected, nodepath, out_merged, "description");
                // failure |= check_scalar(filepath, have, expected, nodepath, out_merged,
                // "annotation");

                return failure;
            });
    }

    if (expected.count("typedefs")) {
        failure |= check_map(
            filepath, have, expected, "", out_merged, "typedefs",
            [this](const std::string& filepath, const json& have, const json& expected,
                   const std::string& nodepath, json& out_merged) {
                bool failure{false};

                failure |=
                    check_scalar(filepath, have, expected, nodepath, out_merged, "definition");
                failure |=
                    check_scalar(filepath, have, expected, nodepath, out_merged, "description");
                // failure |= check_scalar(filepath, have, expected, nodepath, out_merged,
                // "annotation");

                return failure;
            });
    }

    return failure;
}

/**************************************************************************************************/

bool yaml_class_emitter::emit(const json& j, json& out_emitted) {
    json node = base_emitter_node("class", j["name"], "class");
    node["defined-in-file"] = defined_in_file(j["defined-in-file"], _src_root);
    maybe_annotate(j, node);

    std::string declaration = format_template_parameters(j, true) + '\n' +
                              static_cast<const std::string&>(j["kind"]) + " " +
                              static_cast<const std::string&>(j["qualified_name"]) + ";";
    node["declaration"] = std::move(declaration);

    for (const auto& ns : j["namespaces"])
        node["namespace"].push_back(static_cast<const std::string&>(ns));

    if (j.count("ctor")) node["ctor"] = static_cast<const std::string&>(j["ctor"]);
    if (j.count("dtor")) node["dtor"] = static_cast<const std::string&>(j["dtor"]);

    if (j.count("fields")) {
        for (const auto& field : j["fields"]) {
            const std::string& key = field["name"];
            auto& field_node = node["fields"][key];
            field_node["type"] = static_cast<const std::string&>(field["type"]);
            field_node["description"] = tag_value_missing_k;
            maybe_annotate(field, field_node);
        }
    }

    insert_typedefs(j, node);

    auto dst = dst_path(j,
                        static_cast<const std::string&>(j["name"]));

    bool failure = reconcile(std::move(node), _dst_root, std::move(dst) / index_filename_k, out_emitted);

    const auto& methods = j["methods"];
    yaml_function_emitter function_emitter(_src_root, _dst_root, _mode, true);

    for (auto it = methods.begin(); it != methods.end(); ++it) {
        function_emitter.set_key(it.key());
        auto function_emitted = hyde::json::object();
        failure |= function_emitter.emit(it.value(), function_emitted);
        out_emitted["methods"].push_back(std::move(function_emitted));
    }

    return failure;
}

/**************************************************************************************************/

} // namespace hyde

/**************************************************************************************************/
