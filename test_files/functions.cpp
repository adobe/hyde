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

int nullary_function_example() { return 0; }

int binary_function_example(int first, int second) { return 0; }

auto overloaded(int first) -> float { return 0; }
auto overloaded(int first, int second) -> double { return 0; }
auto overloaded(int first, int second, int third) -> float { return 0; }

static int static_function_example() { return 0; }

static auto static_auto_function_example() { return 0; }

static auto static_trailing_type_function_example() -> int { return 0; }

template <class T>
T template_function_example() = delete;

template <>
int template_function_example() {
    return 42;
}

//------------------------------------------------------------------------------------------------------------------------------------------
