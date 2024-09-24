# Copyright 2018 Adobe
# All Rights Reserved.

# NOTICE: Adobe permits you to use, modify, and distribute this file in
# accordance with the terms of the Adobe license agreement accompanying
# it. If you have received this file from a source other than Adobe,
# then your use, modification, or distribution of it requires the prior
# written permission of Adobe.

cmake_minimum_required(VERSION 3.23)

set(CMAKE_POLICY_DEFAULT_CMP0077 NEW) # Avoid overriding normal variables with option()
set(CMAKE_POLICY_DEFAULT_CMP0126 NEW) # Avoid overriding normal variables with set(CACHE)

include(FetchContent)

set(FETCHCONTENT_QUIET FALSE)

set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64")

project(hyde)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_XCODE_GENERATE_SCHEME OFF)

message(STATUS "INFO: Setting up LLVM...")

FetchContent_Declare(
    llvm
    GIT_REPOSITORY https://github.com/llvm/llvm-project.git
    GIT_TAG 7cbf1a2591520c2491aa35339f227775f4d3adf6 # llvmorg-16.0.6
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
    SOURCE_SUBDIR llvm
)

FetchContent_Declare(
    diff
    GIT_REPOSITORY https://github.com/fosterbrereton/diff.git
    GIT_TAG 702efcba1ab81f6a86a196a941c2393125347386
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
    SOURCE_SUBDIR diff
)

set(LLVM_ENABLE_PROJECTS "clang")
set(LLVM_TARGETS_TO_BUILD "X86;AArch64")
set(LLVM_ENABLE_ZSTD OFF)

FetchContent_MakeAvailable(llvm)
FetchContent_MakeAvailable(diff)

message(STATUS "INFO: LLVM source dir: ${llvm_SOURCE_DIR}")
message(STATUS "INFO: LLVM binary dir: ${llvm_BINARY_DIR}")

add_executable(hyde)

set(SRC_SOURCES
    ${PROJECT_SOURCE_DIR}/sources/autodetect.cpp
    ${PROJECT_SOURCE_DIR}/sources/main.cpp
    ${PROJECT_SOURCE_DIR}/sources/output_yaml.cpp
)

set(SRC_EMITTERS
    ${PROJECT_SOURCE_DIR}/emitters/yaml_base_emitter.cpp
    ${PROJECT_SOURCE_DIR}/emitters/yaml_class_emitter.cpp
    ${PROJECT_SOURCE_DIR}/emitters/yaml_enum_emitter.cpp
    ${PROJECT_SOURCE_DIR}/emitters/yaml_function_emitter.cpp
    ${PROJECT_SOURCE_DIR}/emitters/yaml_library_emitter.cpp
    ${PROJECT_SOURCE_DIR}/emitters/yaml_sourcefile_emitter.cpp
)

set(SRC_MATCHERS
    ${PROJECT_SOURCE_DIR}/matchers/class_matcher.cpp
    ${PROJECT_SOURCE_DIR}/matchers/enum_matcher.cpp
    ${PROJECT_SOURCE_DIR}/matchers/function_matcher.cpp
    ${PROJECT_SOURCE_DIR}/matchers/namespace_matcher.cpp
    ${PROJECT_SOURCE_DIR}/matchers/typealias_matcher.cpp
    ${PROJECT_SOURCE_DIR}/matchers/typedef_matcher.cpp
    ${PROJECT_SOURCE_DIR}/matchers/utilities.cpp
)

