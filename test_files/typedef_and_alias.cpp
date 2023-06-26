/*
Copyright 2018 Adobe
All Rights Reserved.

NOTICE: Adobe permits you to use, modify, and distribute this file in
accordance with the terms of the Adobe license agreement accompanying
it. If you have received this file from a source other than Adobe,
then your use, modification, or distribution of it requires the prior
written permission of Adobe.
*/

// AST dump with
// clang --std=c++1z -Xclang -ast-dump -fsyntax-only ./test_files/typedef_and_alias.cpp

//------------------------------------------------------------------------------------------------------------------------------------------

/// Example typedef expression whose underlying type is `int`.
typedef int typedef_example;

/// Example using expression whose underlying type is `int`.
using using_example = int;

//------------------------------------------------------------------------------------------------------------------------------------------

/// Example class with two type definitions
template <typename T, typename U>
struct template_example {
    /// Type derived from the first template parameter.
    typedef T typedef_from_T;
    /// Type derived from the second template parameter.
    using using_from_U = U;
};

//------------------------------------------------------------------------------------------------------------------------------------------

/// Partial specialization of the above `template_example` template
template <typename U>
using using_partial_specialization_example = template_example<bool, U>;

/// Full specialization of the above partial specialization
using using_full_specialization_example = using_partial_specialization_example<bool>;

/// Using typedef to define another full specialization of the above partial specialization
typedef using_partial_specialization_example<double> typedef_full_specialization_example;

//------------------------------------------------------------------------------------------------------------------------------------------

struct template_example_user {
    template <typename U>
    using using_partial_specialization_example = template_example<int, U>;
    using using_full_specialization_example = using_partial_specialization_example<bool>;
    typedef using_partial_specialization_example<double> typedef_full_specialization_example;
};

//------------------------------------------------------------------------------------------------------------------------------------------
