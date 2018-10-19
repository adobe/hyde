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
#include "function_matcher.hpp"

// stdc++
#include <iostream>

// clang
#include "clang/AST/ASTConsumer.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/Lex/Lexer.h"

// application
#include "json.hpp"
#include "matchers/utilities.hpp"

using namespace clang;
using namespace clang::ast_matchers;

/**************************************************************************************************/

namespace hyde {

/**************************************************************************************************/

void FunctionInfo::run(const MatchFinder::MatchResult& Result) {
    auto function = Result.Nodes.getNodeAs<FunctionDecl>("func");

    // Do not process class methods here.
    if (llvm::dyn_cast_or_null<CXXMethodDecl>(function)) return;

    if (!PathCheck(_paths, function, Result.Context)) return;

    auto info = DetailFunctionDecl(Result.Context, function);

    _j["functions"][static_cast<const std::string&>(info["qualified_name"])].
        push_back(std::move(info));
}

/**************************************************************************************************/

} // namespace hyde

/**************************************************************************************************/
