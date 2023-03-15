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

/**************************************************************************************************/

namespace hyde {

/**************************************************************************************************/

bool yaml_sourcefile_emitter::do_merge(const std::string& filepath,
                                       const json& have,
                                       const json& expected,
                                       json& out_merged) {
    bool failure{false};

    failure |= check_scalar(filepath, have, expected, "", out_merged, "library-type");
    failure |= check_typedefs(filepath, have, expected, "", out_merged);

    return failure;
}

/**************************************************************************************************/

bool yaml_sourcefile_emitter::emit(const json& j, json& out_emitted) {
    const auto sub_path = subcomponent(static_cast<const std::string&>(j["paths"]["src_path"]), _src_root);
    json node = base_emitter_node("library", sub_path.string(), "sourcefile");
    node["library-type"] = "sourcefile";

    insert_typedefs(j, node);

    _sub_dst = dst_path(j, sub_path);

    return reconcile(std::move(node), _dst_root, _sub_dst / index_filename_k, out_emitted);
}

/**************************************************************************************************/

bool yaml_sourcefile_emitter::extraneous_file_check_internal(const std::filesystem::path& root,
                                                             const std::filesystem::path& path) {
    bool failure{false};

    std::filesystem::directory_iterator first(path);
    std::filesystem::directory_iterator last;

    while (first != last) {
        const auto& entry = *first;

        if (!checker_s.checked(entry)) {
            std::filesystem::path entry_path(entry);
            if (entry_path.filename() == ".DS_Store") {
                std::cerr << entry_path.string() << ": Unintended OS file (not a failure)\n";
            } else if (entry_path.extension() != ".cpp") {
                // We need better validation here against the existence of example (cpp) files.
                std::cerr << entry_path.string() << ": extraneous file\n";
                failure = true;
            }
        }

        if (is_directory(entry)) {
            failure |= extraneous_file_check_internal(root, entry);
        }

        ++first;
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
