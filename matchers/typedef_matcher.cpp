/*
Copyright 2018 Adobe
All Rights Reserved.

NOTICE: Adobe permits you to use, modify, and distribute this file in
accordance with the terms of the Adobe license agreement accompanying
it. If you have received this file from a source other than Adobe,
then your use, modification, or distribution of it requires the prior
written permission of Adobe. 
*/

// identity
#include "typedef_matcher.hpp"

// stdc++
#include <iostream>

// clang/llvm
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wall"
#include "clang/AST/ASTConsumer.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#pragma clang diagnostic pop

// application
#include "json.hpp"
#include "matchers/utilities.hpp"

using namespace clang;
using namespace clang::ast_matchers;

/**************************************************************************************************/

namespace hyde {

/**************************************************************************************************/

void TypedefInfo::run(const MatchFinder::MatchResult& Result) {
    auto node = Result.Nodes.getNodeAs<TypedefDecl>("typedef");
    auto info_opt = StandardDeclInfo(_options, node);
    if (!info_opt) return;
    auto info = std::move(*info_opt);

    // do not process class type aliases here.
    if (!info["parents"].empty()) return;

    info["type"] = hyde::to_string(node, node->getUnderlyingType());

    _j["typedefs"].push_back(std::move(info));
}

/**************************************************************************************************/

} // namespace hyde

/**************************************************************************************************/
