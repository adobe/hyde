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
// clang --std=c++1z -Xclang -ast-dump -fsyntax-only ./test_files/classes.cpp

#include <string>
#include <vector>

/**************************************************************************************************/

namespace hyde {

/**************************************************************************************************/

namespace detail {

/**************************************************************************************************/

void do_implementation(); 

/**************************************************************************************************/

} // namespace detail

/**************************************************************************************************/

class foo {
public:
    enum class color {
        red,
        green,
        blue
    };

    struct nested_class {
        int _x;
        int _y;
    };

    typedef std::string my_type;
    using my_other_type = std::string;

    foo() = default;

    explicit foo(int x) : _x(x) {}

    auto get_x() const -> int { return _x; }

    void double_x() { _x *= 2; }

    void overloaded(const std::string& first);
    void overloaded(const std::string& first, const std::string& second) volatile;
    void overloaded(const std::string& first, std::vector<int> second) const;
    void overloaded(const std::string& first, foo* second, int third, bool fourth, std::size_t fifth);
    void overloaded(const std::string&, foo*, int); // intentionally unnamed
    [[deprecated]]
    void deprecated(const std::string& first, foo* second);
    [[deprecated("message")]]
    void deprecated_with_message(const std::string& s, foo* f);

    static const int static_member = 0;

    static int static_method() { return 0; };

private:
    int _x;
    [[deprecated("message")]]
    int deprecated_member = 0;
    nested_class _nested;
};



  template <typename T>
    struct property_limits {
        constexpr auto as_tuple() const { return std::forward_as_tuple(); }
    };

    template<>
    struct property_limits<std::int32_t> {
        using value_type = std::int32_t;
        value_type _min{0};
        value_type _max{0};
        value_type _step{0};
        constexpr auto as_tuple() const { return std::forward_as_tuple(_min, _max, _step); }
    };

    template<>
    struct property_limits<float> {
        using value_type = float;
        value_type _min{0};
        value_type _max{0};
        value_type _step{0};
        // dragons galore here now with floating point comparisons going on.
        constexpr auto as_tuple() const { return std::forward_as_tuple(_min, _max, _step); }
    };


    template<class T1, class T2>
    class A {};    
 
    template<class T>
    class A<int, T> {};


/**************************************************************************************************/

} // namespace hyde

/**************************************************************************************************/
