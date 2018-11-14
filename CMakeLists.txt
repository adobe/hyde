# Copyright 2018 Adobe
# All Rights Reserved.

# NOTICE: Adobe permits you to use, modify, and distribute this file in
# accordance with the terms of the Adobe license agreement accompanying
# it. If you have received this file from a source other than Adobe,
# then your use, modification, or distribution of it requires the prior
# written permission of Adobe. 


cmake_minimum_required(VERSION 3.12)

project(hyde)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


if(APPLE)
    set(Clang_DIR /usr/local/Cellar/llvm/7.0.0/lib/cmake/clang/)
    set(LLVM_DIR /usr/local/Cellar/llvm/7.0.0/lib/cmake/llvm/)
endif()

find_package(Clang REQUIRED clangTooling libClang clangASTMatchers)
find_package(Clang REQUIRED CONFIG)
find_package(LLVM REQUIRED CONFIG)
find_package(YAML-CPP REQUIRED CONFIG)

set(Boost_USE_STATIC_LIBS ON)
find_package(Boost COMPONENTS system filesystem REQUIRED)

add_definitions(${Clang_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS})
add_definitions(${YAML_CPP_DEFINITIONS})
add_definitions(${Boost_LIB_DIAGNOSTIC_DEFINITIONS})


#lets get the clang resource dir so we can set inside hyde
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

execute_process(COMMAND ${CLANG_BIN} -print-resource-dir
    RESULT_VARIABLE CLANG_RESOURCE_DIR_RESULT
    OUTPUT_VARIABLE CLANG_RESOURCE_DIR
    ERROR_VARIABLE  CLANG_RESOURCE_DIR_ERROR
    OUTPUT_STRIP_TRAILING_WHITESPACE)

if(CLANG_RESOURCE_DIR_RESULT)
    message(FATAL_ERROR "Could not locate the resource dir for clang we are going to link against")
    message(FATAL_ERROR ${CLANG_RESOURCE_DIR_ERROR})
endif()

message(STATUS "Using the following resource dir for clang: ${CLANG_RESOURCE_DIR}")

file(GLOB EMITTER_FILES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/emitters/*.cpp)
file(GLOB MATCHER_FILES CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/matchers/*.cpp)
file(GLOB SRC_FILES CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/sources/*.cpp)

#TODO: use target_source
add_executable(hyde  ${EMITTER_FILES} ${MATCHER_FILES} ${SRC_FILES})
if (NOT LLVM_ENABLE_RTTI)
    target_compile_options (hyde PRIVATE -fno-rtti)
endif()

target_compile_definitions(hyde PRIVATE
 CLANG_RESOURCE_DIR="${CLANG_RESOURCE_DIR}"
)

#our headers
target_include_directories(hyde PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(hyde PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

#3rd party
target_include_directories(hyde SYSTEM PUBLIC ${Boost_INCLUDE_DIRS})
target_include_directories(hyde SYSTEM PUBLIC ${CLANG_INCLUDE_DIRS})
target_include_directories(hyde SYSTEM PUBLIC ${LLVM_INCLUDE_DIRS})
#this gets a little messy with gcc so it can't be a system include :(
#you end up with a "can't find stdlib.h" error if it is
target_include_directories(hyde PUBLIC ${YAML_CPP_INCLUDE_DIR})


target_compile_options(hyde PUBLIC -Wall  -Werror)

target_link_libraries(hyde
                      ${YAML_CPP_LIBRARIES}
                      ${Boost_FILESYSTEM_LIBRARY}
                      ${Boost_SYSTEM_LIBRARY}
                      clangASTMatchers
                      clangBasic
                      clangTooling
                      )
