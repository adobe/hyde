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
// clang --std=c++1z -Xclang -ast-dump -fsyntax-only ./test_files/template_specialization.cpp

/**************************************************************************************************/

template <typename T, typename U>
struct foo {
    foo() = default;
    int _template;
    T _x;
    U _y;
};

// partial specialization
template <typename U>
struct foo<bool, U> {
    foo() = default;

    int _partial_specialization;
    bool _x;
    U _y;
};

// full specialization
template <>
struct foo<bool, int> {
    foo() = default;

    int _full_specialization;
    bool _x;
    int _y;
};

template <typename T>
struct bar {
    template <typename U>
    static void func() {

    }

    template <>
    static void func<double>() {

    }
};

/**************************************************************************************************/
