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

//------------------------------------------------------------------------------------------------------------------------------------------

class class_example {
public:
    enum class color { red, green = 42, blue };

    struct nested_class_example {
        int _x{0};
        int _y;
    };

    typedef std::string typedef_example;
    using using_example = std::string;

    class_example() = default;

    explicit class_example(int x) : _x(x) {}

    auto member_function_trailing_return_type() const -> int { return _x; }

    void member_function() { _x *= 2; }

    void overloaded(const std::string& first);
    void overloaded(const std::string& first, const std::string& second) volatile;
    void overloaded(const std::string& first, std::vector<int> second) const;
    void overloaded(
        const std::string& first, class_example* second, int third, bool fourth, std::size_t fifth);
    void overloaded(const std::string&, class_example*, int); // intentionally unnamed
    [[deprecated]] void overloaded();

    [[deprecated]] void deprecated(const std::string& first, class_example* second);
    [[deprecated("message")]] void deprecated_with_message(const std::string& s, class_example* f);

    static const int _static_member = 0;

    static int static_method() { return 0; };

    template <typename U>
    void template_member_function() {}

    template <>
    void template_member_function<double>() {}

private:
    int _x;
    [[deprecated("message")]] int _deprecated_member = 0;
    nested_class_example _nested;
};

//------------------------------------------------------------------------------------------------------------------------------------------

template <typename T>
struct specialization_example {
    constexpr auto as_tuple() const { return std::forward_as_tuple(); }
};

template <>
struct specialization_example<std::int32_t> {
    using value_type = std::int32_t;
    value_type _first{0};
    constexpr auto as_tuple() const { return std::forward_as_tuple(_first); }
};

template <>
struct specialization_example<float> {
    using value_type = float;
    value_type _first{0};
    constexpr auto as_tuple() const { return std::forward_as_tuple(_first); }
};

//------------------------------------------------------------------------------------------------------------------------------------------

template <class T1, class T2>
class partial_specialization_example {
    T1 _first;
    T2 _second;
};

template <class T>
class partial_specialization_example<int, T> {
    std::string _first;
    T _second;
};

//------------------------------------------------------------------------------------------------------------------------------------------
