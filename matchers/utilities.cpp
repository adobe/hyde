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
#include "utilities.hpp"

// stdc++
#include <iostream>
#include <sstream>

// boost
#include "boost/filesystem.hpp"

// clang
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/Type.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/Lex/Lexer.h"
#include "llvm/ADT/ArrayRef.h"

// application
#include "json.hpp"

using namespace clang;
using namespace clang::ast_matchers;

/**************************************************************************************************/

namespace {

/**************************************************************************************************/

void trim_back(std::string& src) {
    std::size_t start(src.size());

    while (start != 0 && (std::isspace(src[start - 1]) || src[start - 1] == '\n'))
        start--;

    src.erase(start, std::string::npos);
}

/**************************************************************************************************/
// If `as_token` is true, the end of the range is assumed to be the beginning of a token, and the
// range will be extended to the end of that token before it is serialized. Otherwise, the end of
// the range is unchanged before serialization.
std::string to_string(const ASTContext* n, SourceRange range, bool as_token) {
    const auto char_range = as_token ? clang::CharSourceRange::getTokenRange(range) :
                                       clang::CharSourceRange::getCharRange(range);
    std::string result =
        Lexer::getSourceText(char_range, n->getSourceManager(), n->getLangOpts()).str();
    trim_back(result);
    return result;
}

/**************************************************************************************************/

std::string to_string(const ASTContext* n, const clang::TemplateDecl* template_decl) {
    std::size_t count{0};
    std::string result = "template <";
    for (const auto& parameter_decl : *template_decl->getTemplateParameters()) {
        if (count++) result += ", ";
        if (auto* template_type = dyn_cast<TemplateTypeParmDecl>(parameter_decl)) {
            result += template_type->wasDeclaredWithTypename() ? "typename" : "class";
            if (template_type->isParameterPack()) result += "...";
            result += " " + template_type->getNameAsString();
        } else if (auto* non_template_type = dyn_cast<NonTypeTemplateParmDecl>(parameter_decl)) {
            result += hyde::to_string(non_template_type, non_template_type->getType());
            if (non_template_type->isParameterPack()) result += "...";
            result += " " + non_template_type->getNameAsString();
        }
    }
    result += ">\n";

    return result;
}

/**************************************************************************************************/

template <typename F>
void ForEachParent(const Decl* d, F f) {
    if (!d) return;

    auto& context = d->getASTContext();
    auto parents = context.getParents(*d);

    while (true) {
        if (parents.size() == 0) break;
        if (parents.size() != 1) {
            assert(false && "What exactly is going on here?");
        }

        auto* parent = parents.begin();

        if (auto* node = parent->get<Decl>()) {
            f(node);
        }

        parents = context.getParents(*parent);
    }
}

/**************************************************************************************************/

enum class signature_options : std::uint8_t {
    none = 0,
    fully_qualified = 1 << 0,
    named_args = 1 << 1,
};

template <typename T>
bool flag_set(const T& value, const T& flag) {
    using type = std::underlying_type_t<T>;
    return (static_cast<type>(value) & static_cast<type>(flag)) != 0;
}

/**************************************************************************************************/
// See DeclPrinter::VisitFunctionDecl in clang/lib/AST/DeclPrinter.cpp for hints
// on how to make this routine better.
std::string GetSignature(const ASTContext* n,
                         const FunctionDecl* function,
                         signature_options options = signature_options::none) {
    if (!function) return "";

    bool fully_qualified = flag_set(options, signature_options::fully_qualified);
    bool named_args = flag_set(options, signature_options::named_args);
    bool isTrailing = false;
    std::stringstream signature;

    if (const auto* fp = function->getType()->getAs<FunctionProtoType>()) {
        isTrailing = fp->hasTrailingReturn();
    }

    if (auto template_decl = function->getDescribedFunctionTemplate()) {
        signature << to_string(n, template_decl);
    }

    if (auto ctor_decl = llvm::dyn_cast_or_null<CXXConstructorDecl>(function)) {
        auto specifier = ctor_decl->getExplicitSpecifier();
        if (specifier.isExplicit()) signature << "explicit ";
    } else if (auto conversion_decl = llvm::dyn_cast_or_null<CXXConversionDecl>(function)) {
        auto specifier = conversion_decl->getExplicitSpecifier();
        if (specifier.isExplicit()) signature << "explicit ";
    }

    if (!isa<CXXConstructorDecl>(function) && !isa<CXXDestructorDecl>(function) &&
        !isa<CXXConversionDecl>(function)) {
        if (function->isConstexpr()) {
            signature << "constexpr ";
        }

        switch (function->getStorageClass()) {
            case SC_Static:
                signature << "static ";
                break;
            case SC_Extern:
                signature << "extern ";
                break;
            default:
                break;
        }

        if (isTrailing) {
            signature << "auto ";
        } else {
            signature << hyde::to_string(function, function->getReturnType()) << " ";
        }
    }

    if (fully_qualified) {
        bool first{true};

        for (const auto& ns : hyde::GetParentNamespaces(n, function)) {
            if (!first) signature << "::";
            first = false;
            signature << static_cast<const std::string&>(ns);
        }

        for (const auto& p : hyde::GetParentCXXRecords(n, function)) {
            if (!first) signature << "::";
            first = false;
            signature << static_cast<const std::string&>(p);
        }

        if (!first) signature << "::";
    }

    if (auto conversionDecl = llvm::dyn_cast_or_null<CXXConversionDecl>(function)) {
        signature << "operator "
                  << hyde::to_string(conversionDecl, conversionDecl->getConversionType());
    } else {
        signature << hyde::PostProcessType(function, function->getNameInfo().getAsString());
    }

    signature << "(";

    for (int i = 0, paramsCount = function->getNumParams(); i < paramsCount; ++i) {
        if (i) signature << ", ";
        auto* paramDecl = function->getParamDecl(i);
        signature << hyde::to_string(paramDecl, paramDecl->getType());
        if (named_args) {
            auto arg_name = function->getParamDecl(i)->getNameAsString();
            if (!arg_name.empty()) {
                signature << " " << std::move(arg_name);
            }
        }
    }

    if (function->isVariadic()) signature << ", ...";
    signature << ")";
    const auto* functionT = llvm::dyn_cast_or_null<FunctionType>(function->getType().getTypePtr());
    bool canHaveCV = functionT || isa<CXXMethodDecl>(function);
    if (isTrailing) {
        if (canHaveCV) {
            // bit of repetition but hey not much.
            if (functionT->isConst()) signature << " const";
            if (functionT->isVolatile()) signature << " volatile";
            if (functionT->isRestrict()) signature << " restrict";
        }

        signature << " -> " << hyde::to_string(function, function->getReturnType());
    }

    if (!canHaveCV) {
        return signature.str();
    }

    if (!isTrailing) {
        if (functionT->isConst()) signature << " const";
        if (functionT->isVolatile()) signature << " volatile";
        if (functionT->isRestrict()) signature << " restrict";
    }

    if (const auto* functionPT =
            dyn_cast_or_null<FunctionProtoType>(function->getType().getTypePtr())) {
        switch (functionPT->getRefQualifier()) {
            case RQ_LValue:
                signature << " &";
                break;
            case RQ_RValue:
                signature << " &&";
                break;
            default:
                break;
        }
    }

    return signature.str();
}

/**************************************************************************************************/

std::string GetShortName(const clang::ASTContext* n, const clang::FunctionDecl* function) {
    // The "short name" is the unqualified-id of the function, running between
    // the return type and the open paren. It's used e.g., for the file names
    // being output.

    std::string result = hyde::PostProcessType(function, function->getNameInfo().getAsString());

    if (!isa<CXXConstructorDecl>(function) && !isa<CXXDestructorDecl>(function) &&
        !isa<CXXConversionDecl>(function)) {
        std::string return_type = hyde::to_string(function, function->getReturnType());
        auto return_pos = result.find(return_type);
        if (return_pos != std::string::npos) {
            result = result.substr(return_pos + return_type.size(), std::string::npos);
        }
    }

    return result;
}

/**************************************************************************************************/

template <typename NodeType>
hyde::json GetParents(const ASTContext* n, const Decl* d) {
    hyde::json result = hyde::json::array();

    if (!n || !d) return result;

    auto parent = const_cast<ASTContext*>(n)->getParents(*d);

    while (true) {
        if (parent.size() == 0) break;
        if (parent.size() != 1) {
            assert(false && "What exactly is going on here?");
        }

        auto node = parent.begin()->get<NodeType>();
        if (node) {
            std::string name = node->getNameAsString();
            if (auto specialization = dyn_cast_or_null<ClassTemplateSpecializationDecl>(node)) {
                if (auto taw = specialization->getTypeAsWritten()) {
                    name = hyde::to_string(specialization, taw->getType());
                } else {
                }
            } else if (auto cxxrecord = dyn_cast_or_null<CXXRecordDecl>(node)) {
                if (auto template_decl = cxxrecord->getDescribedClassTemplate()) {
                    name +=
                        hyde::GetArgumentList(template_decl->getTemplateParameters()->asArray());
                }
            }

            result.push_back(std::move(name));
        }

        parent = const_cast<ASTContext*>(n)->getParents(*parent.begin());
    }

    std::reverse(result.begin(), result.end());

    return result;
}

/**************************************************************************************************/

} // namespace

