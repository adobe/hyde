/*
Copyright 2018 Adobe
All Rights Reserved.

NOTICE: Adobe permits you to use, modify, and distribute this file in
accordance with the terms of the Adobe license agreement accompanying
it. If you have received this file from a source other than Adobe,
then your use, modification, or distribution of it requires the prior
written permission of Adobe.
*/

// stdc++
#include <iomanip>
#include <iostream>

// boost
#include "boost/filesystem.hpp"

// clang/llvm
#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/CommandLine.h"

// application
#include "json.hpp"
#include "output_yaml.hpp"

// instead of this, probably have a matcher manager that pushes the json object
// into the file then does the collation and passes it into jsonAST to do
// anything it needs to do
#include "matchers/class_matcher.hpp"
#include "matchers/enum_matcher.hpp"
#include "matchers/function_matcher.hpp"
#include "matchers/namespace_matcher.hpp"
#include "matchers/typealias_matcher.hpp"
#include "matchers/typedef_matcher.hpp"

using namespace clang::tooling;
using namespace llvm;

/**************************************************************************************************/

namespace {

/**************************************************************************************************/

std::string exec(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
    if (!pipe) throw std::runtime_error("popen() failed");
    while (!feof(pipe.get())) {
        if (fgets(buffer.data(), 128, pipe.get()) != nullptr) result += buffer.data();
    }

    while (std::isspace(result.back()))
        result.pop_back();

    return result;
}

/**************************************************************************************************/

std::vector<std::string> make_absolute(std::vector<std::string> paths) {
    for (auto& path : paths) {
        boost::filesystem::path bfp(path);
        if (bfp.is_absolute()) continue;
        static const std::string pwd = exec("pwd");
        static const boost::filesystem::path bfspwd(pwd);
        boost::filesystem::path abs_path(bfspwd / bfp);
        path = canonical(abs_path).string();
    }

    return paths;
}

/**************************************************************************************************/

} // namespace

/**************************************************************************************************/
/*
    Command line arguments section. These are intentionally global. See:
        https://llvm.org/docs/CommandLine.html
*/
enum ToolMode { ToolModeJSON, ToolModeYAMLValidate, ToolModeYAMLUpdate };
static llvm::cl::OptionCategory MyToolCategory(
    "Hyde is a tool to scan library headers to ensure documentation is kept up to\n"
    "date");
static cl::opt<ToolMode> ToolMode(
    cl::desc("There are several modes under which the tool can run:"),
    cl::values(
        clEnumValN(ToolModeJSON, "hyde-json", "JSON analysis"),
        clEnumValN(ToolModeYAMLValidate, "hyde-validate", "Validate existing YAML documentation"),
        clEnumValN(ToolModeYAMLUpdate,
                   "hyde-update",
                   "Write updated YAML documentation for missing elements")),
    cl::cat(MyToolCategory));
static cl::opt<std::string> YamlDstDir("hyde-yaml-dir",
                                       cl::desc("Root directory for YAML validation / update"),
                                       cl::cat(MyToolCategory));
static cl::opt<std::string> YamlSrcDir(
    "hyde-src-root",
    cl::desc("The root path to the header file(s) being analyzed"),
    cl::cat(MyToolCategory));

static cl::extrahelp HydeHelp(
    "\nThis tool parses the header source(s) using Clang. To pass arguments to the\n"
    "compiler (e.g., include directories), append them after the `--` token on the\n"
    "command line. For example:\n"
    "\n"
    "    hyde input_file.hpp -hyde-json-- -x c++ -I/path/to/includes\n"
    "\n"
    "Alternatively, if you have a compilation database and would like to pass that\n"
    "instead of command-line compiler arguments, you can pass that with -p.\n"
    "\n"
    "While compiling the source file, the non-function macro `ADOBE_TOOL_HYDE` is\n"
    "defined to the value `1`. This can be useful to explicitly omit code from\n"
    "the documentation.\n"
    "\n");

/**************************************************************************************************/

const char* clang_path() {
#define ADOBE_HYDE_XSTR(X) ADOBE_HYDE_STR(X)
#define ADOBE_HYDE_STR(X) #X
    return "/Toolchains/XcodeDefault.xctoolchain/usr/lib/clang/" \
            ADOBE_HYDE_XSTR(__clang_major__) "." \
            ADOBE_HYDE_XSTR(__clang_minor__) "." \
            ADOBE_HYDE_XSTR(__clang_patchlevel__) "/include";
#undef ADOBE_HYDE_XSTR
#undef ADOBE_HYDE_STR
}

