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

// clang/llvm
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wall"
#include "clang/AST/ASTConsumer.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/Lex/Lexer.h"
#pragma clang diagnostic pop

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
    if (!_options._process_class_methods) {
        if (llvm::dyn_cast_or_null<CXXMethodDecl>(function)) return;
    }

    auto info_opt = DetailFunctionDecl(_options, function);
    if (!info_opt) return;
    auto info = std::move(*info_opt);

    const std::string& short_name(info["short_name"]);

    // Omit compiler-reserved functions
    if (short_name.find("__") == 0) return;

    _j["functions"][short_name].push_back(std::move(info));
}

/**************************************************************************************************/

} // namespace hyde

/**************************************************************************************************/
