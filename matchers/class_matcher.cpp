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

// clang/llvm
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Werror"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#pragma clang diagnostic pop

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
    FindStaticMembers(const hyde::processing_options& options)
        : _options(options), _static_members(hyde::json::object()) {}
    bool VisitVarDecl(const VarDecl* d) {
        if (!AccessCheck(_options._access_filter, d->getAccess())) return true;

        auto storage = d->getStorageClass();
        // TODO(Wyles): Do we want to worry about other kinds of storage?
        if (storage == SC_Static) {
            auto type_opt = hyde::StandardDeclInfo(_options, d);
            if (!type_opt) return true;
            auto type = std::move(*type_opt);
            auto name = type["qualified_name"].get<std::string>();
            type["static"] = true;
            type["type"] = hyde::to_string(d, d->getType());
            _static_members[name] = type;
        }
        return true;
    }

    const hyde::json& get_results() const { return _static_members; }

private:
    const hyde::processing_options& _options;
    hyde::json _static_members;
};

namespace hyde {

/**************************************************************************************************/

void ClassInfo::run(const MatchFinder::MatchResult& Result) {
    auto clas = Result.Nodes.getNodeAs<CXXRecordDecl>("class");

    if (!clas->isCompleteDefinition()) return; // e.g., a forward declaration.

    if (clas->isLambda()) return;

    if (!clas->getSourceRange().isValid()) return; // e.g., compiler-injected class specialization

    // e.g., compiler-injected class specializations not caught by the above
    if (auto s = llvm::dyn_cast_or_null<ClassTemplateSpecializationDecl>(clas)) {
        if (!s->getTypeAsWritten()) return;
    }

    auto info_opt = DetailCXXRecordDecl(_options, clas);
    if (!info_opt) return;
    auto info = std::move(*info_opt);

    if (NamespaceBlacklist(_options._namespace_blacklist, info)) return;

    info["kind"] = clas->getKindName();
    info["methods"] = json::object();

    FindConstructor ctor_finder;
    ctor_finder.TraverseDecl(const_cast<Decl*>(static_cast<const Decl*>(clas)));
    if (!ctor_finder) info["ctor"] = "unspecified";

    FindDestructor dtor_finder;
    dtor_finder.TraverseDecl(const_cast<Decl*>(static_cast<const Decl*>(clas)));
    if (!dtor_finder) info["dtor"] = "unspecified";

    FindStaticMembers static_finder(_options);
    static_finder.TraverseDecl(const_cast<Decl*>(static_cast<const Decl*>(clas)));

    if (const auto& template_decl = clas->getDescribedClassTemplate()) {
        info["template_parameters"] = GetTemplateParameters(Result.Context, template_decl);
    }

    for (const auto& method : clas->methods()) {
        auto methodInfo_opt = DetailFunctionDecl(_options, method);
        if (!methodInfo_opt) continue;
        auto methodInfo = std::move(*methodInfo_opt);
        info["methods"][static_cast<const std::string&>(methodInfo["short_name"])].push_back(
            std::move(methodInfo));
    }

    for (const auto& decl : clas->decls()) {
        auto* function_template_decl = dyn_cast<FunctionTemplateDecl>(decl);
        if (!function_template_decl) continue;
        auto methodInfo_opt =
            DetailFunctionDecl(_options, function_template_decl->getTemplatedDecl());
        if (!methodInfo_opt) continue;
        auto methodInfo = std::move(*methodInfo_opt);
        info["methods"][static_cast<const std::string&>(methodInfo["short_name"])].push_back(
            std::move(methodInfo));
    }

    for (const auto& field : clas->fields()) {
        auto fieldInfo_opt = StandardDeclInfo(_options, field);
        if (!fieldInfo_opt) continue;
        auto fieldInfo = std::move(*fieldInfo_opt);
        fieldInfo["type"] = hyde::to_string(field, field->getType());
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
        auto typedefInfo_opt = StandardDeclInfo(_options, type_def);
        if (!typedefInfo_opt) continue;
        auto typedefInfo = std::move(*typedefInfo_opt);

        typedefInfo["type"] = hyde::to_string(type_def, type_def->getUnderlyingType());

        info["typedefs"].push_back(std::move(typedefInfo));
    }

    using typealias_iterator = CXXRecordDecl::specific_decl_iterator<TypeAliasDecl>;
    using typealias_range = llvm::iterator_range<typealias_iterator>;
    typealias_range typealiases(typealias_iterator(clas->decls_begin()),
                                typealias_iterator(CXXRecordDecl::decl_iterator()));
    for (const auto& type_alias : typealiases) {
        // REVISIT (fbrereto) : Refactor this block and TypeAliasInfo::run's.
        auto typealiasInfo_opt = StandardDeclInfo(_options, type_alias);
        if (!typealiasInfo_opt) continue;
        auto typealiasInfo = std::move(*typealiasInfo_opt);

        typealiasInfo["type"] = hyde::to_string(type_alias, type_alias->getUnderlyingType());
        if (auto template_decl = type_alias->getDescribedAliasTemplate()) {
            typealiasInfo["template_parameters"] =
                GetTemplateParameters(Result.Context, template_decl);
        }

        info["typealiases"].push_back(std::move(typealiasInfo));
    }

    _j["classes"].push_back(std::move(info));
}

/**************************************************************************************************/

} // namespace hyde

/**************************************************************************************************/
