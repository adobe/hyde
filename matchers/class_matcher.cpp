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
#include "class_matcher.hpp"

// stdc++
#include <iostream>

// clang
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"

// application
#include "json.hpp"
#include "matchers/utilities.hpp"

using namespace clang;
using namespace clang::ast_matchers;

/**************************************************************************************************/

class FindDestructor : public RecursiveASTVisitor<FindDestructor> {
public:
    bool VisitDecl(const Decl* d) {
        if (isa<CXXDestructorDecl>(d)) _found = true;
        return !_found;
    }

    explicit operator bool() const { return _found; }

private:
    bool _found{false};
};

/**************************************************************************************************/

class FindConstructor : public RecursiveASTVisitor<FindConstructor> {
public:
    bool VisitDecl(const Decl* d) {
        if (isa<CXXConstructorDecl>(d)) _found = true;
        return !_found;
    }

    explicit operator bool() const { return _found; }

private:
    bool _found{false};
};

/**************************************************************************************************/

class FindStaticMembers : public RecursiveASTVisitor<FindStaticMembers> {
public:
    FindStaticMembers(ASTContext* context) : context(context), static_members(hyde::json::object()) {}
    bool VisitVarDecl(const VarDecl* d) {
        auto storage = d->getStorageClass();
        // TODO(Wyles): Do we want to worry about other kinds of storage?
        if (storage == SC_Static) {
            auto type = hyde::StandardDeclInfo(context, d);
            auto name = type["qualified_name"].get<std::string>();
            type["static"] = true;
            type["type"] = hyde::to_string(context, d->getType());
            static_members[name] = type;
        }
        return true;
    }

    hyde::json get_results() { return static_members; }

private:
    ASTContext* context = nullptr;
    hyde::json static_members;
};

namespace hyde {

/**************************************************************************************************/

void ClassInfo::run(const MatchFinder::MatchResult& Result) {
    auto clas = Result.Nodes.getNodeAs<CXXRecordDecl>("class");

    if (!PathCheck(_paths, clas, Result.Context)) return;

    if (!clas->isCompleteDefinition()) return; // e.g., a forward declaration.

    if (clas->isLambda()) return;

    json info = DetailCXXRecordDecl(Result.Context, clas);
    info["kind"] = clas->getKindName();
    info["methods"] = json::object();

    FindConstructor ctor_finder;
    ctor_finder.TraverseDecl(const_cast<Decl*>(static_cast<const Decl*>(clas)));
    if (!ctor_finder) info["ctor"] = "unspecified";

    FindDestructor dtor_finder;
    dtor_finder.TraverseDecl(const_cast<Decl*>(static_cast<const Decl*>(clas)));
    if (!dtor_finder) info["dtor"] = "unspecified";

    FindStaticMembers static_finder(Result.Context);
    static_finder.TraverseDecl(const_cast<Decl*>(static_cast<const Decl*>(clas)));

    if (const auto& template_decl = clas->getDescribedClassTemplate()) {
        info["template_parameters"] = GetTemplateParameters(Result.Context, template_decl);
    }

    for (const auto& method : clas->methods()) {
        json methodInfo = DetailFunctionDecl(Result.Context, method);
        info["methods"][static_cast<const std::string&>(methodInfo["short_name"])].push_back(
            std::move(methodInfo));
    }

    for (const auto& decl : clas->decls()) {
        auto* function_template_decl = dyn_cast<FunctionTemplateDecl>(decl);
        if (!function_template_decl) continue;
        json methodInfo =
            DetailFunctionDecl(Result.Context, function_template_decl->getTemplatedDecl());
        info["methods"][static_cast<const std::string&>(methodInfo["short_name"])].push_back(
            std::move(methodInfo));
    }

    for (const auto& field : clas->fields()) {
        json fieldInfo = StandardDeclInfo(Result.Context, field);
        fieldInfo["type"] = hyde::to_string(Result.Context, field->getType());
        info["fields"][static_cast<const std::string&>(fieldInfo["qualified_name"])] =
            fieldInfo; // can't move this into place for some reason.
    }
    hyde::json static_members = static_finder.get_results();
    if (static_members.size() > 0) {
        if (info["fields"].size() == 0) {
            info["fields"] = hyde::json::object();
        }
        info["fields"].insert(static_members.begin(), static_members.end());
    }

    using typedef_iterator = CXXRecordDecl::specific_decl_iterator<TypedefDecl>;
    using typedef_range = llvm::iterator_range<typedef_iterator>;
    typedef_range typedefs(typedef_iterator(clas->decls_begin()),
                           typedef_iterator(CXXRecordDecl::decl_iterator()));
    for (const auto& type_def : typedefs) {
        // REVISIT (fbrereto) : Refactor this block and TypedefInfo::run's.
        json typedefInfo = StandardDeclInfo(Result.Context, type_def);
        typedefInfo["type"] = hyde::to_string(Result.Context, type_def->getUnderlyingType());

        info["typedefs"].push_back(std::move(typedefInfo));
    }

    using typealias_iterator = CXXRecordDecl::specific_decl_iterator<TypeAliasDecl>;
    using typealias_range = llvm::iterator_range<typealias_iterator>;
    typealias_range typealiases(typealias_iterator(clas->decls_begin()),
                                typealias_iterator(CXXRecordDecl::decl_iterator()));
    for (const auto& type_alias : typealiases) {
        // REVISIT (fbrereto) : Refactor this block and TypeAliasInfo::run's.
        json typealiasInfo = StandardDeclInfo(Result.Context, type_alias);
        typealiasInfo["type"] = hyde::to_string(Result.Context, type_alias->getUnderlyingType());
        if (auto template_decl = type_alias->getDescribedAliasTemplate()) {
            typealiasInfo["template_parameters"] = GetTemplateParameters(Result.Context, template_decl);
        }

        info["typealiases"].push_back(std::move(typealiasInfo));
    }

    _j["classes"].push_back(std::move(info));
}

/**************************************************************************************************/

} // namespace hyde

/**************************************************************************************************/
