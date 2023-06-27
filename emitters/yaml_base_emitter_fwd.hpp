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

// stdc++
#include <stdexcept>
#include <string>

/**************************************************************************************************/

namespace hyde {

/**************************************************************************************************/

static constexpr char const* tag_value_missing_k = "__MISSING__";
static constexpr char const* tag_value_optional_k = "__OPTIONAL__";
static constexpr char const* tag_value_deprecated_k = "__DEPRECATED__";
static constexpr char const* tag_value_inlined_k = "__INLINED__";
static constexpr char const* index_filename_k = "index.md";

/**************************************************************************************************/

enum class attribute_category { disabled, required, optional, deprecated };

static constexpr char const* get_tag(attribute_category c) {
    switch (c) {
        case attribute_category::required:
            return tag_value_missing_k;
        case attribute_category::optional:
            return tag_value_optional_k;
        case attribute_category::deprecated:
            return tag_value_deprecated_k;
        default:
            throw std::invalid_argument("unexpected attribute category");
    }
}

static inline bool is_tag(const std::string& s) { return s.substr(0, 2) == "__"; }

/**************************************************************************************************/

struct emit_options {
    attribute_category _tested_by{attribute_category::disabled};
    bool _ignore_extraneous_files{false};
    bool _fixup_hyde_subfield{false};
};

/**************************************************************************************************/

} // namespace hyde

/**************************************************************************************************/
