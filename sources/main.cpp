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
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <optional>
#include <sstream>
#include <unordered_set>

// clang/llvm
#include "_clang_include_prefix.hpp" // must be first to disable warnings for clang headers
#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/CommandLine.h"
#include "_clang_include_suffix.hpp" // must be last to re-enable warnings

// application
#include "autodetect.hpp"
#include "config.hpp"
#include "json.hpp"
#include "output_yaml.hpp"
#include "emitters/yaml_base_emitter_fwd.hpp"

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

std::filesystem::path make_absolute(std::filesystem::path path) {
    if (path.is_absolute()) return path;
    static const auto pwd = std::filesystem::current_path();
    std::error_code ec;
    const auto relative = pwd / path;
    auto result = weakly_canonical(relative, ec);

    if (!ec) {
        return result;
    }

    throw std::runtime_error("make_absolute: \"" + relative.string() + "\": " + ec.message());
}

/**************************************************************************************************/

std::string make_absolute(std::string path_string) {
    return make_absolute(std::filesystem::path(std::move(path_string))).string();
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
enum ToolMode { ToolModeJSON, ToolModeYAMLValidate, ToolModeYAMLUpdate, ToolModeFixupSubfield };
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
                   "Write updated YAML documentation for missing elements"),
        clEnumValN(ToolModeFixupSubfield,
                   "hyde-fixup-subfield",
                   "Fix-up preexisting documentation; move all fields except `layout` and `title` into a `hyde` subfield. Note this mode is unique in that it takes pre-existing documentation as source(s), not a C++ source file.")
    ),
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
static cl::opt<std::string> YamlDstDir(
    "hyde-yaml-dir",
    cl::desc("Root directory for YAML validation / update"),
    cl::cat(MyToolCategory));

static cl::opt<bool> EmitJson(
    "hyde-emit-json",
    cl::desc("Output JSON emitted from operation"),
    cl::cat(MyToolCategory),
    cl::ValueDisallowed);

