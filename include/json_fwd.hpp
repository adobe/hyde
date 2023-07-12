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
#include <optional>

// nlohmann/json
#include "nlohmann/json_fwd.hpp"

/**************************************************************************************************/

namespace hyde {

/**************************************************************************************************/

using json = nlohmann::json;

using optional_json = std::optional<json>;

/**************************************************************************************************/

} // namespace hyde

/**************************************************************************************************/
