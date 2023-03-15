# Copyright 2018 Adobe
# All Rights Reserved.

# NOTICE: Adobe permits you to use, modify, and distribute this file in
# accordance with the terms of the Adobe license agreement accompanying
# it. If you have received this file from a source other than Adobe,
# then your use, modification, or distribution of it requires the prior
# written permission of Adobe.

cmake_minimum_required(VERSION 3.23)

include(FetchContent)

set(CMAKE_OSX_ARCHITECTURES x86_64) # Only build x86 for now

project(hyde)

if (NOT DEFINED CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 20)
endif ()

set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_XCODE_GENERATE_SCHEME OFF)

if (NOT TARGET llvm::llvm)
    message(STATUS "Hyde third-party: creating target 'llvm::llvm'...")

    FetchContent_Declare(
        llvm
        GIT_REPOSITORY https://github.com/llvm/llvm-project.git
        GIT_TAG 8dfdcc7b7bf66834a761bd8de445840ef68e4d1a # llvmorg-15.0.7
        GIT_SHALLOW TRUE
        SOURCE_SUBDIR llvm
    )

    set(LLVM_ENABLE_PROJECTS clang)
    set(LLVM_TARGETS_TO_BUILD X86)

    FetchContent_MakeAvailable(llvm)
endif()

set(Boost_USE_STATIC_LIBS ON)

find_package(Boost
    COMPONENTS
        system
        filesystem
    REQUIRED
)

# Let's get the clang resource dir so we can set inside hyde
if(DEFINED USE_APPLE_TOOLCHAIN)
    execute_process(COMMAND "xcode-select" -p
        RESULT_VARIABLE XCODE_SELECT_RESULT
        OUTPUT_VARIABLE XCODE_SELECT
        ERROR_VARIABLE  XCODE_SELECT_ERROR
        OUTPUT_STRIP_TRAILING_WHITESPACE)

    if(XCODE_SELECT_RESULT)
        message(FATAL_ERROR "USE_APPLE_TOOLCHAIN was set but can not invoke xcode-select -p")
        message(FATAL_ERROR ${XCODE_SELECT_ERROR})
    endif()

    set (CLANG_BIN ${XCODE_SELECT}/usr/bin/clang++)
else()
    set (CLANG_BIN ${CLANG_INSTALL_PREFIX}/bin/clang++)
endif()

file(GLOB EMITTER_FILES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/emitters/*.cpp)
file(GLOB MATCHER_FILES CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/matchers/*.cpp)
file(GLOB SRC_FILES CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/sources/*.cpp)
file(GLOB YAML_CPP_SRC_FILES CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/submodules/yaml-cpp/src/*.cpp)

#TODO: use target_source
add_executable(hyde
    ${EMITTER_FILES}
    ${MATCHER_FILES}
    ${SRC_FILES}
    ${YAML_CPP_SRC_FILES}
)

target_include_directories(hyde
    PUBLIC
        ${Boost_INCLUDE_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/submodules/yaml-cpp/include/
        ${PROJECT_SOURCE_DIR}/submodules/json/include/
        ${llvm_SOURCE_DIR}/clang/include
        ${llvm_BINARY_DIR}/tools/clang/include
        ${llvm_SOURCE_DIR}/llvm/include
        ${llvm_BINARY_DIR}/include
)

target_compile_options(hyde
    PUBLIC
        -Wall
        -Wno-comment
        -Werror
        -Wno-range-loop-analysis
        -DHYDE_FORCE_BOOST_FILESYSTEM=1
)

if (NOT LLVM_ENABLE_RTTI)
    target_compile_options(hyde PRIVATE -fno-rtti)
endif()

target_link_libraries(hyde
    ${Boost_FILESYSTEM_LIBRARY}
    ${Boost_SYSTEM_LIBRARY}
    clang
    clangASTMatchers
    clangBasic
    clangTooling
)

if (PROJECT_IS_TOP_LEVEL)
    set_target_properties(hyde PROPERTIES XCODE_GENERATE_SCHEME ON)
endif()
