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

# Change these if you have a different location.
set(BOOST_DIR /usr/local/Cellar/boost/1.67.0_1/)
set(Clang_DIR /usr/local/Cellar/llvm/7.0.0/lib/cmake/clang/)
set(LLVM_DIR /usr/local/Cellar/llvm/7.0.0/lib/cmake/llvm/)
set(YAML_CPP_DIR /usr/local/Cellar/yaml-cpp/0.6.2/lib/cmake/yaml-cpp/)

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


file(GLOB EMITTER_FILES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/emitters/*.cpp)
file(GLOB MATCHER_FILES CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/matchers/*.cpp)
file(GLOB SRC_FILES CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/sources/*.cpp)

#TODO: use target_source
add_executable(hyde   ${EMITTER_FILES} ${MATCHER_FILES} ${SRC_FILES})
if (NOT LLVM_ENABLE_RTTI)
    target_compile_options (hyde PRIVATE -fno-rtti)
endif()


target_include_directories(hyde PUBLIC ${Boost_INCLUDE_DIRS})
target_include_directories(hyde PUBLIC ${CLANG_INCLUDE_DIRS})
target_include_directories(hyde PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(hyde PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(hyde PUBLIC ${LLVM_INCLUDE_DIRS})
target_include_directories(hyde PUBLIC ${YAML_CPP_INCLUDE_DIR})

target_link_libraries(hyde
                      ${YAML_CPP_LIBRARIES}
                      ${Boost_FILESYSTEM_LIBRARY}
                      ${Boost_SYSTEM_LIBRARY}
                      clangASTMatchers
                      clangBasic
                      clangTooling
                      )
