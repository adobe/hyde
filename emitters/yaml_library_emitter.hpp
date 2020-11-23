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

// application
#include "emitters/yaml_base_emitter.hpp"
#include "json.hpp"

/**************************************************************************************************/

namespace hyde {

/**************************************************************************************************/

struct yaml_library_emitter : public yaml_base_emitter {
    explicit yaml_library_emitter(boost::filesystem::path src_root,
                                  boost::filesystem::path dst_root,
                                  yaml_mode mode,
                                  emit_options options)
        : yaml_base_emitter(std::move(src_root), std::move(dst_root), mode, std::move(options), true) {}

    bool emit(const json& j, json& out_emitted) override;

    bool do_merge(const std::string& filepath,
                  const json& have,
                  const json& expected,
                  json& out_merged) override;
};

/**************************************************************************************************/

} // namespace hyde

/**************************************************************************************************/