/**************************************************************************************************/

namespace hyde {

/**************************************************************************************************/

json GetParentNamespaces(const ASTContext* n, const Decl* d) {
    return GetParents<NamespaceDecl>(n, d);
}

/**************************************************************************************************/

json GetParentCXXRecords(const ASTContext* n, const Decl* d) {
    return GetParents<CXXRecordDecl>(n, d);
}

/**************************************************************************************************/

boost::optional<json> DetailCXXRecordDecl(const hyde::processing_options& options,
                                          const clang::CXXRecordDecl* cxx) {
    auto info_opt = StandardDeclInfo(options, cxx);
    if (!info_opt) return info_opt;
    auto info = std::move(*info_opt);

    // overrides for various fields if the record is of a specific sub-type.
    if (auto s = llvm::dyn_cast_or_null<ClassTemplateSpecializationDecl>(cxx)) {
        info["name"] = hyde::to_string(s, s->getTypeAsWritten()->getType());
        info["qualified_name"] = s->getQualifiedNameAsString();
    } else if (auto template_decl = cxx->getDescribedClassTemplate()) {
        std::string arguments = GetArgumentList(template_decl->getTemplateParameters()->asArray());
        info["name"] = static_cast<const std::string&>(info["name"]) + arguments;
        info["qualified_name"] = template_decl->getQualifiedNameAsString();
    }

    return info;
}

/**************************************************************************************************/

json GetTemplateParameters(const ASTContext* n, const clang::TemplateDecl* d) {
    json result = json::array();

    for (const auto& parameter_decl : *d->getTemplateParameters()) {
        json parameter_info = json::object();

        if (const auto& template_type = dyn_cast<TemplateTypeParmDecl>(parameter_decl)) {
            parameter_info["type"] =
                template_type->wasDeclaredWithTypename() ? "typename" : "class";
            if (template_type->isParameterPack()) parameter_info["parameter_pack"] = "true";
            parameter_info["name"] = template_type->getNameAsString();
        } else if (const auto& non_template_type =
                       dyn_cast<NonTypeTemplateParmDecl>(parameter_decl)) {
            parameter_info["type"] =
                hyde::to_string(non_template_type, non_template_type->getType());
            if (non_template_type->isParameterPack()) parameter_info["parameter_pack"] = "true";
            parameter_info["name"] = non_template_type->getNameAsString();
        } else if (const auto& template_template_type =
                       dyn_cast<TemplateTemplateParmDecl>(parameter_decl)) {
            parameter_info["type"] =
                ::to_string(n, template_template_type->getSourceRange(), false);
            if (template_template_type->isParameterPack())
                parameter_info["parameter_pack"] = "true";
            parameter_info["name"] = template_template_type->getNameAsString();
        } else {
            std::cerr << "What type is this thing, exactly?\n";
        }

        result.push_back(std::move(parameter_info));
    }

    return result;
}

/**************************************************************************************************/

boost::optional<json> DetailFunctionDecl(const hyde::processing_options& options, const FunctionDecl* f) {
    auto info_opt = StandardDeclInfo(options, f);
    if (!info_opt) return info_opt;
    auto info = std::move(*info_opt);
    const clang::ASTContext* n = &f->getASTContext();

    info["return_type"] = hyde::to_string(f, f->getReturnType());
    info["arguments"] = json::array();
    info["signature"] = GetSignature(n, f);
    info["signature_with_names"] = GetSignature(n, f, signature_options::named_args);
    info["short_name"] = GetShortName(n, f);
    // redo the name and qualified name for this entry, now that we have a proper function
    info["name"] = info["signature"];
    info["qualified_name"] = GetSignature(n, f, signature_options::fully_qualified);

    if (f->isConstexpr()) info["constexpr"] = true;

    auto storage = f->getStorageClass();
    switch (storage) {
        case SC_Static:
            info["static"] = true;
            break;
        case SC_Extern:
            info["extern"] = true;
            break;
        default:
            break;
    }

    if (const auto* method = llvm::dyn_cast_or_null<CXXMethodDecl>(f)) {
        if (method->isConst()) info["const"] = true;
        if (method->isVolatile()) info["volatile"] = true;
        if (method->isStatic()) info["static"] = true;

        bool is_ctor = isa<CXXConstructorDecl>(method);
        bool is_dtor = isa<CXXDestructorDecl>(method);

        if (is_ctor || is_dtor) {
            if (is_ctor) {
                info["is_ctor"] = true;

                if (auto ctor_decl = llvm::dyn_cast_or_null<CXXConstructorDecl>(method)) {
                    auto specifier = ctor_decl->getExplicitSpecifier();
                    if (specifier.isExplicit()) info["explicit"] = true;
                }
            }
            if (is_dtor) info["is_dtor"] = true;
            if (method->isDeletedAsWritten()) info["delete"] = true;
            if (method->isExplicitlyDefaulted()) info["default"] = true;
        }

        if (auto conversion_decl = llvm::dyn_cast_or_null<CXXConversionDecl>(method)) {
            auto specifier = conversion_decl->getExplicitSpecifier();
            if (specifier.isExplicit()) info["explicit"] = true;
        }
    }

    if (auto template_decl = f->getDescribedFunctionTemplate()) {
        info["template_parameters"] = GetTemplateParameters(n, template_decl);
    }

    for (const auto& p : f->parameters()) {
        json argument = json::object();

        argument["type"] = to_string(p, p->getOriginalType());
        argument["name"] = p->getNameAsString();

        info["arguments"].push_back(std::move(argument));
    }
    return info;
}

/**************************************************************************************************/

std::string GetArgumentList(const llvm::ArrayRef<clang::NamedDecl*> args) {
    std::size_t count{0};
    std::string result("<");

    for (const auto& arg : args) {
        if (count++) {
            result += ", ";
        }
        result += arg->getNameAsString();
    }

    result += ">";

    return result;
}

/**************************************************************************************************/

bool PathCheck(const std::vector<std::string>& paths, const Decl* d, ASTContext* n) {
    auto beginLoc = d->getBeginLoc();
    auto location = beginLoc.printToString(n->getSourceManager());
    std::string path = location.substr(0, location.find(':'));
    return std::find(paths.begin(), paths.end(), path) != paths.end();
}

/**************************************************************************************************/

bool NamespaceBlacklist(const std::vector<std::string>& blacklist, const json& j) {
    // iff one of the namespaces in j are found in the blacklist, return true.
    if (j.count("namespaces")) {
        for (const auto& ns : j["namespaces"]) {
            auto found = std::find(blacklist.begin(), blacklist.end(), ns);
            if (found != blacklist.end()) return true;
        }
    }

    return false;
}

/**************************************************************************************************/

bool AccessCheck(ToolAccessFilter hyde_filter, clang::AccessSpecifier clang_access) {
    // return true iff the element should be included in the documentation based
    // on its access specifier. false otherwise.

    if (clang_access == AS_none) return true;

    switch (hyde_filter) {
        case ToolAccessFilterPrivate:
            return true;
        case ToolAccessFilterProtected:
            switch (clang_access) {
                case AS_public:
                case AS_protected:
                    return true;
                default:
                    return false;
            }
        case ToolAccessFilterPublic:
            switch (clang_access) {
                case AS_public:
                    return true;
                default:
                    return false;
            }
    }
}

/**************************************************************************************************/

std::string PostProcessTypeParameter(const clang::Decl* decl, std::string type) {
    // If our type contains one or more `type-parameter-N-M`s, run up the parent
    // tree in an attempt to resolve them.
    static const std::string needle("type-parameter-");

    auto pos = type.find(needle);

    // Do this early to avoid the following rigamaroll for nearly all use cases.
    if (pos == std::string::npos) return type;

    auto& context = decl->getASTContext();
    std::vector<std::pair<std::string, std::string>> parent_template_types;

    const auto iterate_template_params = [&](TemplateParameterList& tpl) {
        for (const auto& parameter_decl : tpl) {
            if (auto* template_type = dyn_cast<TemplateTypeParmDecl>(parameter_decl)) {
                auto depth = template_type->getDepth();
                auto index = template_type->getIndex();
                auto qualType = context.getTemplateTypeParmType(
                    depth, index, template_type->isParameterPack(), template_type);

                std::string old_type =
                    needle + std::to_string(depth) + "-" + std::to_string(index);
                std::string new_type = hyde::to_string(template_type, qualType);
                parent_template_types.emplace_back(
                    std::make_pair(std::move(old_type), std::move(new_type)));
            }
        }
    };

    ForEachParent(decl, [&](const Decl* parent) {
        if (auto* ctpsd = dyn_cast_or_null<ClassTemplatePartialSpecializationDecl>(parent)) {
            iterate_template_params(*ctpsd->getTemplateParameters());
        } else if (auto* ctd = dyn_cast_or_null<ClassTemplateDecl>(parent)) {
            iterate_template_params(*ctd->getTemplateParameters());
        } else if (auto* ftd = dyn_cast_or_null<FunctionTemplateDecl>(parent)) {
            iterate_template_params(*ftd->getTemplateParameters());
        }
    });

    while (true) {
        auto end_pos = pos + needle.size();

        // This loop is faster than std::string::find_first_not_of
        while (true) {
            auto c = type[end_pos];
            if (!std::isdigit(c) && c != '-') break;
            ++end_pos;
        }

        auto length = end_pos - pos;
        std::string old_type = type.substr(pos, length);

        // sort and lower_bound this? parent_template_types.size() is usually
        // small (< 5 or so), so it might not be worth the effort.
        auto found = std::find_if(parent_template_types.begin(), parent_template_types.end(),
                                  [&](const auto& cur_pair) { return cur_pair.first == old_type; });

        if (found != parent_template_types.end()) {
            const auto& new_type = found->second;
            type.replace(pos, length, new_type);
            pos += new_type.size();
        } else {
            // Not resolved. Skip to avoid infinite loop.
            pos += old_type.size();
        }

        pos = type.find(needle, pos);

        if (pos == std::string::npos) break;
    }

    return type;
}

/**************************************************************************************************/

std::string PostProcessSpacing(std::string type) {
    std::string::size_type pos{0};

    while (true) {
        pos = type.find("> >", pos);
        if (pos == std::string::npos) return type;
        type.replace(pos, 3, ">>");
        pos += 2;
    }
}

/**************************************************************************************************/

std::string PostProcessType(const clang::Decl* decl, std::string type) {
    std::string result = PostProcessTypeParameter(decl, std::move(type));
    result = PostProcessSpacing(std::move(result));
    return result;
}

/**************************************************************************************************/

} // namespace hyde

/**************************************************************************************************/
