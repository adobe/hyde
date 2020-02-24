/*
Copyright 2018 Adobe
All Rights Reserved.

NOTICE: Adobe permits you to use, modify, and distribute this file in
accordance with the terms of the Adobe license agreement accompanying
it. If you have received this file from a source other than Adobe,
then your use, modification, or distribution of it requires the prior
written permission of Adobe. 
*/

#pragma once

// boost
#include "boost/filesystem/path.hpp"

// application
#include "json_fwd.hpp"
#include "emitters/yaml_base_emitter_fwd.hpp"

/**************************************************************************************************/

namespace hyde {

/**************************************************************************************************/

enum class yaml_mode {
    validate,
    update,
};

/**************************************************************************************************/

void output_yaml(json j,
                 const boost::filesystem::path& src_root,
                 const boost::filesystem::path& dst_root,
                 json& out_emitted,
                 yaml_mode mode,
                 const emit_options& options);

/**************************************************************************************************/

} // namespace hyde

/**************************************************************************************************/
