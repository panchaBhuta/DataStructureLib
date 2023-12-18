#
# URL:      https://github.com/panchaBhuta/dataStructure
#
# Copyright (c) 2023-2023 Gautam Dhar
# All rights reserved.
#
# dataStructure is distributed under the BSD 3-Clause license, see LICENSE for details. 
#

# This code is losely based on 'project(UpstreamLib)' as in url below.
# https://cmake.org/cmake/help/v3.27/manual/cmake-packages.7.html#creating-packages

# Project
cmake_minimum_required(VERSION 3.23)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")
include(dataStructure)
set(PROJECT_DESCRIPTION "A header-only lightweight C++ object version")

# Get the version of the library
dataStructure_getversion(VERSION)

#[==================================================================================[
# https://cmake.org/cmake/help/v3.27/guide/tutorial/Adding%20Generator%20Expressions.html#cmakelists-txt-target-compile-options-genex
#
# refer https://cmake.org/cmake/help/v3.27/manual/cmake-generator-expressions.7.html#genex:COMPILE_LANG_AND_ID
# when the language used for compilation unit matches language and CMake's compiler id of the language compiler
# matches any one of the comma-separated entries in compiler_ids, otherwise 0.
#
# Compiler identification string : Possible values
# https://cmake.org/cmake/help/latest/variable/CMAKE_LANG_COMPILER_ID.html
#
#]==================================================================================]

project(dataStructure
        VERSION "${VERSION}"
        DESCRIPTION "${PROJECT_DESCRIPTION}"
        HOMEPAGE_URL "${PROJECT_HOMEPAGE_URL}"
        LANGUAGES CXX)



# Must include after the project call due to GNUInstallDirs requiring a language be enabled (IE. CXX)
include(GNUInstallDirs)

message(STATUS "#### OS-id=${CMAKE_SYSTEM_NAME}   OS-id-version=${CMAKE_SYSTEM}")
message(STATUS "#### CXX_COMPILER_ID=${CMAKE_CXX_COMPILER_ID}    version=${CMAKE_CXX_COMPILER_VERSION}")

# Determine whether this is a standalone project or included by other projects
set(DATASTRUCTURE_STANDALONE_PROJECT OFF)
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(DATASTRUCTURE_STANDALONE_PROJECT ON)
endif()
#[===========[  donot use generator-expressions in set() functions
set(DATASTRUCTURE_STANDALONE_PROJECT   $<STREQUAL:"${CMAKE_SOURCE_DIR}","${PROJECT_SOURCE_DIR}">)
# refer  https://stackoverflow.com/questions/35695152/cmake-generator-expression-is-not-being-evaluated
# While generator expression is stored at configuration stage
# (when corresponded CMake command is executed), evaluation of
# generator expressions is performed at build stage.

# This is why message() command prints generator expression in
# non-dereferenced form: value denoted by the generator
# expression is not known at this stage.
#]===========]
message(STATUS "DATASTRUCTURE_STANDALONE_PROJECT=${DATASTRUCTURE_STANDALONE_PROJECT}")

# Establish the project options
option(DATASTRUCTURE_BUILD_EXAMPLES "Set to ON to build examples"        ${DATASTRUCTURE_STANDALONE_PROJECT})
option(DATASTRUCTURE_BUILD_TESTS "Set to ON to build tests"              ${DATASTRUCTURE_STANDALONE_PROJECT})
option(DATASTRUCTURE_ENABLE_INSTALL "Generate the install target"        ${DATASTRUCTURE_STANDALONE_PROJECT})
option(DATASTRUCTURE_ENABLE_WARNINGS "Add warnings to CMAKE_CXX_FLAGS"   ${DATASTRUCTURE_STANDALONE_PROJECT})
#option(DATASTRUCTURE_USE_UNICODE_HELP "Use ICU Unicode library"         OFF)

if (DATASTRUCTURE_STANDALONE_PROJECT)
    dataStructure_set_cxx_standard()
endif()

#message(STATUS "clang_cxx=${clang_cxx}")
file(TO_NATIVE_PATH "/" _path_separator)
message(STATUS "The systems's path separator is '${_path_separator}'")

# Library
add_library(dataStructure INTERFACE)
add_library(dataStructure::dataStructure ALIAS dataStructure)

fetch_dependencies()

#if (DATASTRUCTURE_STANDALONE_PROJECT)
    dataStructure_check_cxx_compiler_flag_file_prefix_map()
#endif()

if (DATASTRUCTURE_ENABLE_WARNINGS)
    dataStructure_enable_warnings()
endif()

dataStructure_build()


# Link against the ICU library when requested
#if(DATASTRUCTURE_USE_UNICODE_HELP)
#    dataStructure_use_unicode()
#endif()

# Install dataStructure when requested by the user
if (DATASTRUCTURE_ENABLE_INSTALL)
    dataStructure_install_logic()
endif()




# Enable testing when requested by the user
message(STATUS "Build tests: ${DATASTRUCTURE_BUILD_TESTS}")
if(DATASTRUCTURE_BUILD_TESTS)
    enable_testing()
    include(CTest)
    # Ccache
    find_program(CCACHE_PROGRAM ccache)
    if(CCACHE_PROGRAM)
        message(STATUS "Found ccache")
        set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
    endif()

    add_subdirectory(tests)
endif()




#[===========[
# Build examples when requested by the user
message(STATUS "Build examples: ${DATASTRUCTURE_BUILD_EXAMPLES}")
if (DATASTRUCTURE_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
#]===========]

