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
#include "yaml_base_emitter.hpp"

// stdc++
#include <iostream>

// boost
#include "boost/filesystem.hpp"

// yaml-cpp
#include "yaml-cpp/yaml.h"

// application
#include "emitters/yaml_base_emitter_fwd.hpp"
#include "json.hpp"

/**************************************************************************************************/

namespace {

/**************************************************************************************************/

YAML::Node update_cleanup(const YAML::Node& node) {
    YAML::Node result;

    if (node.IsScalar() && node.Scalar() != hyde::tag_value_deprecated_k) {
        result = node;
    } else if (node.IsSequence()) {
        const auto count = node.size();
        for (std::size_t i{0}; i < count; ++i) {
            YAML::Node subnode = update_cleanup(node[i]);
            if (!subnode.IsNull()) {
                result.push_back(std::move(subnode));
            }
        }
    } else if (node.IsMap()) {
        for (const auto& pair : node) {
            const auto& key = pair.first.Scalar();
            YAML::Node subnode = update_cleanup(node[key]);
            if (!subnode.IsNull()) {
                result[key] = std::move(subnode);
            }
        }
    }

    return result;
}

/**************************************************************************************************/

hyde::json yaml_to_json(const YAML::Node& yaml) {
    switch (yaml.Type()) {
        case YAML::NodeType::Null: {
            return hyde::json();
        } break;
        case YAML::NodeType::Scalar: {
            return yaml.Scalar();
        } break;
        case YAML::NodeType::Sequence: {
            hyde::json result = hyde::json::array();
            for (std::size_t i{0}, count{yaml.size()}; i < count; ++i) {
                result.emplace_back(yaml_to_json(yaml[i]));
            }
            return result;
        } break;
        case YAML::NodeType::Map: {
            hyde::json result = hyde::json::object();
            for (auto iter{yaml.begin()}, last{yaml.end()}; iter != last; ++iter) {
                if (!iter->first.IsScalar()) throw std::runtime_error("key is not scalar?");
                result[iter->first.Scalar()] = yaml_to_json(iter->second);
            }
            return result;
        } break;
        case YAML::NodeType::Undefined: {
            throw std::runtime_error("YAML is not defined!");
        } break;
    }
}

/**************************************************************************************************/

YAML::Node json_to_yaml(const hyde::json& json) {
    switch (json.type()) {
        case hyde::json::value_t::null: {
            return YAML::Node();
        } break;
        case hyde::json::value_t::string: {
            return YAML::Node(static_cast<const std::string&>(json));
        } break;
        case hyde::json::value_t::array: {
            YAML::Node result;
            for (const auto& n : json) {
                result.push_back(json_to_yaml(n));
            }
            return result;
        } break;
        case hyde::json::value_t::object: {
            YAML::Node result;
            for (auto it = json.begin(); it != json.end(); ++it) {
                result[it.key()] = json_to_yaml(it.value());
            }
            return result;
        } break;
        case hyde::json::value_t::boolean: {
            return YAML::Node(json.get<bool>());
        } break;
        case hyde::json::value_t::number_integer: {
            return YAML::Node(json.get<int>());
        } break;
        case hyde::json::value_t::number_unsigned: {
            return YAML::Node(json.get<unsigned int>());
        } break;
        case hyde::json::value_t::number_float: {
            return YAML::Node(json.get<float>());
        } break;
        case hyde::json::value_t::discarded: {
            throw std::runtime_error("JSON value discarded!");
        } break;
    }
}

/**************************************************************************************************/

YAML::Node json_to_yaml_ordered(hyde::json j) {
    // YAML preserves the order of addition, while JSON orders lexicographically
    // by key value. We want the values common to all jekyll pages to appear
    // higher in the YAML than other keys, so we do a specific ordering here.

    YAML::Node result;

    const auto move_key = [&](const std::string& key) {
        if (!j.count(key)) return;
        result[key] = json_to_yaml(j[key]);
        j.erase(key);
    };

    // These are in some ROUGH order/grouping from generic to specific fields.

    move_key("layout");
    move_key("title");
    move_key("owner");
    move_key("brief");
    move_key("tags");
    move_key("library-type");
    move_key("defined_in_file");
    move_key("declaration");
    move_key("annotation");

    move_key("ctor");
    move_key("dtor");
    move_key("is_ctor");
    move_key("is_dtor");

    move_key("typedefs");
    move_key("fields");
    move_key("methods");
    move_key("overloads");

    // copy over the remainder of the keys.
    for (auto it = j.begin(); it != j.end(); ++it) {
        result[it.key()] = json_to_yaml(it.value());
    }

    return result;
}

/**************************************************************************************************/

static const std::string front_matter_delimiter_k("---\n");

/**************************************************************************************************/

} // namespace

