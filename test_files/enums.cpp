/*
Copyright 2018 Adobe
All Rights Reserved.

NOTICE: Adobe permits you to use, modify, and distribute this file in
accordance with the terms of the Adobe license agreement accompanying
it. If you have received this file from a source other than Adobe,
then your use, modification, or distribution of it requires the prior
written permission of Adobe.
*/

/// @file
/// @brief Enumeration sample source file.
/// @hyde-owner fosterbrereton

//------------------------------------------------------------------------------------------------------------------------------------------

/// @brief An example typed enumeration with three values.
/// @hyde-owner fosterbrereton
enum class color_channel {
    /// Red commentary
    red,
    /// Green commentary. Note this enum has a pre-set value.
    green = 42,
    /// Blue commentary
    blue,
};

//------------------------------------------------------------------------------------------------------------------------------------------

/// @brief An example untyped enumeration with three values.
/// @hyde-owner fosterbrereton
enum untyped {
    /// Apple commentary
    apple,
    /// Orange commentary
    orange,
    /// Banana commentary
    banana,
};

//------------------------------------------------------------------------------------------------------------------------------------------
