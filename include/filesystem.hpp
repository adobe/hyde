/*
Copyright 2018 Adobe
All Rights Reserved.

NOTICE: Adobe permits you to use, modify, and distribute this file in
accordance with the terms of the Adobe license agreement accompanying
it. If you have received this file from a source other than Adobe,
then your use, modification, or distribution of it requires the prior
written permission of Adobe.
*/

/**************************************************************************************************/

#pragma once

/**************************************************************************************************/

#define HYDE_FILESYSTEM_PRIVATE_STD() 0
#define HYDE_FILESYSTEM_PRIVATE_STD_EXPERIMENTAL() 1
#define HYDE_FILESYSTEM_PRIVATE_BOOST() 2

/**************************************************************************************************/

#if defined(HYDE_FORCE_BOOST_FILESYSTEM)
    #define HYDE_FILESYSTEM_PRIVATE_SELECTION() HYDE_FILESYSTEM_PRIVATE_BOOST()
#elif defined(__has_include) // Check if __has_include is present
    #if __has_include(<filesystem>)
        #include <filesystem>
        #define HYDE_FILESYSTEM_PRIVATE_SELECTION() HYDE_FILESYSTEM_PRIVATE_STD()
    #elif __has_include(<experimental/filesystem>)
        #include <experimental/filesystem>
        #define HYDE_FILESYSTEM_PRIVATE_SELECTION() HYDE_FILESYSTEM_PRIVATE_STD_EXPERIMENTAL()
    #else
        #define HYDE_FILESYSTEM_PRIVATE_SELECTION() HYDE_FILESYSTEM_PRIVATE_BOOST()
    #endif
#else
    #define HYDE_FILESYSTEM_PRIVATE_SELECTION() HYDE_FILESYSTEM_PRIVATE_BOOST()
#endif

#define HYDE_FILESYSTEM(X) (HYDE_FILESYSTEM_PRIVATE_SELECTION() == HYDE_FILESYSTEM_PRIVATE_##X())

/**************************************************************************************************/
// The library can be used with boost::filesystem, std::experimental::filesystem or std::filesystem.
// Without any additional define, it uses the versions from the standard, if it is available.
//
// If using of boost::filesystem shall be enforced, define HYDE_FORCE_BOOST_FILESYSTEM.

#if HYDE_FILESYSTEM(BOOST)
    #include <boost/filesystem.hpp>
#endif

/**************************************************************************************************/

namespace hyde {

/**************************************************************************************************/

#if HYDE_FILESYSTEM(STD)

    namespace filesystem = std::filesystem;

#elif HYDE_FILESYSTEM(STD_EXPERIMENTAL)

    namespace filesystem = std::experimental::filesystem;

#elif HYDE_FILESYSTEM(BOOST)

    namespace filesystem = boost::filesystem;

#else

    #error `filesystem` variant not specified

#endif

/**************************************************************************************************/

} // namespace hyde

/**************************************************************************************************/
