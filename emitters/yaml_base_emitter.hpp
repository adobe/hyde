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

// boost
#include "boost/filesystem.hpp"

// application
#include "emitters/yaml_base_emitter_fwd.hpp"
#include "json.hpp"
#include "output_yaml.hpp"

/**************************************************************************************************/

namespace hyde {

/**************************************************************************************************/

struct file_checker {
    bool exists(boost::filesystem::path p) {
        _files.emplace_back(std::move(p));
        _sorted = false;
        return boost::filesystem::exists(_files.back());
    }

    bool checked(const boost::filesystem::path& p) {
        if (!_sorted) {
            std::sort(_files.begin(), _files.end());
            _files.erase(std::unique(_files.begin(), _files.end()), _files.end());
            _sorted = true;
        }

        auto found = std::lower_bound(_files.begin(), _files.end(), p);
        return found != _files.end() && *found == p;
    }

private:
    std::vector<boost::filesystem::path> _files;
    bool _sorted{false};
};

/**************************************************************************************************/

struct yaml_base_emitter {
public:
    yaml_base_emitter(boost::filesystem::path src_root,
                      boost::filesystem::path dst_root,
                      yaml_mode mode,
                      emit_options options)
        : _src_root(std::move(src_root)), _dst_root(std::move(dst_root)), _mode(mode), _options(std::move(options)) {}

    virtual bool emit(const json& j, json& out_emitted) = 0;

protected:
    json base_emitter_node(std::string layout, std::string title, std::string tag);

    bool reconcile(json node, boost::filesystem::path root_path, boost::filesystem::path path, json& out_reconciled);

    std::string defined_in_file(const std::string& src_path,
                                const boost::filesystem::path& src_root);

    boost::filesystem::path subcomponent(const boost::filesystem::path& src_path,
                                         const boost::filesystem::path& src_root);

    void maybe_annotate(const json& j, json& node); // make out arg?

    std::string format_template_parameters(const json& json, bool with_types);

    std::string filename_filter(std::string f);
    std::string filename_truncate(std::string s);

    void insert_typedefs(const json& j, json& node);

    template <typename... Args>
    boost::filesystem::path dst_path(const json& j, Args&&... args);

    bool check_scalar(const std::string& filepath,
                      const json& have,
                      const json& expected,
                      const std::string& nodepath,
                      json& out_merged,
                      const std::string& key);

    bool check_ungenerated_scalar_array(const std::string& filepath,
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
    boost::filesystem::path dst_path_append(boost::filesystem::path p, Arg&& arg, Args&&... args);

    template <typename Arg>
    boost::filesystem::path dst_path_append(boost::filesystem::path, Arg&& arg);

    boost::filesystem::path dst_path_append(boost::filesystem::path p) { return p; }

    bool create_directory_stub(boost::filesystem::path p);
    bool create_path_directories(boost::filesystem::path p);

    boost::filesystem::path directory_mangle(boost::filesystem::path p);

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
    const boost::filesystem::path _src_root;
    const boost::filesystem::path _dst_root;
    const yaml_mode _mode;
    const emit_options _options;

    static file_checker checker_s;
};

/**************************************************************************************************/

template <typename... Args>
boost::filesystem::path yaml_base_emitter::dst_path(const json& j, Args&&... args) {
    boost::filesystem::path result(_dst_root);

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
boost::filesystem::path yaml_base_emitter::dst_path_append(boost::filesystem::path p, Arg&& arg, Args&&... args) {
    return dst_path_append(dst_path_append(std::move(p), arg), std::forward<Args>(args)...);
}

/**************************************************************************************************/

template <typename Arg>
boost::filesystem::path yaml_base_emitter::dst_path_append(boost::filesystem::path p, Arg&& arg) {
    p /= directory_mangle(arg);
    return p;
}

/**************************************************************************************************/

} // namespace hyde

/**************************************************************************************************/
