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

#include <utility>

//------------------------------------------------------------------------------------------------------------------------------------------

/// @brief An example point class
template <class T>
struct point {
    /// The `x` coordinate of the point.
    T x{0};
    /// The `y` coordinate of the point.
    T y{0};

    /// A static routine that returns the origin point.
    /// @return The point `(0, 0)`
    static constexpr auto origin() { return point(); }


    /// Default constructor of default definition.
    constexpr point() noexcept = default;

    /// Value-based constructor that takes `x` and `y` values and sinks them into place.
    /// @param _x The `x` coordinate to sink.
    /// @param _y The `y` coordinate to sink.
    constexpr point(T _x, T _y) noexcept : x(std::move(_x)), y(std::move(_y)) {}

    /// Equality operator.
    /// @return `true` iff the two points' `x` and `y` coordinates are memberwise equal.
    friend inline constexpr bool operator==(const point& a, const point& b) {
        return (a.x == b.x) && (a.y == b.y);
    }

    /// Inequality operator.
    /// @return `true` iff the two points' `x` or `y` coordinates are memberwise inequal.
    friend inline constexpr bool operator!=(const point& a, const point& b) { return !(a == b); }

    /// Subtraction operator.
    /// @param a The point to be subtracted from.
    /// @param b The point to subtract.
    /// @return A new point whose axis values are subtractions of the two inputs' axis values.
    friend inline constexpr point operator-(const point& a, const point& b) {
        return point(a.x - b.x, a.y - b.y);
    }

    /// Subtraction-assignment operator.
    /// @param a The point to subtract from this point
    /// @return A reference to `this`.
    constexpr point& operator-=(const point& a) {
        x -= a.x;
        y -= a.y;
        return *this;
    }
};

//------------------------------------------------------------------------------------------------------------------------------------------
