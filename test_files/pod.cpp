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
// clang --std=c++1z -Xclang -ast-dump -fsyntax-only ./test_files/pod.cpp

#include <functional>

/**************************************************************************************************/

template <class T>
class point {
public:
    T x;
    T y;

    static constexpr auto zero() { return point(0, 0); }

    constexpr point() noexcept = default;
    constexpr point(T _x, T _y) noexcept : x(std::move(_x)), y(std::move(_y)) {}

    friend inline bool operator==(const point& a, const point& b) {
        return (a.x == b.x) && (a.y == b.y);
    }
    friend inline bool operator!=(const point& a, const point& b) { return !(a == b); }

    friend inline point operator-(const point& a, const point& b) {
        return point(a.x - b.x, a.y - b.y);
    }

    point& operator-=(const point& a) {
        x -= a.x;
        y -= a.y;
        return *this;
    }
};

/**************************************************************************************************/
