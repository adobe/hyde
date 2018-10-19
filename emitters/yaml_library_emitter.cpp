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

    return failure;
}

/**************************************************************************************************/

bool yaml_library_emitter::emit(const json& j) {
    json node = base_emitter_node("library", "API Documentation", "library");
    node["library-type"] = "library";

    return reconcile(std::move(node), _dst_root, _dst_root / index_filename_k);
}

/**************************************************************************************************/

} // namespace hyde

/**************************************************************************************************/
