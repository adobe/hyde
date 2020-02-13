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

/**************************************************************************************************/

namespace hyde {

/**************************************************************************************************/

static constexpr char const* tag_value_missing_k = "__MISSING__";
static constexpr char const* tag_value_optional_k = "__OPTIONAL__";
static constexpr char const* tag_value_deprecated_k = "__DEPRECATED__";
static constexpr char const* index_filename_k = "index.md";

/**************************************************************************************************/

struct emit_options {
    bool _enable_tested_by;
};

/**************************************************************************************************/

} // namespace hyde

/**************************************************************************************************/
