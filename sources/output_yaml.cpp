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
#include "output_yaml.hpp"

// stdc++
#include <iostream>

// boost
#include "boost/filesystem.hpp"
#include "boost/filesystem/fstream.hpp"

// yaml-cpp
#include "yaml-cpp/yaml.h"

// application
#include "emitters/yaml_class_emitter.hpp"
#include "emitters/yaml_enum_emitter.hpp"
#include "emitters/yaml_function_emitter.hpp"
#include "emitters/yaml_library_emitter.hpp"
#include "emitters/yaml_sourcefile_emitter.hpp"
#include "json.hpp"

/**************************************************************************************************/

namespace hyde {

/**************************************************************************************************/

void output_yaml(json j,
                 const boost::filesystem::path& src_root,
                 const boost::filesystem::path& dst_root,
                 yaml_mode mode) {
    bool failure{false};

    // Process top-level library
    yaml_library_emitter(src_root, dst_root, mode).emit(j);

    // Process sourcefile
    yaml_sourcefile_emitter sourcefile_emitter(src_root, dst_root, mode);
    sourcefile_emitter.emit(j);

    // Process classes
    yaml_class_emitter class_emitter(src_root, dst_root, mode);
    for (const auto& c : j["classes"]) {
        failure |= class_emitter.emit(c);
    }

    // Process enums
    yaml_enum_emitter enum_emitter(src_root, dst_root, mode);
    for (const auto& c : j["enums"]) {
        failure |= enum_emitter.emit(c);
    }

    // Process functions
    yaml_function_emitter function_emitter(src_root, dst_root, mode, false);
    const auto& functions = j["functions"];
    for (auto it = functions.begin(); it != functions.end(); ++it) {
        function_emitter.set_key(it.key());
        failure |= function_emitter.emit(it.value());
    }

    // Check for extra files. Always do this last.
    failure |= sourcefile_emitter.extraneous_file_check();

    if (failure && mode == yaml_mode::validate)
        throw std::runtime_error("YAML documentation failed to validate.");
}

/**************************************************************************************************/

} // namespace hyde

/**************************************************************************************************/
