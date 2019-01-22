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
#include <string>
#include <vector>

/**************************************************************************************************/

namespace hyde {

/**************************************************************************************************/

enum ToolAccessFilter {
    ToolAccessFilterPrivate,
    ToolAccessFilterProtected,
    ToolAccessFilterPublic,
};

/**************************************************************************************************/

struct processing_options {
    std::vector<std::string> _paths;
    ToolAccessFilter _access_filter;
    std::vector<std::string> _namespace_blacklist;
};

/**************************************************************************************************/

} // namespace hyde

/**************************************************************************************************/
