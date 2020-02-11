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
#include "yaml_sourcefile_emitter.hpp"

// stdc++
#include <iostream>

// boost
#include "boost/range/irange.hpp"

/**************************************************************************************************/

namespace hyde {

/**************************************************************************************************/

bool yaml_sourcefile_emitter::do_merge(const std::string& filepath,
                                       const json& have,
                                       const json& expected,
                                       json& out_merged) {
    bool failure{false};

    failure |= check_scalar(filepath, have, expected, "", out_merged, "library-type");

    if (expected.count("typedefs")) {
        failure |= check_map(
            filepath, have, expected, "", out_merged, "typedefs",
            [this](const std::string& filepath, const json& have, const json& expected,
                   const std::string& nodepath, json& out_merged) {
                bool failure{false};

                failure |= check_scalar(filepath, have, expected, nodepath, out_merged, "definition");
                failure |= check_scalar(filepath, have, expected, nodepath, out_merged, "description");
                // failure |= check_scalar(filepath, have, expected, nodepath, out_merged, "annotation");

                return failure;
            });
    }

    return failure;
}

/**************************************************************************************************/

bool yaml_sourcefile_emitter::emit(const json& j) {
    const auto sub_path = subcomponent(static_cast<const std::string&>(j["paths"]["src_path"]), _src_root);
    json node = base_emitter_node("library", sub_path.string(), "sourcefile");
    node["library-type"] = "sourcefile";

    insert_typedefs(j, node);

    _sub_dst = dst_path(j, sub_path);

    return reconcile(std::move(node), _dst_root, _sub_dst / index_filename_k);
}

/**************************************************************************************************/

bool yaml_sourcefile_emitter::extraneous_file_check_internal(const boost::filesystem::path& root,
                                                             const boost::filesystem::path& path) {
    bool failure{false};

    for (const auto& entry :
         boost::make_iterator_range(boost::filesystem::directory_iterator(path), {})) {
        if (!checker_s.checked(entry)) {
            boost::filesystem::path entry_path(entry);
            // We need better validation here against the existence of example (cpp) files.
            if (entry_path.extension() != ".cpp") {
                std::string bad_path = boost::filesystem::path(entry).string();
                std::cerr << bad_path << ": extraneous file\n";
                failure = true;
            }
        }

        if (is_directory(entry)) {
            failure |= extraneous_file_check_internal(root, entry);
        }
    }

    return failure;
}

/**************************************************************************************************/

bool yaml_sourcefile_emitter::extraneous_file_check() {
    return extraneous_file_check_internal(_sub_dst, _sub_dst);
}

/**************************************************************************************************/

} // namespace hyde

/**************************************************************************************************/
