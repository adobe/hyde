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
/// @brief an example class that demonstrates what Hyde documents.
/// @hyde-owner fosterbrereton
class class_example {
public:
    /// @brief an example enumeration within the class.
    enum class color { red, green = 42, blue };

    /// @brief a class definition contained within `example_class`.
    struct nested_class_example {
        int _x{0};
        int _y;
    };


    /// @brief a nested typedef expression.
    typedef std::string typedef_example;

    /// @brief a nested using expression.
    using using_example = std::string;


    /// @brief default constructor.
    class_example() = default;

    /// @brief an explicit constructor that takes a single `int`.
    explicit class_example(int x) : _x(x) {}

    /// @brief member function with a trailing return type.
    auto member_function_trailing_return_type() const -> int { return _x; }

    /// @brief example member function.
    void member_function() { _x *= 2; }

    /// @brief an overloaded member function that takes one parameter.
    /// @param first the first parameter of the first overload.
    void overloaded(const std::string& first);

    /// @brief an overloaded member function that takes two parameters.
    /// @param first the first parameter of the second overload.
    /// @param second the second parameter of the second overload.
    void overloaded(const std::string& first, const std::string& second) volatile;

    /// @brief another overloaded member function that takes two parameters.
    /// @param first the first parameter of the third overload.
    /// @param second the second parameter of the third overload.
    void overloaded(const std::string& first, std::vector<int> second) const;

    /// @brief an overloaded member function that takes _five_ parameters.
    /// @param first the first parameter of the fourth overload.
    /// @param second the second parameter of the fourth overload.
    /// @param third the third parameter of the fourth overload.
    /// @param fourth the fourth parameter of the fourth overload.
    /// @param fifth the fifth parameter of the fourth overload.
    void overloaded(
        const std::string& first, class_example* second, int third, bool fourth, std::size_t fifth);

    /// @brief an overloaded member function that takes three unnamed parameters.
    /// Let it be known that Doxygen doesn't support documenting unnamed parameters at this time.
    /// There is a bug open on the issue, but as of this writing does not appear to be progressing:
    /// https://github.com/doxygen/doxygen/issues/6926
    void overloaded(const std::string&, class_example*, int); // intentionally unnamed

    /// @brief an overloaded member function that takes zero unnamed parameters.
    [[deprecated]] void overloaded();


    /// @brief deprecated member function
    [[deprecated]] void deprecated(const std::string& first, class_example* second);

    /// @brief deprecated member function that contains a compile-time deprecation message.
    [[deprecated("message")]] void deprecated_with_message(const std::string& s, class_example* f);

    /// @brief static member variable.
    static const int _static_member = 0;

    /// @brief static member function.
    static int static_method() { return 0; };

    /// @brief templatized member function.
    template <typename U>
    void template_member_function() {}

    /// @brief specialization of the above templatized member function.
    template <>
    void template_member_function<double>() {}

private:
    /// @brief some variable that holds an integer.
    int _x;

    /// @brief a deprecated member variable that contains a message. Apparently this works?!
    [[deprecated("message")]] int _deprecated_member = 0;

    /// @brief an instance of the nested class example defined earlier.
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
