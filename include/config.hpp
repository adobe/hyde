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

#define HYDE_PRIVATE_STRING_XSMASH(X, Y) X##Y
#define HYDE_PRIVATE_STRING_SMASH(X, Y) HYDE_PRIVATE_STRING_XSMASH(X, Y)()

#define HYDE_PLATFORM_PRIVATE_APPLE() 0
#define HYDE_PLATFORM_PRIVATE_MICROSOFT() 0

#define HYDE_PLATFORM(X) HYDE_PRIVATE_STRING_SMASH(HYDE_PLATFORM_PRIVATE_, X)

#if defined(__APPLE__)
    #undef HYDE_PLATFORM_PRIVATE_APPLE
    #define HYDE_PLATFORM_PRIVATE_APPLE() 1
#elif defined(_MSC_VER)
    #undef HYDE_PLATFORM_PRIVATE_MICROSOFT
    #define HYDE_PLATFORM_PRIVATE_MICROSOFT() 1
#endif

/**************************************************************************************************/