set(SRC_YAMLCPP
    ${PROJECT_SOURCE_DIR}/submodules/yaml-cpp/src/binary.cpp
    ${PROJECT_SOURCE_DIR}/submodules/yaml-cpp/src/convert.cpp
    ${PROJECT_SOURCE_DIR}/submodules/yaml-cpp/src/depthguard.cpp
    ${PROJECT_SOURCE_DIR}/submodules/yaml-cpp/src/directives.cpp
    ${PROJECT_SOURCE_DIR}/submodules/yaml-cpp/src/emit.cpp
    ${PROJECT_SOURCE_DIR}/submodules/yaml-cpp/src/emitfromevents.cpp
    ${PROJECT_SOURCE_DIR}/submodules/yaml-cpp/src/emitter.cpp
    ${PROJECT_SOURCE_DIR}/submodules/yaml-cpp/src/emitterstate.cpp
    ${PROJECT_SOURCE_DIR}/submodules/yaml-cpp/src/emitterutils.cpp
    ${PROJECT_SOURCE_DIR}/submodules/yaml-cpp/src/exceptions.cpp
    ${PROJECT_SOURCE_DIR}/submodules/yaml-cpp/src/exp.cpp
    ${PROJECT_SOURCE_DIR}/submodules/yaml-cpp/src/memory.cpp
    ${PROJECT_SOURCE_DIR}/submodules/yaml-cpp/src/node.cpp
    ${PROJECT_SOURCE_DIR}/submodules/yaml-cpp/src/node_data.cpp
    ${PROJECT_SOURCE_DIR}/submodules/yaml-cpp/src/nodebuilder.cpp
    ${PROJECT_SOURCE_DIR}/submodules/yaml-cpp/src/nodeevents.cpp
    ${PROJECT_SOURCE_DIR}/submodules/yaml-cpp/src/null.cpp
    ${PROJECT_SOURCE_DIR}/submodules/yaml-cpp/src/ostream_wrapper.cpp
    ${PROJECT_SOURCE_DIR}/submodules/yaml-cpp/src/parse.cpp
    ${PROJECT_SOURCE_DIR}/submodules/yaml-cpp/src/parser.cpp
    ${PROJECT_SOURCE_DIR}/submodules/yaml-cpp/src/regex_yaml.cpp
    ${PROJECT_SOURCE_DIR}/submodules/yaml-cpp/src/scanner.cpp
    ${PROJECT_SOURCE_DIR}/submodules/yaml-cpp/src/scanscalar.cpp
    ${PROJECT_SOURCE_DIR}/submodules/yaml-cpp/src/scantag.cpp
    ${PROJECT_SOURCE_DIR}/submodules/yaml-cpp/src/scantoken.cpp
    ${PROJECT_SOURCE_DIR}/submodules/yaml-cpp/src/simplekey.cpp
    ${PROJECT_SOURCE_DIR}/submodules/yaml-cpp/src/singledocparser.cpp
    ${PROJECT_SOURCE_DIR}/submodules/yaml-cpp/src/stream.cpp
    ${PROJECT_SOURCE_DIR}/submodules/yaml-cpp/src/tag.cpp
)

target_sources(hyde
    PRIVATE
        ${SRC_SOURCES}
        ${SRC_EMITTERS}
        ${SRC_MATCHERS}
        ${SRC_YAMLCPP}
)

source_group(sources FILES ${SRC_SOURCES})
source_group(emitters FILES ${SRC_EMITTERS})
source_group(matchers FILES ${SRC_MATCHERS})
source_group(yaml-cpp FILES ${SRC_YAMLCPP})

target_include_directories(hyde
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/submodules/yaml-cpp/include/
        ${PROJECT_SOURCE_DIR}/submodules/json/include/
        ${llvm_SOURCE_DIR}/clang/include
        ${llvm_BINARY_DIR}/tools/clang/include
        ${llvm_SOURCE_DIR}/llvm/include
        ${llvm_BINARY_DIR}/include
        ${diff_SOURCE_DIR}/include
)

target_compile_options(hyde
    PUBLIC
        -Wall
        -Wno-comment
        -Werror
        -Wno-range-loop-analysis
)

if (NOT LLVM_ENABLE_RTTI)
    target_compile_options(hyde PRIVATE -fno-rtti)
endif()

target_link_libraries(hyde
    clang
    clangAST
    clangASTMatchers
    clangBasic
    clangFrontend
    clangLex
    clangTooling
)

if (PROJECT_IS_TOP_LEVEL)
    set_target_properties(hyde PROPERTIES XCODE_GENERATE_SCHEME ON)
endif()