static cl::opt<hyde::attribute_category> TestedBy(
    "hyde-tested-by",
    cl::values(
        clEnumValN(hyde::attribute_category::disabled, "disabled", "Disable tested_by attribute (default)"),
        clEnumValN(hyde::attribute_category::required, "required", "Require tested_by attribute"),
        clEnumValN(hyde::attribute_category::optional, "optional", "Enable tested_by attribute with optional value")),
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

static cl::opt<bool> ProcessClassMethods(
    "process-class-methods",
    cl::desc("Process Class Methods"),
    cl::cat(MyToolCategory),
    cl::ValueDisallowed);
    
static cl::opt<bool> IgnoreExtraneousFiles(
    "ignore-extraneous-files",
    cl::desc("Ignore extraneous files while validating"),
    cl::cat(MyToolCategory),
    cl::ValueDisallowed);

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

bool IsVerbose() {
    return ToolDiagnostic == ToolDiagnosticVerbose || ToolDiagnostic == ToolDiagnosticVeryVerbose;
}

/**************************************************************************************************/

using optional_path_t = std::optional<std::filesystem::path>;

/**************************************************************************************************/

optional_path_t find_hyde_config(std::filesystem::path src_file) {
    if (!exists(src_file)) {
        return std::nullopt;
    }

    if (src_file.is_relative()) {
        const std::filesystem::path pwd_k = std::filesystem::current_path();

        src_file = std::filesystem::canonical(pwd_k / src_file);
    }

    if (!is_directory(src_file)) {
        src_file = src_file.parent_path();
    }

    const auto hyde_config_check = [](std::filesystem::path path) -> optional_path_t {
        if (std::filesystem::exists(path)) {
            return std::move(path);
        }

        return std::nullopt;
    };

    while (true) {
        if (!exists(src_file)) return std::nullopt;
        if (auto path = hyde_config_check(src_file / ".hyde-config")) {
            return path;
        }
        if (auto path = hyde_config_check(src_file / "_hyde-config")) {
            return path;
        }
        auto new_parent = src_file.parent_path();
        if (src_file == new_parent) return std::nullopt;
        src_file = std::move(new_parent);
    }
}

/**************************************************************************************************/

std::pair<std::filesystem::path, hyde::json> load_hyde_config(
    std::filesystem::path src_file) try {
    optional_path_t hyde_config_path(find_hyde_config(src_file));

    if (IsVerbose()) {
        if (hyde_config_path) {
            std::cout << "INFO: hyde-config file: " << hyde_config_path->string() << '\n';
        } else {
            std::cout << "INFO: hyde-config file: not found\n";
        }
    }

    return hyde_config_path ?
               std::make_pair(hyde_config_path->parent_path(),
                              hyde::json::parse(std::ifstream(*hyde_config_path))) :
               std::make_pair(std::filesystem::path(), hyde::json());
} catch (...) {
    throw std::runtime_error("failed to parse the hyde-config file");
}

/**************************************************************************************************/

struct command_line_args {
    std::vector<std::string> _hyde;
    std::vector<std::string> _clang;
};

command_line_args integrate_hyde_config(int argc, const char** argv) {
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
    std::filesystem::path config_dir;
    hyde::json config;
    std::tie(config_dir, config) =
        load_hyde_config(cli_hyde_flags.empty() ? "" : cli_hyde_flags.back());

    hyde_flags.push_back(argv[0]);

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

    if (config.count("hyde-tested-by")) {
        const std::string& tested_by = config["hyde-tested-by"];
        hyde_flags.emplace_back("-hyde-tested-by=" + tested_by);
    }

    hyde_flags.insert(hyde_flags.end(), cli_hyde_flags.begin(), cli_hyde_flags.end());
    clang_flags.insert(clang_flags.end(), cli_clang_flags.begin(), cli_clang_flags.end());

    hyde_flags.push_back("--");

    command_line_args result;

    result._hyde = std::move(hyde_flags);
    result._clang = std::move(clang_flags);

    return result;
}

/**************************************************************************************************/

namespace {

/**************************************************************************************************/

CommonOptionsParser MakeOptionsParser(int argc, const char** argv) {
    auto MaybeOptionsParser = CommonOptionsParser::create(argc, argv, MyToolCategory);
    if (!MaybeOptionsParser) {
        throw MaybeOptionsParser.takeError();
    }
    return std::move(*MaybeOptionsParser);
}

/**************************************************************************************************/
// Hyde may accumulate many "fixups" throughout its lifetime. The first of these so far is to move
// the hyde fields under a `hyde` subfield in the YAML, allowing for other tools' fields to coexist
// under other values in the same have file.
bool fixup_have_file_subfield(const std::filesystem::path& path) {
    // Passing `true` is what's actually causing the fixup.
    const auto parsed = hyde::parse_documentation(path, true);
    const auto failure = parsed._error || hyde::write_documentation(parsed, path);

    if (failure) {
        std::cerr << "Failed to fixup " << path << '\n';
    }

    return failure;
}

/**************************************************************************************************/

} // namespace

/**************************************************************************************************/

std::vector<std::string> source_paths(int argc, const char** argv) {
    return MakeOptionsParser(argc, argv).getSourcePathList();
}

/**************************************************************************************************/

int main(int argc, const char** argv) try {
    auto sources = source_paths(argc, argv);
    command_line_args args = integrate_hyde_config(argc, argv);
    int new_argc = static_cast<int>(args._hyde.size());
    std::vector<const char*> new_argv(args._hyde.size(), nullptr);

    std::transform(args._hyde.begin(), args._hyde.end(), new_argv.begin(),
                   [](const auto& arg) { return arg.c_str(); });

    CommonOptionsParser OptionsParser(MakeOptionsParser(new_argc, &new_argv[0]));

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
        std::cout << "INFO:   (hyde)\n";
        for (const auto& arg : args._hyde) {
            std::cout << "INFO:     " << arg << '\n';
        }
        std::cout << "INFO:   (clang)\n";
        for (const auto& arg : args._clang) {
            std::cout << "INFO:     " << arg << '\n';
        }
        std::cout << "INFO: Working directory: " << std::filesystem::current_path().string()
                  << '\n';
    }

    auto sourcePaths = make_absolute(OptionsParser.getSourcePathList());
    // Remove duplicates (CommonOptionsParser is duplicating every single entry)
    std::unordered_set<std::string> s;
    for (std::string i : sourcePaths) {
        s.insert(i);
    }
    sourcePaths.assign(s.begin(), s.end());

    if (ToolMode == ToolModeFixupSubfield) {
        bool failure{false};

        for (const auto& path : sourcePaths) {
            failure |= fixup_have_file_subfield(path);
        }

        // In this mode, once the documentation file has been fixed up, we're done.
        return failure ? EXIT_FAILURE : EXIT_SUCCESS;
    }

    MatchFinder Finder;
    hyde::processing_options options{sourcePaths, ToolAccessFilter, NamespaceBlacklist, ProcessClassMethods};

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

    // start by appending the command line clang args.
    for (auto& arg : args._clang) {
        arguments.emplace_back(std::move(arg));
    }

    if (ToolDiagnostic == ToolDiagnosticVeryVerbose) {
        arguments.emplace_back("-v");
    }

#if HYDE_PLATFORM(APPLE)
    //
    // Specify the isysroot directory to the driver
    // 
    // in some versions of osx they have stopped using `/usr/include`; Apple seems to rely
    // on the isysroot parameter to accomplish this task in the general case, so we add it here.
    std::filesystem::path include_dir{"/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/"};

    if (AutoSysrootDirectory) {
        if (IsVerbose()) {
            std::cout << "INFO: Sysroot autodetected\n";
        }
        include_dir = hyde::autodetect_sysroot_directory();
    }

    if (std::filesystem::exists(include_dir)) {
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
        std::vector<std::filesystem::path> includes = hyde::autodetect_toolchain_paths();
        if (IsVerbose()) {
            std::cout << "INFO: Toolchain paths autodetected:\n";
        }
        for (const auto& arg : includes) {
            if (IsVerbose()) {
                std::cout << "INFO:     " << arg.string() << '\n';
            }

            arguments.emplace_back("-I" + arg.string());
        }
    }

    //
    // Specify the resource directory to the driver
    // 
    // this may not work on windows, need to investigate using strings
    std::filesystem::path resource_dir;

    if (AutoResourceDirectory) {
        if (IsVerbose()) {
            std::cout << "INFO: Resource directory autodetected\n";
        }

        resource_dir = hyde::autodetect_resource_directory();
    } else if (!ArgumentResourceDir.empty()) {
        resource_dir = std::filesystem::path(ArgumentResourceDir.getValue());
    }

    if (std::filesystem::exists(resource_dir)) {
        if (IsVerbose()) {
            std::cout << "INFO: Using resource-dir: " << resource_dir.string() << std::endl;
        }
        arguments.emplace_back("-resource-dir=" + resource_dir.string());
    }

    // Specify the hyde preprocessor macro
    arguments.emplace_back("-DADOBE_TOOL_HYDE=1");

    // Have the driver parse comments. See:
    // https://clang.llvm.org/docs/UsersManual.html#comment-parsing-options
    // This isn't strictly necessary, as Doxygen comments will be detected
    // and parsed regardless. Better to be thorough, though.
    arguments.emplace_back("-fparse-all-comments");

    // Enables some checks built in to the clang driver to ensure comment
    // documentation matches whatever it is documenting. We also make it
    // an error because the documentation should be accurate when generated.
    arguments.emplace_back("-Werror=documentation");
    arguments.emplace_back("-Werror=documentation-deprecated-sync");
    arguments.emplace_back("-Werror=documentation-html");
    arguments.emplace_back("-Werror=documentation-pedantic");

    // Add hyde-specific commands to the Clang Doxygen parser. For hyde, we'll require the first
    // word to be the hyde field (e.g., `@hyde-owner fosterbrereton`.) Because the Doxygen parser
    // doesn't consider `-` or `_` as part of the command token, the first word will be
    // `-owner` in this case, which gives us something parseable, and it reads
    // reasonably in the code as well.
    arguments.emplace_back("-fcomment-block-commands=hyde");

    //
    // Spin up the tool and run it.
    //

    ClangTool Tool(OptionsParser.getCompilations(), sourcePaths);

    Tool.appendArgumentsAdjuster(OptionsParser.getArgumentsAdjuster());

    Tool.appendArgumentsAdjuster(
        getInsertArgumentAdjuster(arguments, clang::tooling::ArgumentInsertPosition::END));

    if (Tool.run(newFrontendActionFactory(&Finder).get()))
        throw std::runtime_error("compilation failed.");

    //
    // Take the results of the tool and process them.
    //

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

        std::filesystem::path src_root(YamlSrcDir.getValue());
        std::filesystem::path dst_root(YamlDstDir.getValue());

        hyde::emit_options emit_options;
        emit_options._tested_by = TestedBy;
        emit_options._ignore_extraneous_files = IgnoreExtraneousFiles;

        auto out_emitted = hyde::json::object();
        output_yaml(std::move(result), std::move(src_root), std::move(dst_root), out_emitted,
                    ToolMode == ToolModeYAMLValidate ? hyde::yaml_mode::validate :
                                                       hyde::yaml_mode::update, std::move(emit_options));
        
        if (EmitJson) {
            std::cout << out_emitted << '\n';
        }
    }
} catch (const std::exception& error) {
    std::cerr << "Fatal error: " << error.what() << '\n';
    return EXIT_FAILURE;
} catch (const Error& error) {
    std::string description;
    raw_string_ostream stream(description);
    stream << error;
    std::cerr << "Fatal error: " << stream.str() << '\n';
    return EXIT_FAILURE;
} catch (...) {
    std::cerr << "Fatal error: unknown\n";
    return EXIT_FAILURE;
}

/**************************************************************************************************/