/**************************************************************************************************/

int main(int argc, const char** argv) try {
    CommonOptionsParser OptionsParser(argc, argv, MyToolCategory);
    auto sourcePaths = make_absolute(OptionsParser.getSourcePathList());
    ClangTool Tool(OptionsParser.getCompilations(), sourcePaths);
    MatchFinder Finder;

    hyde::FunctionInfo function_matcher(sourcePaths);
    Finder.addMatcher(hyde::FunctionInfo::GetMatcher(), &function_matcher);

    hyde::EnumInfo enum_matcher(sourcePaths);
    Finder.addMatcher(hyde::EnumInfo::GetMatcher(), &enum_matcher);

    hyde::ClassInfo class_matcher(sourcePaths);
    Finder.addMatcher(hyde::ClassInfo::GetMatcher(), &class_matcher);

    hyde::NamespaceInfo namespace_matcher(sourcePaths);
    Finder.addMatcher(hyde::NamespaceInfo::GetMatcher(), &namespace_matcher);

    hyde::TypeAliasInfo typealias_matcher(sourcePaths);
    Finder.addMatcher(hyde::TypeAliasInfo::GetMatcher(), &typealias_matcher);

    hyde::TypedefInfo typedef_matcher(sourcePaths);
    Finder.addMatcher(hyde::TypedefInfo::GetMatcher(), &typedef_matcher);

    // Get the current Xcode toolchain and add its include directories to the tool.
    std::string xcode_path = exec("xcode-select -p");

    // Order matters here. The first include path will be looked up first, so should
    // be the highest priority path.
    std::string include_directories[] = {
        xcode_path + clang_path(),
        "/Library/Developer/CommandLineTools/usr/include/c++/v1",
        xcode_path + "/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/usr/include/",
    };

    for (const auto& include : include_directories) {
        Tool.appendArgumentsAdjuster(getInsertArgumentAdjuster(("-I" + include).c_str()));
    }

    //Tool.appendArgumentsAdjuster(getInsertArgumentAdjuster("-xc++"));
    //Tool.appendArgumentsAdjuster(getInsertArgumentAdjuster("-std=c++17"));
    Tool.appendArgumentsAdjuster(getInsertArgumentAdjuster("-DADOBE_TOOL_HYDE=1"));

    if (Tool.run(newFrontendActionFactory(&Finder).get()))
        throw std::runtime_error("compilation failed.");

    hyde::json paths = hyde::json::object();
    paths["src_root"] = YamlSrcDir;
    paths["src_path"] = sourcePaths[0]; // Hmm... including multiple sources
                                        // implies we'd be analyzing multiple
                                        // subcomponents at the same time. We
                                        // should account for this at some
                                        // point.

    hyde::json result = hyde::json::object();
    result["functions"] = function_matcher.getJSON()["functions"];
    result["enums"] = enum_matcher.getJSON()["enums"];
    result["classes"] = class_matcher.getJSON()["classes"];
    result["namespaces"] = namespace_matcher.getJSON()["namespaces"];
    result["typealiases"] = typealias_matcher.getJSON()["typealiases"];
    result["typedefs"] = typedef_matcher.getJSON()["typedefs"];
    result["paths"] = std::move(paths);

    if (ToolMode == ToolModeJSON) {
        // The std::setw(2) is for pretty-printing. Remove it for ugly serialization.
        std::cout << std::setw(2) << result << '\n';
    } else {
        if (YamlDstDir.empty())
            throw std::runtime_error("no YAML output directory specified (-hyde-yaml-dir)");

        boost::filesystem::path src_root(YamlSrcDir);
        boost::filesystem::path dst_root(YamlDstDir);

        output_yaml(std::move(result), std::move(src_root), std::move(dst_root),
                    ToolMode == ToolModeYAMLValidate ? hyde::yaml_mode::validate :
                                                       hyde::yaml_mode::update);
    }
} catch (const std::exception& error) {
    std::cerr << "Error: " << error.what() << '\n';
    return EXIT_FAILURE;
} catch (...) {
    std::cerr << "Error: unknown\n";
    return EXIT_FAILURE;
}

/**************************************************************************************************/
