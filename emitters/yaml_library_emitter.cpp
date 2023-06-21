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
#include "yaml_library_emitter.hpp"

// stdc++
#include <iostream>

/**************************************************************************************************/

namespace hyde {

/**************************************************************************************************/

bool yaml_library_emitter::do_merge(const std::string& filepath,
                                    const json& have,
                                    const json& expected,
                                    json& out_merged) {
    bool failure{false};

    failure |= check_scalar(filepath, have, expected, "", out_merged, "library-type");
    failure |= check_editable_scalar(filepath, have, expected, "", out_merged, "icon");
    failure |= check_editable_scalar(filepath, have, expected, "", out_merged, "tab");
    failure |= check_editable_scalar(filepath, have, expected, "", out_merged, "short_title");

    return failure;
}

/**************************************************************************************************/

bool yaml_library_emitter::emit(const json&, json& out_emitted) {
    json node = base_emitter_node("library", tag_value_missing_k, "library");
    node["hyde"]["library-type"] = "library";
    node["hyde"]["icon"] = tag_value_missing_k;
    node["hyde"]["tab"] = tag_value_missing_k;
    node["hyde"]["short_title"] = tag_value_optional_k;

    return reconcile(std::move(node), _dst_root, _dst_root / index_filename_k, out_emitted);
}

/**************************************************************************************************/

} // namespace hyde

/**************************************************************************************************/
