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

// application
#include "emitters/yaml_base_emitter_fwd.hpp"
#include "json.hpp"
#include "output_yaml.hpp"

/**************************************************************************************************/

namespace hyde {

/**************************************************************************************************/

struct file_checker {
    bool exists(std::filesystem::path p) {
        _files.emplace_back(std::move(p));
        _sorted = false;
        return std::filesystem::exists(_files.back());
    }

    bool checked(const std::filesystem::path& p) {
        if (!_sorted) {
            std::sort(_files.begin(), _files.end());
            _files.erase(std::unique(_files.begin(), _files.end()), _files.end());
            _sorted = true;
        }

        auto found = std::lower_bound(_files.begin(), _files.end(), p);
        return found != _files.end() && *found == p;
    }

private:
    std::vector<std::filesystem::path> _files;
    bool _sorted{false};
};

/**************************************************************************************************/

inline bool has_json_flag(const json& j, const char* k) {
    return j.count(k) && j.at(k).get<bool>();
}

/**************************************************************************************************/

struct yaml_base_emitter {
public:
    yaml_base_emitter(std::filesystem::path src_root,
                      std::filesystem::path dst_root,
                      yaml_mode mode,
                      emit_options options,
                      bool editable_title = false)
        : _src_root(std::move(src_root)), _dst_root(std::move(dst_root)), _mode(mode),
          _options(std::move(options)), _editable_title{editable_title} {}

    virtual bool emit(const json& j, json& out_emitted) = 0;

protected:
    json base_emitter_node(std::string layout, std::string title, std::string tag, bool implicit);

    bool reconcile(json node,
                   std::filesystem::path root_path,
                   std::filesystem::path path,
                   json& out_reconciled);

    std::string defined_in_file(const std::string& src_path, const std::filesystem::path& src_root);

    std::filesystem::path subcomponent(const std::filesystem::path& src_path,
                                       const std::filesystem::path& src_root);

    void insert_annotations(const json& j, json& node); // make out arg?

    void insert_doxygen(const json& j, json& node); // make out arg?

    std::string format_template_parameters(const json& json, bool with_types);

    std::string filename_filter(std::string f);
    std::string filename_truncate(std::string s);

    void insert_typedefs(const json& j, json& node);

    void copy_inline_comments(const json& expected, json& out_merged);

    bool check_typedefs(const std::string& filepath,
                        const json& have_node,
                        const json& expected_node,
                        const std::string& nodepath,
                        json& merged_node);

    template <typename... Args>
    std::filesystem::path dst_path(const json& j, Args&&... args);

    bool check_removed(const std::string& filepath,
                       const json& have_node,
                       const std::string& nodepath,
                       const std::string& key);

    bool check_scalar(const std::string& filepath,
                      const json& have_node,
                      const json& expected_node,
                      const std::string& nodepath,
                      json& out_merged,
                      const std::string& key);

    bool check_editable_scalar(const std::string& filepath,
                               const json& have_node,
                               const json& expected_node,
                               const std::string& nodepath,
                               json& out_merged,
                               const std::string& key);

    bool check_editable_scalar_array(const std::string& filepath,
                                     const json& have_node,
                                     const json& expected_node,
                                     const std::string& nodepath,
                                     json& merged_node,
                                     const std::string& key);

    bool check_scalar_array(const std::string& filepath,
                            const json& have_node,
                            const json& expected_node,
                            const std::string& nodepath,
                            json& merged_node,
                            const std::string& key);

    using check_proc = std::function<bool(const std::string& filepath,
                                          const json& have,
                                          const json& expected,
                                          const std::string& nodepath,
                                          json& out_merged)>;

    bool check_object_array(const std::string& filepath,
                            const json& have_node,
                            const json& expected_node,
                            const std::string& nodepath,
                            json& merged_node,
                            const std::string& key,
                            const std::string& object_key,
                            const check_proc& proc);

    bool check_map(const std::string& filepath,
                   const json& have,
                   const json& expected,
                   const std::string& nodepath,
                   json& out_merged,
                   const std::string& key,
                   const check_proc& proc);

private:
    template <typename Arg, typename... Args>
    std::filesystem::path dst_path_append(std::filesystem::path p, Arg&& arg, Args&&... args);

    template <typename Arg>
    std::filesystem::path dst_path_append(std::filesystem::path, Arg&& arg);

    std::filesystem::path dst_path_append(std::filesystem::path p) { return p; }

    bool create_directory_stub(std::filesystem::path p);
    bool create_path_directories(std::filesystem::path p);

    std::filesystem::path directory_mangle(std::filesystem::path p);

    void check_notify(const std::string& filepath,
                      const std::string& nodepath,
                      const std::string& key,
                      const std::string& validate_message,
                      const std::string& update_message);

    virtual std::pair<bool, json> merge(const std::string& filepath,
                                        const json& have,
                                        const json& expected);

    virtual bool do_merge(const std::string& filepath,
                          const json& have,
                          const json& expected,
                          json& out_merged) = 0;

protected: // make private?
    const std::filesystem::path _src_root;
    const std::filesystem::path _dst_root;
    const yaml_mode _mode;
    const emit_options _options;
    const bool _editable_title{false};

    static file_checker checker_s;
};

/**************************************************************************************************/

template <typename... Args>
std::filesystem::path yaml_base_emitter::dst_path(const json& j, Args&&... args) {
    std::filesystem::path result(_dst_root);

    if (j.count("defined_in_file")) {
        const std::string& defined_in_file = j["defined_in_file"];
        result /= directory_mangle(subcomponent(defined_in_file, _src_root));
    }

    if (j.count("parents")) {
        for (const auto& dir : j["parents"]) {
            std::string dir_str{dir};
            result /= directory_mangle(std::move(dir_str));
        }
    }

    return dst_path_append(std::move(result), std::forward<Args>(args)...);
}

/**************************************************************************************************/

template <typename Arg, typename... Args>
std::filesystem::path yaml_base_emitter::dst_path_append(std::filesystem::path p,
                                                         Arg&& arg,
                                                         Args&&... args) {
    return dst_path_append(dst_path_append(std::move(p), arg), std::forward<Args>(args)...);
}

/**************************************************************************************************/

template <typename Arg>
std::filesystem::path yaml_base_emitter::dst_path_append(std::filesystem::path p, Arg&& arg) {
    p /= directory_mangle(arg);
    return p;
}

/**************************************************************************************************/

} // namespace hyde

/**************************************************************************************************/
