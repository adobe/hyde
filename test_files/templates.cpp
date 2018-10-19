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
// clang --std=c++1z -Xclang -ast-dump -fsyntax-only ./test_files/templates.cpp

#include <functional>

/**************************************************************************************************/

template <std::size_t N, class... EventArgs>
struct banana {
    template <std::size_t M, class... Args>
    bool operator()(Args&&... args) const {
        return _f(std::forward<Args>(args)...);
    }

private:
    std::function<bool(EventArgs...)> _f;
};

/**************************************************************************************************/
