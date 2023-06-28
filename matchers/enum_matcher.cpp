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
#include "enum_matcher.hpp"

// stdc++
#include <iostream>

// clang/llvm
// clang-format off
#include "_clang_include_prefix.hpp" // must be first to disable warnings for clang headers
#include "clang/AST/ASTConsumer.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "_clang_include_suffix.hpp" // must be last to re-enable warnings
// clang-format on

// application
#include "json.hpp"
#include "matchers/utilities.hpp"

using namespace clang;
using namespace clang::ast_matchers;

/**************************************************************************************************/

namespace hyde {

/**************************************************************************************************/

void EnumInfo::run(const MatchFinder::MatchResult& Result) {
    auto enumeration = Result.Nodes.getNodeAs<EnumDecl>("enum");
    auto info_opt = StandardDeclInfo(_options, enumeration);
    if (!info_opt) return;
    auto info = std::move(*info_opt);

    // info["scoped"] = enumeration->isScoped();
    // info["fixed"] = enumeration->isFixed();
    info["type"] = hyde::to_string(enumeration, enumeration->getIntegerType());
    info["values"] = json::array();

    for (const auto& p : enumeration->enumerators()) {
        json enumerator = json::object();

        enumerator["name"] = p->getNameAsString();

        if (auto comments = ProcessComments(p)) {
            enumerator["comments"] = std::move(*comments);
        }

        info["values"].push_back(std::move(enumerator));
    }

    _j["enums"].push_back(std::move(info));
}

/**************************************************************************************************/

} // namespace hyde

/**************************************************************************************************/