/**************************************************************************************************/

namespace hyde {

/**************************************************************************************************/

file_checker yaml_base_emitter::checker_s; // REVISIT (fbrereto) : Global. Bad programmer. No donut.

/**************************************************************************************************/

json yaml_base_emitter::base_emitter_node(std::string layout, std::string title, std::string tag) {
    json node;

    node["layout"] = std::move(layout);
    node["title"] = std::move(title);
    node["owner"] = tag_value_missing_k;
    node["tags"].emplace_back(std::move(tag));
    node["brief"] = tag_value_missing_k;

    return node;
}

/**************************************************************************************************/

void yaml_base_emitter::insert_typedefs(const json& j, json& node) {
    if (j.count("typedefs")) {
        for (const auto& type_def : j["typedefs"]) {
            const std::string& key = type_def["name"];
            auto& type_node = node["typedefs"][key];
            type_node["definition"] = static_cast<const std::string&>(type_def["type"]);
            type_node["description"] = tag_value_missing_k;
            maybe_annotate(type_def, type_node);
        }
    }

    if (j.count("typealiases")) {
        for (const auto& type_def : j["typealiases"]) {
            const std::string& key = type_def["name"];
            auto& type_node = node["typedefs"][key];
            type_node["definition"] = static_cast<const std::string&>(type_def["type"]);
            type_node["description"] = tag_value_missing_k;
            maybe_annotate(type_def, type_node);
        }
    }
}

/**************************************************************************************************/

void yaml_base_emitter::check_notify(const std::string& filepath,
                                     const std::string& nodepath,
                                     const std::string& key,
                                     const std::string& validate_message,
                                     const std::string& update_message) {
    switch (_mode) {
        case yaml_mode::validate: {
            std::cerr << filepath << "@" << nodepath << "['" << key << "']: " << validate_message
                      << "\n";
        } break;
        case yaml_mode::update: {
            std::cout << filepath << "@" << nodepath << "['" << key << "']: " << update_message
                      << "\n";
        } break;
    }
}

/**************************************************************************************************/

bool yaml_base_emitter::check_scalar(const std::string& filepath,
                                     const json& have_node,
                                     const json& expected_node,
                                     const std::string& nodepath,
                                     json& merged_node,
                                     const std::string& key) {
    const auto notify = [&](const std::string& validate_message,
                            const std::string& update_message) {
        check_notify(filepath, nodepath, key, validate_message, update_message);
    };

    if (!expected_node.count(key)) {
        if (!have_node.count(key)) {
            // Removed key not present in have. Do nothing, no error.
            return false;
        } else {
            notify("value present for removed key", "value removed");
            auto merged_iter = merged_node.find(key);
            if (merged_iter != merged_node.end()) {
                merged_node.erase(merged_iter);
            }
            return true;
        }
    }

    const json& expected = expected_node[key];

    if (!expected.is_string()) {
        throw std::runtime_error("expected type mismatch?");
    }

    const std::string& expected_scalar(expected);
    json& result = merged_node[key];

    if (!have_node.count(key)) {
        if (expected_scalar == tag_value_deprecated_k) {
            // deprecated key not present in have. Do nothing, no error.
            return false;
        } else {
            notify("value missing", "value inserted");
            result = expected;
            return true;
        }
    }

    if (expected_scalar == tag_value_optional_k) {
        // Value is optional, and the docs have *something*, so we're good.
        return false;
    }

    const json& have = have_node[key];

    if (!have.is_string()) {
        notify("value not scalar; expected `" + expected_scalar + "`",
               "value not scalar; updated to `" + expected_scalar + "`");
        result = expected;
        return true;
    }

    const std::string& have_scalar(have);

    if (expected_scalar == tag_value_missing_k) {
        if (have_scalar != tag_value_missing_k)
            return false;

        if (_mode == yaml_mode::validate) {
            notify("value not documented", "");
        }

        return true;
    }

    if (expected_scalar == tag_value_deprecated_k) {
        notify("key is deprecated", "deprecated key removed");
        return true;
    }

    if (expected_scalar != have_scalar) {
        notify("value mismatch; have `" + have_scalar + "`, expected `" + expected_scalar + "`",
               "value updated from `" + have_scalar + "` to `" + expected_scalar + "`");
        result = expected;
        return true;
    }

    // both have and expected are both scalar and are the same value
    result = have;
    return false;
}

/**************************************************************************************************/

bool yaml_base_emitter::check_ungenerated_scalar_array(const std::string& filepath,
                                                       const json& have_node,
                                                       const json& expected_node,
                                                       const std::string& nodepath,
                                                       json& merged_node,
                                                       const std::string& key) {
    const auto notify = [&](const std::string& validate_message,
                            const std::string& update_message) {
        check_notify(filepath, nodepath, key, validate_message, update_message);
    };

    const auto notify_fail = [&](const std::string& message) {
        check_notify(filepath, nodepath, key, message, message);
    };

    if (!expected_node.count(key)) {
        if (!have_node.count(key)) {
            // Removed key not present in have. Do nothing, no error.
            return false;
        } else {
            notify("value present for removed key", "value removed");
            auto merged_iter = merged_node.find(key);
            if (merged_iter != merged_node.end()) {
                merged_node.erase(merged_iter);
            }
            return true;
        }
    }

    const json& expected = expected_node[key];

    if (!expected.is_string()) {
        throw std::runtime_error("expected type mismatch?");
    }

    const std::string& expected_scalar(expected);
    json& result = merged_node[key];

    if (!have_node.count(key)) {
        if (expected_scalar == tag_value_deprecated_k) {
            // deprecated key not present in have. Do nothing, no error.
            return false;
        } else {
            notify("value missing", "value inserted");
            result = expected;
            return true;
        }
    }

    const json& have = have_node[key];

    if (have.is_string()) {
        const std::string& have_scalar(have);

        if (expected_scalar == tag_value_missing_k && have_scalar == tag_value_missing_k) {
            if (_mode == yaml_mode::validate) {
                notify("value not documented", "");
            }

            return true;
        }

        if (expected_scalar == tag_value_deprecated_k) {
            notify("key is deprecated", "deprecated key removed");
            return true;
        }

        if (expected_scalar == tag_value_optional_k && have_scalar == tag_value_optional_k) {
            result = have;
            return false;
        }

        if (hyde::is_tag(have_scalar)) {
            notify("value is unexpected tag", 
                   "value updated from `" + have_scalar + "` to `" + expected_scalar + "`");
            result = expected; // Replace unexpected tag
            return true;
        }
    }

    if (!have.is_array()) {
        notify_fail("value not an array; expected an array of scalar values");
        result = have;
        return true;
    }

    result = have;

    // We have an array; make sure its elements are scalar
    std::size_t index{0};
    bool failure{false};
    for (const auto& have_element : have) {
        if (!have_element.is_string()) {
            failure = true;
            notify_fail("non-scalar array element at index " + std::to_string(index));
        } else if (hyde::is_tag(have_element)) {
            failure = true;
            notify_fail("invalid value at index " + std::to_string(index));
        }
    }

    return failure;
}

/**************************************************************************************************/

bool yaml_base_emitter::check_scalar_array(const std::string& filepath,
                                           const json& have_node,
                                           const json& expected_node,
                                           const std::string& nodepath,
                                           json& merged_node,
                                           const std::string& key) {
    const auto notify = [&](const std::string& validate_message,
                            const std::string& update_message) {
        check_notify(filepath, nodepath, key, validate_message, update_message);
    };

    const auto notify_fail = [&](const std::string& message) {
        check_notify(filepath, nodepath, key, message, message);
    };

    if (!expected_node.count(key)) {
        if (!have_node.count(key)) {
            // Removed key not present in have. Do nothing, no error.
            return false;
        } else {
            notify("value present for removed key", "value removed");
            return true;
        }
    }

    const json& expected = expected_node[key];

    if (!expected.is_array()) {
        notify_fail("expected type mismatch");
        throw std::runtime_error("Merge scalar array failure");
    }

    json& result = merged_node[key];

    if (!have_node.count(key) || !have_node[key].is_array()) {
        notify("value not an array", "non-array value replaced");
        result = expected;
        return true;
    }

    const json& have = have_node[key];

    // How does one merge an array of scalars? The solution is to use scalar value
    // as the "key", and treat the array like a dictionary. This does create the
    // possibility that multiple objects will resolve to the same key, and/or the
    // key value may not be a string, so we establish those requirements as function
    // preconditions.
    //
    // As for the actual merge, we have a `have` array, and an `expected` array.
    // The resulting merge will be built up in a resulting array, then moved
    // into `result.`
    //
    // First we build up a sorted vector of key/position pairs of the `have`
    // array. This will let us find elements in that array in log(N) time.
    //
    // Then we iterate the expected array from first to last. We pull out the
    // expected key and search for it in the have array. If it is missing, copy
    // the expected element into the result array, and move on. If it is found,
    // make sure they are in the same index in both arrays, otherwise it's a
    // validation error. The result array is then moved into the resulting json.
    //
    // If `have` has extraneous keys, they will be skipped during the iteration
    // of `expected` and subsequently dropped.
    //
    // The entire merge process should be O(N log N) time, bounded on the sort
    // of the have key vector. Thanks to Jared Wyles for the tip on how to solve
    // this one.

    std::vector<std::pair<std::string, std::size_t>> have_map;
    std::size_t count{0};
    bool failure{false};
    for (const auto& have_element : have) {
        const std::string& have_str = have_element;
        have_map.push_back(std::make_pair(have_str, count++));
    }

    std::sort(have_map.begin(), have_map.end());

    // Now go one by one through the expected list, and reorganize.
    std::size_t index{0};
    json result_array;
    for (const auto& expected_element : expected) {
        const std::string& expected_str = expected_element;
        const auto have_found_iter =
            std::lower_bound(have_map.begin(), have_map.end(), expected_str,
                             [](const auto& a, const auto& b) { return a.first < b; });
        bool have_found =
            have_found_iter != have_map.end() && have_found_iter->first == expected_str;
        std::string index_str(std::to_string(index));
        if (!have_found) {
            notify("missing required string at index " + index_str,
                   "required string inserted at index " + index_str);
            result_array.push_back(expected_str);
            failure = true;
        } else {
            std::size_t have_index = have_found_iter->second;
            if (have_index != index) {
                std::string have_index_str(std::to_string(have_index));
                notify("bad item location for item `" + expected_str + "`; have: " + have_index_str + ", expected: " + index_str,
                       "moved item `" + expected_str + "` at index " + have_index_str + " to index " + index_str);
                failure = true;
            }
            result_array.push_back(expected_str);
        }
        ++index;
    }

    if (have.size() != expected.size()) {
        std::string have_size(std::to_string(have.size()));
        std::string expected_size(std::to_string(expected.size()));
        std::string message =
            "sequence size mismatch; have " + have_size + ", expected " + expected_size;
        notify(message, message + "; fixed");
        failure = true;
    }

    result = std::move(result_array);

    return failure;
}

/**************************************************************************************************/

bool yaml_base_emitter::check_object_array(const std::string& filepath,
                                           const json& have_node,
                                           const json& expected_node,
                                           const std::string& nodepath,
                                           json& merged_node,
                                           const std::string& key,
                                           const std::string& object_key,
                                           const check_proc& proc) {
    const auto notify = [&](const std::string& validate_message,
                            const std::string& update_message) {
        check_notify(filepath, nodepath, key, validate_message, update_message);
    };

    const auto notify_fail = [&](const std::string& message) {
        check_notify(filepath, nodepath, key, message, message);
    };

    if (!expected_node.count(key)) {
        if (!have_node.count(key)) {
            // Removed key not present in have. Do nothing, no error.
            return false;
        } else {
            notify("value present for removed key", "value removed");
            return true;
        }
    }

    const json& expected = expected_node[key];

    if (!expected.is_array()) {
        notify_fail("expected type mismatch");
        throw std::runtime_error("Merge object array failure");
    }

    json& result = merged_node[key];

    if (!have_node.count(key) || !have_node[key].is_array()) {
        notify("value not an array", "non-array value replaced");
        result = expected;
        return true;
    }

    const json& have = have_node[key];

    // How does one merge an array of objects? The solution is to use one of the
    // elements in each object as the object's "key", and treat the array like a
    // dictionary. This does create the possibility that multiple objects will
    // resolve to the same key, and/or the key value may not be a string, so we
    // establish those requirements as function preconditions.
    //
    // As for the actual merge, we have a `have` array, and an `expected` array.
    // The resulting merge will be built up in a resulting array, then moved
    // into `result.`
    //
    // First we build up a sorted vector of key/position pairs of the `have`
    // array. This will let us find elements in that array in log(N) time.
    //
    // Then we iterate the expected array from first to last. We pull out the
    // expected key and search for it in the have array. If it is missing, copy
    // the expected element into the result array, and move on. If it is found,
    // make sure they are in the same index in both arrays, otherwise it's a
    // validation error. Once that is done, merge the two objects together with
    // the user callback. The merged result is then pushed onto the back of the
    // result array. The result array is then moved into the resulting json.
    //
    // If `have` has extraneous keys, they will be skipped during the iteration
    // of `expected` and subsequently dropped.
    //
    // The entire merge process should be O(N log N) time, bounded on the sort
    // of the have key vector. Thanks to Jared Wyles for the tip on how to solve
    // this one.

    std::vector<std::pair<std::string, std::size_t>> have_map;
    std::size_t count{0};
    bool failure{false};
    for (const auto& have_elements : have) {
        if (have_elements.count(object_key) == 0) {
            std::string count_str(std::to_string(count));
            std::string message("object at index " + count_str + " has no key");
            notify(message, message + "; skipped");
            // preserve object indices for merging below. Name is irrelevant as
            // long as it's unique. Prefix with '.' to prevent actual key
            // conflicts.
            have_map.push_back(std::make_pair(".unnamed-" + count_str, count++));
            failure = true;
        } else {
            const std::string& key = have_elements[object_key];
            have_map.push_back(std::make_pair(key, count++));
        }
    }

    std::sort(have_map.begin(), have_map.end());

    // Now go one by one through the expected list, and reorganize.
    std::size_t index{0};
    json result_array;
    for (const auto& expected_object : expected) {
        const std::string& expected_key = expected_object[object_key];
        const auto have_found_iter =
            std::lower_bound(have_map.begin(), have_map.end(), expected_key,
                             [](const auto& a, const auto& b) { return a.first < b; });
        bool have_found =
            have_found_iter != have_map.end() && have_found_iter->first == expected_key;
        std::string index_str(std::to_string(index));
        if (!have_found) {
            notify("missing required object at index " + index_str,
                   "required object inserted at index " + index_str);
            result_array.push_back(expected_object);
            failure = true;
        } else {
            std::size_t have_index = have_found_iter->second;
            if (have_index != index) {
                std::string have_index_str(std::to_string(have_index));
                notify("bad item location for key `" + expected_key + "`; have: " + have_index_str + ", expected: " + index_str,
                       "moved item with key `" + expected_key + "` at index " + have_index_str + " to index " + index_str);
                failure = true;
            }
            std::string nodepath = "['" + key + "'][" + index_str + "]";
            json merged = have[have_index];
            failure |= proc(filepath, have[have_index], expected_object, nodepath, merged);
            result_array.push_back(std::move(merged));
        }
        ++index;
    }

    if (have.size() != expected.size()) {
        std::string have_size(std::to_string(have.size()));
        std::string expected_size(std::to_string(expected.size()));
        std::string message =
            "sequence size mismatch; have " + have_size + ", expected " + expected_size;
        notify(message, message + "; fixed");
        failure = true;
    }

    result = std::move(result_array);

    return failure;
}

/**************************************************************************************************/

bool yaml_base_emitter::check_map(const std::string& filepath,
                                  const json& have_node,
                                  const json& expected_node,
                                  const std::string& nodepath,
                                  json& merged_node,
                                  const std::string& key,
                                  const check_proc& proc) {
    const auto notify = [&](const std::string& validate_message,
                            const std::string& update_message) {
        check_notify(filepath, nodepath, key, validate_message, update_message);
    };

    if (!expected_node.count(key)) {
        if (!have_node.count(key)) {
            // Removed key not present in have. Do nothing, no error.
            return false;
        } else {
            notify("value present for removed key", "value removed");
            return true;
        }
    }

    const json& expected = expected_node[key];

    if (!expected.is_object()) {
        throw std::runtime_error("expected type mismatch?");
    }

    json& result = merged_node[key];

    if (!have_node.count(key) || !have_node[key].is_object()) {
        notify("value not a map", "non-map value replaced");
        result = expected;
        return true;
    }

    const json& have = have_node[key];

    std::vector<std::string> keys;

    for (auto iter{have.begin()}, last{have.end()}; iter != last; ++iter) {
        keys.push_back(static_cast<const std::string&>(iter.key()));
    }
    for (auto iter{expected.begin()}, last{expected.end()}; iter != last; ++iter) {
        keys.push_back(static_cast<const std::string&>(iter.key()));
    }

    std::sort(keys.begin(), keys.end());
    keys.erase(std::unique(keys.begin(), keys.end()), keys.end());

    bool failure{false};
    
    result = have;

    for (const auto& subkey : keys) {
        std::string curnodepath = nodepath + "['" + subkey + "']";

        if (!expected.count(subkey)) {
            notify("extraneous map key: `" + subkey + "`", "map key removed: `" + subkey + "`");
            result.erase(subkey);
            failure = true;
        } else if (!have.count(subkey)) {
            notify("map key missing: `" + subkey + "`", "map key inserted: `" + subkey + "`");
            result[subkey] = expected[subkey];
            failure = true;
        } else {
            failure |= proc(filepath, have[subkey], expected[subkey], curnodepath, result[subkey]);
        }
    }

    return failure;
}

/**************************************************************************************************/

std::pair<bool, json> yaml_base_emitter::merge(const std::string& filepath,
                                               const json& have,
                                               const json& expected) {
    bool failure{false};
    
    // Create a temporary object with the json to merge as a value so we can use `check_map`
    // to make sure removed keys are handled
    static const auto root_key = "<root>";
    json merged_root = hyde::json::object();
    json have_root = hyde::json::object();
    have_root[root_key] = have;
    json expected_root = hyde::json::object();
    expected_root[root_key] = expected;
    failure |= check_map(filepath, have_root, expected_root, "", merged_root, root_key,
        [](const std::string& filepath, const json& have, const json& expected,
            const std::string& nodepath, json& out_merged) { return false; });
    json& merged = merged_root[root_key];
    
    // we can probably get rid of `have` in the check
    // routines; I don't think we can keep it from being an
    // out-arg, though, because we need to preserve the
    // values in `have` that are not managed by the
    // `expected` schema.

    failure |= check_scalar(filepath, have, expected, "", merged, "layout");
    failure |= check_scalar(filepath, have, expected, "", merged, "title");
    failure |= check_scalar(filepath, have, expected, "", merged, "owner");
    failure |= check_scalar(filepath, have, expected, "", merged, "brief");
    failure |= check_scalar_array(filepath, have, expected, "", merged, "tags");

    failure |= do_merge(filepath, have, expected, merged);

    return std::make_pair(failure, merged);
}

/**************************************************************************************************/

inline std::uint64_t fnv_1a(const std::string& s) {
    constexpr std::uint64_t prime_k = 0x100000001b3;

    std::uint64_t result(0xcbf29ce484222325);

    for (const auto& c : s) {
        result = (result ^ static_cast<std::uint64_t>(c)) * prime_k;
    }

    return result;
    }

/**************************************************************************************************/

std::string yaml_base_emitter::filename_truncate(std::string s) {
    if (s.size() <= 32) return s;

    // We cannot use std::hash here because the hashing algorithm may give
    // different results per-platform. Instead we fall back on an old
    // favorite of mine for this kind of thing: FNV-1a.
    const auto hash = [&_s = s]() {
        auto hash = fnv_1a(_s) & 0xFFFFFFFF;
        std::stringstream stream;
        stream << std::hex << hash;
        return stream.str();
    }();

    // 23 + 1 + 8 = 32 max characters per directory name
    s = s.substr(0, 23) + '.' + hash;

    return s;
}

/**************************************************************************************************/

boost::filesystem::path yaml_base_emitter::directory_mangle(boost::filesystem::path p) {
    boost::filesystem::path result;

    for (const auto& part : p) {
        result /= filename_truncate(filename_filter(part.string()));
    }

    return result;
}

/**************************************************************************************************/

bool yaml_base_emitter::create_directory_stub(boost::filesystem::path p) {
    auto stub_name = p / index_filename_k;

    if (exists(stub_name)) return false;

    boost::filesystem::ofstream output(stub_name);

    if (!output) {
        std::cerr << stub_name.string() << ": could not create directory stub\n";
        return true;
    }

    static const auto stub_json_k = json::object_t{
        {"layout", "directory"},
    };

    output << front_matter_delimiter_k;
    output << json_to_yaml_ordered(stub_json_k) << '\n';
    output << front_matter_delimiter_k;

    return false;
}

/**************************************************************************************************/

bool yaml_base_emitter::create_path_directories(boost::filesystem::path p) {
    if (p.has_filename()) p = p.parent_path();

    std::vector<boost::filesystem::path> ancestors;

    while (true) {
        ancestors.push_back(p);
        if (!p.has_parent_path()) break;
        p = p.parent_path();
    }

    std::reverse(ancestors.begin(), ancestors.end());

    for (const auto& ancestor : ancestors) {
        if (checker_s.exists(ancestor)) continue;

        if (_mode == yaml_mode::validate) {
            return true;
        } else {
            boost::system::error_code ec;
            create_directory(ancestor, ec);

            if (ec) {
                static std::map<std::string, bool> bad_path_map_s;
                const auto bad_path = ancestor.string();
                if (!bad_path_map_s.count(bad_path))
                    std::cerr << bad_path << ": directory could not be created (" << ec << ")\n";
                bad_path_map_s[bad_path] = true;
                return true;
            }

            if (create_directory_stub(ancestor)) {
                return true;
            }
        }
    }

    return false;
}

/**************************************************************************************************/

auto load_yaml(const boost::filesystem::path& path) try {
    return YAML::LoadFile(path.c_str());
} catch (...) {
    std::cerr << "YAML File: " << path.string() << '\n';
    throw;
}

/**************************************************************************************************/

bool yaml_base_emitter::reconcile(json expected,
                                  boost::filesystem::path root_path,
                                  boost::filesystem::path path,
                                  json& out_reconciled) {
    bool failure{false};

    /* begin hack */ {
        // I hope to remove this soon. Paths with '...' in them make Perforce go
        // stir-crazy (it's a special token for the tool), so we remove them.
        static const std::string needle = "...";
        std::string p_str = path.string();
        std::size_t pos = 0;
        auto found = false;
        while (true) {
            pos = p_str.find(needle, pos);
            if (pos == std::string::npos) break;
            found = true;
            p_str.replace(pos, needle.size(), "");
        }
        if (found) {
            path = boost::filesystem::path(p_str);
        }
    }

    std::string relative_path(("." / relative(path, root_path)).string());

    failure |= create_path_directories(path);

    if (checker_s.exists(path)) {
        // we have to load the file ourselves and find the place where the
        // front-matter ends and any other relevant documentation begins. We
        // need to do this for the boilerpolate step to keep it from blasting
        // out any extra documentation that's already been added.
        boost::filesystem::ifstream have_file(path);
        std::stringstream have_contents_stream;
        have_contents_stream << have_file.rdbuf();
        std::string have_contents = have_contents_stream.str();
        auto front_matter_pos = have_contents.find_first_of(front_matter_delimiter_k);
        auto front_matter_end = have_contents.find(
            front_matter_delimiter_k, front_matter_pos + front_matter_delimiter_k.size());
        std::string yaml_src = have_contents.substr(
            front_matter_pos, front_matter_end + front_matter_delimiter_k.size());
        have_contents.erase(front_matter_pos, front_matter_end + front_matter_delimiter_k.size());
        std::string remainder = std::move(have_contents);
        json have = yaml_to_json(load_yaml(path));
        json merged;

        std::tie(failure, merged) = merge(relative_path, have, expected);
        out_reconciled = merged;
        out_reconciled["documentation_path"] = relative_path;

        switch (_mode) {
            case hyde::yaml_mode::validate: {
                // do nothing
            } break;
            case hyde::yaml_mode::update: {
                boost::filesystem::ofstream output(path);
                if (!output) {
                    std::cerr << "./" << path.string() << ": could not open file for output\n";
                    failure = true;
                } else {
                    output << front_matter_delimiter_k;
                    output << json_to_yaml_ordered(merged) << '\n';
                    output << front_matter_delimiter_k;
                    output << remainder;
                }
            } break;
        }
    } else { // file does not exist
        out_reconciled = expected;

        switch (_mode) {
            case hyde::yaml_mode::validate: {
                std::cerr << relative_path << ": required file does not exist\n";
                failure = true;
            } break;
            case hyde::yaml_mode::update: {
                // Add update. No remainder yet, as above.
                boost::filesystem::ofstream output(path);
                if (!output) {
                    std::cerr << "./" << path.string() << ": could not open file for output\n";
                    failure = true;
                } else {
                    output << "---\n";
                    output << update_cleanup(json_to_yaml_ordered(expected)) << '\n';
                    output << "---\n";
                }
            } break;
        }
    }

    return failure;
}


/**************************************************************************************************/

std::string yaml_base_emitter::defined_in_file(const std::string& src_path,
                                               const boost::filesystem::path& src_root) {
    return relative(boost::filesystem::path(src_path), src_root).string();
}

/**************************************************************************************************/

boost::filesystem::path yaml_base_emitter::subcomponent(const boost::filesystem::path& src_path,
                                                        const boost::filesystem::path& src_root) {
    return boost::filesystem::relative(src_path, src_root);
}

/**************************************************************************************************/

void yaml_base_emitter::maybe_annotate(const json& j, json& node) {
    std::string annotation;

    if (j.count("access")) {
        const std::string& access = j["access"];

        if (access != "public") {
            node["annotation"].push_back(access);
        }
    }

    if (j.count("default") && j["default"])
        node["annotation"].push_back("default");
    else if (j.count("compiler-default") && j["compiler-default"])
        node["annotation"].push_back("compiler-default");
    else if (j.count("delete") && j["delete"])
        node["annotation"].push_back("delete");
    else if (j.count("compiler-delete") && j["compiler-delete"])
        node["annotation"].push_back("compiler-delete");

    if (j.count("deprecated") && j["deprecated"]) {
        std::string deprecated("deprecated");
        if (j.count("deprecated_message")) {
            const std::string& message_str = j["deprecated_message"];
            if (!message_str.empty()) {
                deprecated = deprecated.append(" (")
                                 .append(message_str)
                                 .append(")");
            }
        }
        node["annotation"].push_back(deprecated);
    }
}

/**************************************************************************************************/

std::string yaml_base_emitter::format_template_parameters(const hyde::json& json, bool with_types) {
    std::string result;

    if (json.count("template_parameters")) {
        std::size_t count{0};
        if (with_types) result += "template ";
        result += "<";
        for (const auto& param : json["template_parameters"]) {
            if (count++) result += ", ";
            if (with_types) {
                result += static_cast<const std::string&>(param["type"]);
                if (param.count("parameter_pack")) result += "...";
                result += " " + static_cast<const std::string&>(param["name"]);
            } else {
                result += static_cast<const std::string&>(param["name"]);
                if (param.count("parameter_pack")) result += "...";
            }
        }
        result += ">";
    }

    return result;
}

/**************************************************************************************************/

std::string yaml_base_emitter::filename_filter(std::string f) {
    constexpr const char* const uri_equivalent[] = {
        "00", "01", "02", "03", "04", "05", "06", "07", "08", "09", "0A", "0B", "0C", "0D", "0E",
        "0F", "10", "11", "12", "13", "14", "15", "16", "17", "18", "19", "1A", "1B", "1C", "1D",
        "1E", "1F", "20", "21", "22", "23", "24", "25", "26", "27", "28", "29", "2A", "2B", "2C",
        "-",  ".",  "2F", "0",  "1",  "2",  "3",  "4",  "5",  "6",  "7",  "8",  "9",  "3A", "3B",
        "3C", "3D", "3E", "3F", "40", "A",  "B",  "C",  "D",  "E",  "F",  "G",  "H",  "I",  "J",
        "K",  "L",  "M",  "N",  "O",  "P",  "Q",  "R",  "S",  "T",  "U",  "V",  "W",  "X",  "Y",
        "Z",  "5B", "5C", "5D", "5E", "_",  "60", "a",  "b",  "c",  "d",  "e",  "f",  "g",  "h",
        "i",  "j",  "k",  "l",  "m",  "n",  "o",  "p",  "q",  "r",  "s",  "t",  "u",  "v",  "w",
        "x",  "y",  "z",  "7B", "7C", "7D", "~",  "7F", "80", "81", "82", "83", "84", "85", "86",
        "87", "88", "89", "8A", "8B", "8C", "8D", "8E", "8F", "90", "91", "92", "93", "94", "95",
        "96", "97", "98", "99", "9A", "9B", "9C", "9D", "9E", "9F", "A0", "A1", "A2", "A3", "A4",
        "A5", "A6", "A7", "A8", "A9", "AA", "AB", "AC", "AD", "AE", "AF", "B0", "B1", "B2", "B3",
        "B4", "B5", "B6", "B7", "B8", "B9", "BA", "BB", "BC", "BD", "BE", "BF", "C0", "C1", "C2",
        "C3", "C4", "C5", "C6", "C7", "C8", "C9", "CA", "CB", "CC", "CD", "CE", "CF", "D0", "D1",
        "D2", "D3", "D4", "D5", "D6", "D7", "D8", "D9", "DA", "DB", "DC", "DD", "DE", "DF", "E0",
        "E1", "E2", "E3", "E4", "E5", "E6", "E7", "E8", "E9", "EA", "EB", "EC", "ED", "EE", "EF",
        "F0", "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "FA", "FB", "FC", "FD", "FE",
        "FF",
    };

    std::string result; // preallocate? stringstream?

    std::for_each(f.begin(), f.end(),
                  [&](const auto& c) { result += uri_equivalent[static_cast<int>(c)]; });

    return result;
}

/**************************************************************************************************/

} // namespace hyde

/**************************************************************************************************/
