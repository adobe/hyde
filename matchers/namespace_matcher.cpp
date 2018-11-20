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
#include "namespace_matcher.hpp"

// stdc++
#include <iostream>

// clang
#include "clang/AST/ASTConsumer.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"

// application
#include "json.hpp"
#include "matchers/utilities.hpp"

using namespace clang;
using namespace clang::ast_matchers;

/**************************************************************************************************/

namespace hyde {

/**************************************************************************************************/

void NamespaceInfo::run(const MatchFinder::MatchResult& Result) {
    auto ns = Result.Nodes.getNodeAs<NamespaceDecl>("ns");
    auto info_opt = StandardDeclInfo(_options, ns);
    if (!info_opt) return;
    auto info = std::move(*info_opt);

    _j["namespaces"].push_back(std::move(info));
}

/**************************************************************************************************/

} // namespace hyde

/**************************************************************************************************/
