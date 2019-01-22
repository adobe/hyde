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
#include "boost/range/irange.hpp"

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
#include "matchers/matcher_fwd.hpp"
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
enum ToolDiagnostic { ToolDiagnosticQuiet, ToolDiagnosticVerbose };
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
        clEnumValN(ToolDiagnosticVerbose, "hyde-verbose", "output more to the console")),
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

std::pair<boost::filesystem::path, hyde::json> load_hyde_config(
    boost::filesystem::path src_file) try {
    bool found{false};
    boost::filesystem::path hyde_config_path;

    if (exists(src_file)) {
        const boost::filesystem::path pwd_k = boost::filesystem::current_path();

        if (src_file.is_relative()) {
            src_file = canonical(pwd_k / src_file);
        }

        if (!is_directory(src_file)) {
            src_file = src_file.parent_path();
        }

        const auto hyde_config_check = [&](boost::filesystem::path path) {
            found = exists(path);
            if (found) {
                hyde_config_path = std::move(path);
            }
            return found;
        };

        const auto directory_walk = [hyde_config_check](boost::filesystem::path directory) {
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
                              hyde::json::parse(boost::filesystem::ifstream(hyde_config_path))) :
               std::make_pair(boost::filesystem::path(), hyde::json());
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
    boost::filesystem::path config_dir;
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
        boost::filesystem::path relative_path =
            static_cast<const std::string&>(config["hyde-src-root"]);
        boost::filesystem::path absolute_path = canonical(config_dir / relative_path);
        hyde_flags.emplace_back("-hyde-src-root=" + absolute_path.string());
    }

    if (config.count("hyde-yaml-dir")) {
        boost::filesystem::path relative_path =
            static_cast<const std::string&>(config["hyde-yaml-dir"]);
        boost::filesystem::path absolute_path = canonical(config_dir / relative_path);
        hyde_flags.emplace_back("-hyde-yaml-dir=" + absolute_path.string());
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

int main(int argc, const char** argv) try {
    std::vector<std::string> args = integrate_hyde_config(argc, argv);
    int new_argc = static_cast<int>(args.size());
    std::vector<const char*> new_argv(args.size(), nullptr);

    std::transform(args.begin(), args.end(), new_argv.begin(),
                   [](const auto& arg) { return arg.c_str(); });

    CommonOptionsParser OptionsParser(new_argc, &new_argv[0], MyToolCategory);

    if (ToolDiagnostic == ToolDiagnosticVerbose) {
        std::cout << "Args:\n";
        for (const auto& arg : args) {
            std::cout << arg << '\n';
        }
    }

    auto sourcePaths = make_absolute(OptionsParser.getSourcePathList());
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
    // this may not work on windows, need to investigate using strings
    boost::filesystem::path resource_dir{CLANG_RESOURCE_DIR};
#ifdef __APPLE__
    // in some versions of osx they have stopped using /usr/include and instead shove it here
    // this doesn't seem to be part of the standard search path for clang so we add it manually
    boost::filesystem::path include_dir{
        "/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/include/"};

    if (boost::filesystem::exists(include_dir)) {
        if (ToolDiagnostic == ToolDiagnosticVerbose) {
            std::cout << "Including: " << include_dir.string() << std::endl;
        }
        arguments.emplace_back(("-I" + include_dir.string()).c_str());
    }
#endif
    if (!ArgumentResourceDir.empty()) {
        resource_dir = boost::filesystem::path{ArgumentResourceDir};
    }
    if (ToolDiagnostic == ToolDiagnosticVerbose) {
        std::cout << "Resource dir: " << resource_dir.string() << std::endl;
    }

    std::string resource_arg("-resource-dir=");
    resource_arg += resource_dir.string();
    arguments.emplace_back("-DADOBE_TOOL_HYDE=1");
    arguments.emplace_back(resource_arg);
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
