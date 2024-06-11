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
#include "autodetect.hpp"

// stdc++
#include <algorithm>
#include <array>
#include <cstdio>
#include <fstream>
#include <iterator>
#include <memory>
#include <random>
#include <string>
#include <utility>
#include <vector>

/**************************************************************************************************/

namespace {

/**************************************************************************************************/

std::vector<std::string> split(const char* p, std::size_t n) {
    auto end = p + n;
    std::vector<std::string> result;

    while (p != end) {
        while (p != end && *p == '\n')
            ++p; // eat newlines
        auto begin = p;
        while (p != end && *p != '\n')
            ++p; // eat non-newlines
        result.emplace_back(begin, p);
    }

    return result;
}

/**************************************************************************************************/

std::vector<std::string> file_slurp(std::filesystem::path p) {
    std::ifstream s(p);
    s.seekg(0, std::ios::end);
    std::size_t size = s.tellg();
    auto buffer{std::make_unique<char[]>(size)};
    s.seekg(0);
    s.read(&buffer[0], size);
    return split(buffer.get(), size);
}

/**************************************************************************************************/

std::string trim_front(std::string s) {
    std::size_t n(0);
    std::size_t end(s.size());

    while (n != end && (std::isspace(s[n]) || s[n] == '\n'))
        ++n;

    s.erase(0, n);

    return s;
}

/**************************************************************************************************/

std::string trim_back(std::string s) {
    std::size_t start(s.size());

    while (start != 0 && (std::isspace(s[start - 1]) || s[start - 1] == '\n'))
        start--;

    s.erase(start, std::string::npos);

    return s;
}

/**************************************************************************************************/

std::string chomp(std::string src) { return trim_back(trim_front(std::move(src))); }

/**************************************************************************************************/

std::string exec(const char* cmd) {
    struct pclose_t {
        void operator()(std::FILE* p) const { (void)pclose(p); }
    };
    std::unique_ptr<std::FILE, pclose_t> pipe{popen(cmd, "r")};

    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }

    std::array<char, 128> buffer;
    std::string result;
    while (fgets(buffer.data(), buffer.size(), pipe.get())) {
        result += buffer.data();
    }

    return chomp(std::move(result));
}

/**************************************************************************************************/

std::vector<std::filesystem::path> autodetect_include_paths() {
    // Add a random value here so two concurrent instances of hyde don't collide.
    auto v = std::to_string(std::mt19937(std::random_device()())());
    auto temp_dir = std::filesystem::temp_directory_path();
    auto temp_out = (temp_dir / ("hyde_" + v + ".tmp")).string();
    auto temp_a_out = (temp_dir / ("deleteme_" + v)).string();
    auto command =
        "echo \"int main() { }\" | clang++ -x c++ -v -o " + temp_a_out + " - 2> " + temp_out;

    auto command_result = std::system(command.c_str());
    (void)command_result; // TODO: handle me

    std::vector<std::string> lines(file_slurp(temp_out));
    static const std::string begin_string("#include <...> search starts here:");
    static const std::string end_string("End of search list.");
    auto paths_begin = std::find(begin(lines), end(lines), begin_string);
    auto paths_end = std::find(begin(lines), end(lines), end_string);
    std::vector<std::filesystem::path> result;

    if (paths_begin != end(lines) && paths_end != end(lines)) {
        lines.erase(paths_end, end(lines));
        lines.erase(begin(lines), std::next(paths_begin));

        // lines.erase(std::remove_if(begin(lines), end(lines), [](auto& s){
        //     return s.find(".sdk/") != std::string::npos;
        // }), end(lines));

        // Some of the paths contain cruft at the end. Filter those out, too.
        std::transform(begin(lines), end(lines), std::back_inserter(result), [](auto s) {
            static const std::string needle_k{" (framework directory)"};
            auto needle_pos = s.find(needle_k);
            if (needle_pos != std::string::npos) {
                s.erase(needle_pos);
            }
            return std::filesystem::path(chomp(std::move(s)));
        });
    }

    return result;
}

/**************************************************************************************************/

} // namespace

/**************************************************************************************************/

namespace hyde {

/**************************************************************************************************/

std::vector<std::filesystem::path> autodetect_toolchain_paths() {
    return autodetect_include_paths();
}

/**************************************************************************************************/

std::filesystem::path autodetect_resource_directory() {
    return std::filesystem::path{exec("clang++ -print-resource-dir")};
}

/**************************************************************************************************/
#if HYDE_PLATFORM(APPLE)
std::filesystem::path autodetect_sysroot_directory() {
    return std::filesystem::path{exec("xcode-select -p")} /
           "Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk";
}
#endif // HYDE_PLATFORM(APPLE)
/**************************************************************************************************/

} // namespace hyde

/**************************************************************************************************/
