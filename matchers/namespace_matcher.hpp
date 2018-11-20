/*
Copyright 2018 Adobe
All Rights Reserved.

NOTICE: Adobe permits you to use, modify, and distribute this file in
accordance with the terms of the Adobe license agreement accompanying
it. If you have received this file from a source other than Adobe,
then your use, modification, or distribution of it requires the prior
written permission of Adobe.
*/

#pragma once

// clang
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"

// application
#include "json.hpp"
#include "matchers/matcher_fwd.hpp"

using namespace clang;
using namespace clang::ast_matchers;

/**************************************************************************************************/

namespace hyde {

/**************************************************************************************************/

class NamespaceInfo : public MatchFinder::MatchCallback {
public:
    NamespaceInfo(std::vector<std::string> paths, processing_options options)
        : _paths(std::move(paths)), _options(options) {
        _j["namespaces"] = json::array();
    }

    void run(const MatchFinder::MatchResult& Result) override;

    json getJSON() { return _j; }

    static DeclarationMatcher GetMatcher() { return namespaceDecl().bind("ns"); }

private:
    std::vector<std::string> _paths;
    processing_options _options;
    json _j;
};

/**************************************************************************************************/

} // namespace hyde

/**************************************************************************************************/
