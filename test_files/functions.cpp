/*
Copyright 2018 Adobe
All Rights Reserved.

NOTICE: Adobe permits you to use, modify, and distribute this file in
accordance with the terms of the Adobe license agreement accompanying
it. If you have received this file from a source other than Adobe,
then your use, modification, or distribution of it requires the prior
written permission of Adobe.
*/

//------------------------------------------------------------------------------------------------------------------------------------------

/// an example nullary function.
/// @return `0`
int nullary_function_example() { return 0; }


/// an example binary function.
/// @param first the first input
/// @param second the second input
/// @return The sum of the two input parameters.
int binary_function_example(int first, int second) { return 0; }


/// an example unary overloaded function
/// @param first the first input parameter
/// @return `first`
auto overloaded(int first) -> float { return first; }

/// an example binary overloaded function
/// @param first the first input parameter
/// @param second the second input parameter
/// @return the product of `first` and `second`
auto overloaded(int first, int second) -> double { return first * second; }

/// an example tertiary overloaded function
/// @param first the first input parameter
/// @param second the second input parameter
/// @param third the third input parameter
/// @return the product of `first`, `second`, and `third`
auto overloaded(int first, int second, int third) -> float { return first * second * third; }


/// an example static function
/// @return `0`
static int static_function_example() { return 0; }


/// an example static function with `auto` return type
/// @return `0`
static auto static_auto_function_example() { return 0; }


/// an example static function with trailing return type
/// @return `0`
static auto static_trailing_type_function_example() -> int { return 0; }


/// an example template function, deleted by default
/// @return Not applicable, seeing that the default definition has been deleted.
template <class T>
T template_function_example() = delete;

/// an example specialization of the template function example
/// @return Forty-two
template <>
int template_function_example() {
    return 42;
}

//------------------------------------------------------------------------------------------------------------------------------------------
