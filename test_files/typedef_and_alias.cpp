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

typedef int typedef_example;
using using_example = int;

//------------------------------------------------------------------------------------------------------------------------------------------

template <typename T, typename U>
struct template_example {
    typedef T typedef_from_T;
    using using_from_U = U;
};

//------------------------------------------------------------------------------------------------------------------------------------------

template <typename U>
using using_partial_specialization_example = template_example<bool, U>;
using using_full_specialization_example = using_partial_specialization_example<bool>;
typedef using_partial_specialization_example<double> typedef_full_specialization_example;

//------------------------------------------------------------------------------------------------------------------------------------------

struct template_example_user {
    template <typename U>
    using using_partial_specialization_example = template_example<int, U>;
    using using_full_specialization_example = using_partial_specialization_example<bool>;
    typedef using_partial_specialization_example<double> typedef_full_specialization_example;
};

//------------------------------------------------------------------------------------------------------------------------------------------
