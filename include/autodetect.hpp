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

// application
#include "config.hpp"
#include "filesystem.hpp"

/**************************************************************************************************/

namespace hyde {

/**************************************************************************************************/

std::vector<filesystem::path> autodetect_toolchain_paths();

filesystem::path autodetect_resource_directory();

#if HYDE_PLATFORM(APPLE)
filesystem::path autodetect_sysroot_directory();
#endif // HYDE_PLATFORM(APPLE)

/**************************************************************************************************/

} // namespace hyde

/**************************************************************************************************/
