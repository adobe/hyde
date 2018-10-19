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

/**************************************************************************************************/

typedef int foo_t;

using bar_using = int;

template <typename T, typename U> struct templ {
    typedef T t_typedef;

    using u_using = U;
};

template <typename U> using partial_using = templ<bool, U>;

using full_using = partial_using<bool>;

typedef partial_using<double> full_typedef;

namespace adobe {

struct templ_user {
    template <typename U> using partial_using = templ<int, U>;

    using full_using = partial_using<bool>;

    typedef partial_using<bool> full_typedef;
};

} // namespace adobe

/**************************************************************************************************/
