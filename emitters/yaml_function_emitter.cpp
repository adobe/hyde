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
#include "yaml_function_emitter.hpp"

// stdc++
#include <iostream>

/**************************************************************************************************/

namespace hyde {

/**************************************************************************************************/

bool yaml_function_emitter::do_merge(const std::string& filepath,
                                     const json& have,
                                     const json& expected,
                                     json& out_merged) {
    bool failure{false};

    failure |= check_scalar(filepath, have, expected, "", out_merged, "defined_in_file");
    failure |= check_scalar_array(filepath, have, expected, "", out_merged, "namespace");
    failure |= check_scalar(filepath, have, expected, "", out_merged, "is_ctor");
    failure |= check_scalar(filepath, have, expected, "", out_merged, "is_dtor");
    failure |= check_map(
        filepath, have, expected, "", out_merged, "overloads",
        [this](const std::string& filepath, const json& have, const json& expected,
               const std::string& nodepath, json& out_merged) {
            bool failure{false};

            failure |= check_editable_scalar(filepath, have, expected, nodepath, out_merged,
                                             "description");
            failure |= check_scalar(filepath, have, expected, nodepath, out_merged,
                                    "signature_with_names");

            if (!has_json_flag(expected, "is_ctor") && !has_json_flag(expected, "is_dtor")) {
                failure |=
                    check_editable_scalar(filepath, have, expected, nodepath, out_merged, "return");
            }

            if (_options._tested_by != hyde::attribute_category::disabled) {
                failure |= check_editable_scalar_array(filepath, have, expected, nodepath,
                                                       out_merged, "tested_by");
            }

            failure |=
                check_scalar_array(filepath, have, expected, nodepath, out_merged, "annotation");

            failure |= check_object_array(
                filepath, have, expected, nodepath, out_merged, "arguments", "name",
                [this](const std::string& filepath, const json& have, const json& expected,
                       const std::string& nodepath, json& out_merged) {
                    bool failure{false};

                    failure |= check_scalar(filepath, have, expected, nodepath, out_merged, "name");
                    failure |= check_scalar(filepath, have, expected, nodepath, out_merged, "type");
                    failure |= check_editable_scalar(filepath, have, expected, nodepath, out_merged,
                                                     "description");
                    failure |=
                        check_scalar(filepath, have, expected, nodepath, out_merged, "unnamed");

                    check_inline_comments(expected, out_merged);

                    return failure;
                });

            check_inline_comments(expected, out_merged);

            return failure;
        });

    // REVISIT (fosterbrereton) : Roll-up the owners and briefs/descriptions to see if we can derive
    // a set of inline values here from inline values used in the function definition(s).

    check_inline_comments(expected, out_merged);

    return failure;
}

/**************************************************************************************************/

bool yaml_function_emitter::emit(const json& jsn, json& out_emitted, const json& inherited) {
    std::filesystem::path dst;
    std::string name;
    std::string filename;
    std::string defined_path;
    json overloads = json::object();
    bool first{true};
    bool is_ctor{false};
    bool is_dtor{false};
    bool all_implicit{true};
    std::size_t count(jsn.size());
    std::size_t inline_description_count{0};
    json last_inline_description;
    std::size_t inline_brief_count{0};
    json last_inline_brief;
    std::vector<std::string> overload_owners;

    for (const auto& overload : jsn) {
        if (first) {
            dst = dst_path(overload);
            // always the unqualified name, as free functions may be defined
            // over different namespaces.
            name = overload["short_name"];
            // prefix to keep free-function from colliding with class member (e.g., `swap`)
            filename = (_as_methods ? "m_" : "f_") + filename_filter(name);
            defined_path = defined_in_file(overload["defined_in_file"], _src_root);
            is_ctor = has_json_flag(overload, "is_ctor");
            is_dtor = has_json_flag(overload, "is_dtor");
            first = false;
        }

        const std::string& key = static_cast<const std::string&>(overload["signature"]);
        auto& destination = overloads[key];

        // If there are any in-source (a.k.a. Doxygen) comments, insert them into
        // the node *first* so we can use them to decide if subsequent Hyde fields
        // can be deferred.
        insert_doxygen(overload, destination);

        // The intent of these checks is to roll up brief/description details that were
        // entered inline to the top-level file that documents this function and all of its
        // overrides. Note that a given override does _not_ require a `brief` value, but
        // _may_ require a `description` value if there is more than one override (otherwise
        // the description is optional, falling back to the top-level `brief` for the functions
        // documentation). I foresee *a lot* of conflation between `brief` and `description`
        // as developers document their code, so we'll have to track both of these values as if
        // they are the same to make it as easy as possible to bubble up information.
        if (destination.count("inline") && destination["inline"].count("brief")) {
            ++inline_brief_count;
            last_inline_brief = destination["inline"]["brief"];
        }
        if (destination.count("inline") && destination["inline"].count("description")) {
            ++inline_description_count;
            last_inline_description = destination["inline"]["description"];
        }
        if (destination.count("inline") && destination["inline"].count("owner")) {
            const auto& owners = destination["inline"]["owner"];
            if (owners.is_string()) {
                overload_owners.push_back(owners.get<std::string>());
            } else if (owners.is_array()) {
                for (const auto& owner : owners) {
                    overload_owners.push_back(owner.get<std::string>());
                }
            }
        }

        destination["signature_with_names"] = overload["signature_with_names"];
        // description is now optional when there is a singular variant, or when the overload
        // is implicit (e.g., compiler-implemented.)
        const bool has_associated_inline = has_inline_field(destination, "description");
        const bool is_implicit_overload = has_json_flag(overload, "implicit");
        const bool is_optional = count <= 1 || is_implicit_overload;
        destination["description"] = has_associated_inline ? tag_value_inlined_k :
                                     is_optional           ? tag_value_optional_k :
                                                             tag_value_missing_k;

        if (!is_ctor && !is_dtor) {
            destination["return"] = tag_value_optional_k;
        }

        if (_options._tested_by != hyde::attribute_category::disabled) {
            destination["tested_by"] = is_implicit_overload ? tag_value_optional_k : hyde::get_tag(_options._tested_by);
        }
        insert_annotations(overload, destination);

        all_implicit &= is_implicit_overload;

        if (!overload["arguments"].empty()) {
            std::size_t j{0};
            auto& args = destination["arguments"];
            for (const auto& arg : overload["arguments"]) {
                auto& cur_arg = args[j];
                const std::string& name = arg["name"];
                const bool unnamed = name.empty();
                cur_arg["name"] = unnamed ? "unnamed-" + std::to_string(j) : name;
                cur_arg["type"] = arg["type"];
                cur_arg["description"] = tag_value_optional_k;
                if (unnamed) {
                    cur_arg["unnamed"] = true;
                }
                ++j;
            }
        }
    }

    json node = base_emitter_node(_as_methods ? "method" : "function", name,
                                  _as_methods ? "method" : "function",
                                  all_implicit || has_json_flag(jsn, "implicit"));

    insert_inherited(inherited, node["hyde"]);

    // If the function being emitted is either the ctor or dtor of a class,
    // the high-level `brief` is optional, as their default implementations
    // should require no additional commenatary beyond that which is provided
    // on an overload-by-overload basis.
    if (is_ctor || is_dtor) {
        node["hyde"]["brief"] = tag_value_optional_k;
    }

    // Here we roll-up the brief(s) and description(s) from the function overloads.
    // The complication here is that `description` may be required for the overloads,
    // but `brief` is not. However at the top level, `brief` _is_ required, and
    // `description` is not. So if there is one brief _or_ description, use that
    // as the inline brief for the top-level docs. (If there is one of each, brief
    // wins.) Beyond that, if there are multiple briefs and multiple descriptions,
    // we'll put some pat statement into the brief indicating as much. Finally, if
    // at least one overload has an inline `brief`, then the top-level `brief` is
    // marked inlined.
    if (inline_brief_count == 1) {
        node["hyde"]["inline"]["brief"] = last_inline_brief;
        node["hyde"]["brief"] = tag_value_inlined_k;
    } else if (inline_description_count == 1) {
        node["hyde"]["inline"]["brief"] = last_inline_description;
        node["hyde"]["brief"] = tag_value_inlined_k;
    } else if (inline_brief_count > 1 || inline_description_count > 1) {
        node["hyde"]["inline"]["brief"] = "_multiple descriptions_";
        node["hyde"]["brief"] = tag_value_inlined_k;
    }

    // Round up any overload owners into an inline top-level owner field,
    // and then set the hyde owner field to inlined.
    if (!overload_owners.empty()) {
        std::sort(overload_owners.begin(), overload_owners.end());
        overload_owners.erase(std::unique(overload_owners.begin(), overload_owners.end()),
                              overload_owners.end());
        json::array_t owners;
        std::copy(overload_owners.begin(), overload_owners.end(), std::back_inserter(owners));
        node["hyde"]["inline"]["owner"] = std::move(owners);
        node["hyde"]["owner"] = tag_value_inlined_k;
    } else if (node["hyde"].count("inline") && node["hyde"]["inline"].count("owner")) {
        node["hyde"]["owner"] = tag_value_inlined_k;
    }

    // All overloads will have the same namespace
    if (!_as_methods && jsn.size() > 0) {
        for (const auto& ns : jsn.front()["namespaces"])
            node["hyde"]["namespace"].push_back(static_cast<const std::string&>(ns));
    }

    node["hyde"]["defined_in_file"] = defined_path;
    node["hyde"]["overloads"] = std::move(overloads);
    if (is_ctor) node["hyde"]["is_ctor"] = true;
    if (is_dtor) node["hyde"]["is_dtor"] = true;

    return reconcile(std::move(node), _dst_root, dst / (filename + ".md"), out_emitted);
}

/**************************************************************************************************/

} // namespace hyde

/**************************************************************************************************/
