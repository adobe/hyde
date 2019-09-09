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
#include <sstream>
#include <unordered_set>

// boost
#include "boost/range/irange.hpp"

// clang/llvm
#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/CommandLine.h"

// application
#include "autodetect.hpp"
#include "filesystem.hpp"
#include "config.hpp"
#include "json.hpp"
#include "output_yaml.hpp"

// instead of this, probably have a matcher manager that pushes the json object
// into the file then does the collation and passes it into jsonAST to do
// anything it needs to do
#include "matchers/class_matcher.hpp"
#include "matchers/enum_matcher.hpp"
#include "matchers/function_matcher.hpp"
#include "matchers/matcher_fwd.hpp"
#include "matchers/namespace_matcher.hpp"
#include "matchers/typealias_matcher.hpp"
#include "matchers/typedef_matcher.hpp"

using namespace clang::tooling;
using namespace llvm;

namespace filesystem = hyde::filesystem;

/**************************************************************************************************/

namespace {

/**************************************************************************************************/

filesystem::path make_absolute(filesystem::path path) {
    if (path.is_absolute()) return path;
    static const auto pwd = filesystem::current_path();
    return canonical(pwd / path);
}

/**************************************************************************************************/

std::string make_absolute(std::string path_string) {
    return make_absolute(filesystem::path(std::move(path_string))).string();
}

/**************************************************************************************************/

std::vector<std::string> make_absolute(std::vector<std::string> paths) {
    for (auto& path : paths)
        path = make_absolute(std::move(path));

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
enum ToolDiagnostic { ToolDiagnosticQuiet, ToolDiagnosticVerbose, ToolDiagnosticVeryVerbose };
static llvm::cl::OptionCategory MyToolCategory(
    "Hyde is a tool to scan library headers to ensure documentation is kept up to\n"
    "date");
static cl::opt<ToolMode> ToolMode(
    cl::desc("There are several modes under which the tool can run:"),
    cl::values(
        clEnumValN(ToolModeJSON, "hyde-json", "JSON analysis (default)"),
        clEnumValN(ToolModeYAMLValidate, "hyde-validate", "Validate existing YAML documentation"),
        clEnumValN(ToolModeYAMLUpdate,
                   "hyde-update",
                   "Write updated YAML documentation for missing elements")),
    cl::cat(MyToolCategory));
static cl::opt<hyde::ToolAccessFilter> ToolAccessFilter(
    cl::desc("Restrict documentation of class elements by their access specifier."),
    cl::values(clEnumValN(hyde::ToolAccessFilterPrivate,
                          "access-filter-private",
                          "Process all elements (default)"),
               clEnumValN(hyde::ToolAccessFilterProtected,
                          "access-filter-protected",
                          "Process only public and protected elements"),
               clEnumValN(hyde::ToolAccessFilterPublic,
                          "access-filter-public",
                          "Process only public elements")),
    cl::cat(MyToolCategory));
static cl::opt<ToolDiagnostic> ToolDiagnostic(
    cl::desc("Several tool diagnostic modes are available:"),
    cl::values(
        clEnumValN(ToolDiagnosticQuiet, "hyde-quiet", "output less to the console (default)"),
        clEnumValN(ToolDiagnosticVerbose, "hyde-verbose", "output more to the console"),
        clEnumValN(ToolDiagnosticVeryVerbose, "hyde-very-verbose", "output much more to the console")),
    cl::cat(MyToolCategory));
static cl::opt<std::string> YamlDstDir("hyde-yaml-dir",
                                       cl::desc("Root directory for YAML validation / update"),
                                       cl::cat(MyToolCategory));
static cl::opt<std::string> YamlSrcDir(
    "hyde-src-root",
    cl::desc("The root path to the header file(s) being analyzed"),
    cl::cat(MyToolCategory));

static cl::opt<std::string> ArgumentResourceDir(
    "resource-dir",
    cl::desc("The resource dir(see clang resource dir) for hyde to use."),
    cl::cat(MyToolCategory));

static cl::opt<bool> AutoResourceDirectory(
    "auto-resource-dir",
    cl::desc("Autodetect and use clang's resource directory"),
    cl::cat(MyToolCategory),
    cl::ValueDisallowed);

static cl::opt<bool> AutoToolchainIncludes(
    "auto-toolchain-includes",
    cl::desc("Autodetect and use the toolchain include paths"),
    cl::cat(MyToolCategory),
    cl::ValueDisallowed);

#if HYDE_PLATFORM(APPLE)
static cl::opt<bool> AutoSysrootDirectory(
    "auto-sysroot",
    cl::desc("Autodetect and use isysroot"),
    cl::cat(MyToolCategory),
    cl::ValueDisallowed);
#endif

static cl::opt<bool> UseSystemClang(
    "use-system-clang",
#if HYDE_PLATFORM(APPLE)
    cl::desc("Synonym for both -auto-resource-dir and -auto-sysroot"),
#else
    cl::desc("Synonym for both -auto-resource-dir and -auto-toolchain-includes"),
#endif
    cl::cat(MyToolCategory),
    cl::ValueDisallowed);

static cl::list<std::string> NamespaceBlacklist(
    "namespace-blacklist",
    cl::desc("Namespace(s) whose contents should not be processed"),
    cl::cat(MyToolCategory),
    cl::CommaSeparated);

static cl::extrahelp HydeHelp(
    "\nThis tool parses the header source(s) using Clang. To pass arguments to the\n"
    "compiler (e.g., include directories), append them after the `--` token on the\n"
    "command line. For example:\n"
    "\n"
    "    hyde -hyde-json input_file.hpp -- -x c++ -I/path/to/includes\n"
    "\n"
    "(The file to be processed must be the last argument before the `--` token.)\n"
    "\n"
    "Alternatively, if you have a compilation database and would like to pass that\n"
    "instead of command-line compiler arguments, you can pass that with -p.\n"
    "\n"
    "While compiling the source file, the non-function macro `ADOBE_TOOL_HYDE` is\n"
    "defined to the value `1`. This can be useful to explicitly omit code from\n"
    "the documentation.\n"
    "\n"
    "Hyde supports project configuration files. It must be named either `.hyde-config`\n"
    "or `_hyde-config`, and must be at or above the file being processed. The\n"
    "format of the file is JSON. This allows you to specify command line\n"
    "parameters in a common location so they do not need to be passed for every\n"
    "file in your project. The flags sent to Clang should be a in a top-level\n"
    "array under the `clang_flags` key.\n"
    "\n");

/**************************************************************************************************/

std::pair<filesystem::path, hyde::json> load_hyde_config(
    filesystem::path src_file) try {
    bool found{false};
    filesystem::path hyde_config_path;

    if (exists(src_file)) {
        const filesystem::path pwd_k = filesystem::current_path();

        if (src_file.is_relative()) {
            src_file = canonical(pwd_k / src_file);
        }

        if (!is_directory(src_file)) {
            src_file = src_file.parent_path();
        }

        const auto hyde_config_check = [&](filesystem::path path) {
            found = exists(path);
            if (found) {
                hyde_config_path = std::move(path);
            }
            return found;
        };

        const auto directory_walk = [hyde_config_check](filesystem::path directory) {
            while (true) {
                if (!exists(directory)) break;
                if (hyde_config_check(directory / ".hyde-config")) break;
                if (hyde_config_check(directory / "_hyde-config")) break;
                directory = directory.parent_path();
            }
        };

        // walk up the directory tree starting from the source file being processed.
        directory_walk(src_file);
    }

    return found ?
               std::make_pair(hyde_config_path.parent_path(),
                              hyde::json::parse(filesystem::ifstream(hyde_config_path))) :
               std::make_pair(filesystem::path(), hyde::json());
} catch (...) {
    throw std::runtime_error("failed to parse the hyde-config file");
}

/**************************************************************************************************/

std::vector<std::string> integrate_hyde_config(int argc, const char** argv) {
    auto cmdline_first = &argv[1];
    auto cmdline_last = &argv[argc];
    auto cmdline_mid = std::find_if(cmdline_first, cmdline_last,
                                    [](const char* arg) { return arg == std::string("--"); });

    const std::vector<std::string> cli_hyde_flags = [argc, cmdline_first, cmdline_mid] {
        std::vector<std::string> result;
        auto hyde_first = cmdline_first;
        auto hyde_last = cmdline_mid;
        while (hyde_first != hyde_last) {
            result.emplace_back(*hyde_first++);
        }
        if (argc == 1) {
            result.push_back("-help");
        }
        return result;
    }();

    const std::vector<std::string> cli_clang_flags = [cmdline_mid, cmdline_last] {
        std::vector<std::string> result;
        auto clang_first = cmdline_mid;
        auto clang_last = cmdline_last;
        while (clang_first != clang_last) {
            std::string arg(*clang_first++);
            if (arg == "--") continue;
            result.emplace_back(std::move(arg));
        }
        return result;
    }();

    std::vector<std::string> hyde_flags;
    std::vector<std::string> clang_flags;
    filesystem::path config_dir;
    hyde::json config;
    std::tie(config_dir, config) =
        load_hyde_config(cli_hyde_flags.empty() ? "" : cli_hyde_flags.back());

    if (exists(config_dir)) current_path(config_dir);

    if (config.count("clang_flags")) {
        for (const auto& clang_flag : config["clang_flags"]) {
            clang_flags.push_back(clang_flag);
        }
    }

    if (config.count("hyde-src-root")) {
        const std::string& path_str = config["hyde-src-root"];
        std::string abs_path_str = make_absolute(path_str);
        hyde_flags.emplace_back("-hyde-src-root=" + abs_path_str);
    }

    if (config.count("hyde-yaml-dir")) {
        const std::string& path_str = config["hyde-yaml-dir"];
        std::string abs_path_str = make_absolute(path_str);
        hyde_flags.emplace_back("-hyde-yaml-dir=" + abs_path_str);
    }

    hyde_flags.insert(hyde_flags.end(), cli_hyde_flags.begin(), cli_hyde_flags.end());
    clang_flags.insert(clang_flags.end(), cli_clang_flags.begin(), cli_clang_flags.end());

    std::vector<std::string> result;

    result.emplace_back(argv[0]);

    result.insert(result.end(), hyde_flags.begin(), hyde_flags.end());

    result.emplace_back("--");

    if (!clang_flags.empty()) {
        // it'd be nice if we could move these into place.
        result.insert(result.end(), clang_flags.begin(), clang_flags.end());
    }

    return result;
}

/**************************************************************************************************/

std::vector<std::string> source_paths(int argc, const char** argv) {
    return CommonOptionsParser(argc, argv, MyToolCategory).getSourcePathList();
}

/**************************************************************************************************/

bool IsVerbose() {
    return ToolDiagnostic == ToolDiagnosticVerbose || ToolDiagnostic == ToolDiagnosticVeryVerbose;
}

/**************************************************************************************************/

int main(int argc, const char** argv) try {
    auto sources = source_paths(argc, argv);
    std::vector<std::string> args = integrate_hyde_config(argc, argv);
    int new_argc = static_cast<int>(args.size());
    std::vector<const char*> new_argv(args.size(), nullptr);

    std::transform(args.begin(), args.end(), new_argv.begin(),
                   [](const auto& arg) { return arg.c_str(); });

    CommonOptionsParser OptionsParser(new_argc, &new_argv[0], MyToolCategory);

    if (UseSystemClang) {
        AutoResourceDirectory = true;
#if HYDE_PLATFORM(APPLE)
        AutoSysrootDirectory = true;
#else
        AutoToolchainIncludes = true;
#endif
    }

    if (IsVerbose()) {
        std::cout << "INFO: Args:\n";
        for (const auto& arg : args) {
            std::cout << "INFO:     " << arg << '\n';
        }
        std::cout << "INFO: Working directory: " << filesystem::current_path().string()
                  << '\n';
    }

    auto sourcePaths = make_absolute(OptionsParser.getSourcePathList());
    // Remove duplicates (CommonOptionsParser is duplicating every single entry)
    std::unordered_set<std::string> s;
    for (std::string i : sourcePaths) {
        s.insert(i);
    }
    sourcePaths.assign(s.begin(), s.end());
    ClangTool Tool(OptionsParser.getCompilations(), sourcePaths);
    MatchFinder Finder;
    hyde::processing_options options{sourcePaths, ToolAccessFilter, NamespaceBlacklist};

    hyde::FunctionInfo function_matcher(options);
    Finder.addMatcher(hyde::FunctionInfo::GetMatcher(), &function_matcher);

    hyde::EnumInfo enum_matcher(options);
    Finder.addMatcher(hyde::EnumInfo::GetMatcher(), &enum_matcher);

    hyde::ClassInfo class_matcher(options);
    Finder.addMatcher(hyde::ClassInfo::GetMatcher(), &class_matcher);

    hyde::NamespaceInfo namespace_matcher(options);
    Finder.addMatcher(hyde::NamespaceInfo::GetMatcher(), &namespace_matcher);

    hyde::TypeAliasInfo typealias_matcher(options);
    Finder.addMatcher(hyde::TypeAliasInfo::GetMatcher(), &typealias_matcher);

    hyde::TypedefInfo typedef_matcher(options);
    Finder.addMatcher(hyde::TypedefInfo::GetMatcher(), &typedef_matcher);

    clang::tooling::CommandLineArguments arguments;

    if (ToolDiagnostic == ToolDiagnosticVeryVerbose) {
        arguments.emplace_back("-v");
    }

#if HYDE_PLATFORM(APPLE)
    //
    // Specify the isysroot directory to the driver
    // 
    // in some versions of osx they have stopped using `/usr/include`; Apple seems to rely
    // on the isysroot parameter to accomplish this task in the general case, so we add it here.
    filesystem::path include_dir{"/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/"};

    if (AutoSysrootDirectory) {
        if (IsVerbose()) {
            std::cout << "INFO: Sysroot autodetected\n";
        }
        include_dir = hyde::autodetect_sysroot_directory();
    }

    if (filesystem::exists(include_dir)) {
        if (IsVerbose()) {
            std::cout << "INFO: Using isysroot: " << include_dir.string() << std::endl;
        }

        arguments.emplace_back(("-isysroot" + include_dir.string()).c_str());
    }
#endif

    //
    // Specify toolchain includes to the driver
    //
    if (AutoToolchainIncludes) {
        std::vector<filesystem::path> includes = hyde::autodetect_toolchain_paths();
        std::cout << "INFO: Toolchain paths autodetected:\n";
        for (const auto& arg : includes) {
            std::cout << "INFO:     " << arg.string() << '\n';

            arguments.emplace_back("-I" + arg.string());
        }
    }

    //
    // Specify the resource directory to the driver
    // 
    // this may not work on windows, need to investigate using strings
    filesystem::path resource_dir{CLANG_RESOURCE_DIR};

    if (AutoResourceDirectory) {
        if (IsVerbose()) {
            std::cout << "INFO: Resource directory autodetected\n";
        }

        resource_dir = hyde::autodetect_resource_directory();
    } else if (!ArgumentResourceDir.empty()) {
        resource_dir = filesystem::path{ArgumentResourceDir};
    }

    if (filesystem::exists(resource_dir)) {
        if (IsVerbose()) {
            std::cout << "INFO: Using resource-dir: " << resource_dir.string() << std::endl;
        }
        arguments.emplace_back("-resource-dir=" + resource_dir.string());
    }

    //
    // Specify the hyde preprocessor macro
    // 
    arguments.emplace_back("-DADOBE_TOOL_HYDE=1");

    Tool.appendArgumentsAdjuster(
        getInsertArgumentAdjuster(arguments, clang::tooling::ArgumentInsertPosition::END));

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

        filesystem::path src_root(YamlSrcDir);
        filesystem::path dst_root(YamlDstDir);

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
