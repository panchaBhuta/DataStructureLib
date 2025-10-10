#
# URL:      https://github.com/panchaBhuta/dataStructure
#
# Copyright (c) 2023-2025 Gautam Dhar
# All rights reserved.
#
# dataStructure is distributed under the BSD 3-Clause license, see LICENSE for details. 
#

# This code is losely based on 'project(UpstreamLib)' as in url below.
# https://cmake.org/cmake/help/v3.27/manual/cmake-packages.7.html#creating-packages

# Project
cmake_minimum_required(VERSION 3.23)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")
include(dataStructure)   #  cmake/dataStructure.cmake
set(PROJECT_DESCRIPTION "A header-only lightweight C++  versioned-object")
set(PROJECT_HOMEPAGE_URL "https://github.com/panchaBhuta/dataStructure")

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

message(STATUS "############################################### dataStructure :: CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}")
message(STATUS "#### OS-id=${CMAKE_SYSTEM_NAME}   OS-id-version=${CMAKE_SYSTEM}")
message(STATUS "#### CMAKE_CXX_COMPILER_ID=${CMAKE_CXX_COMPILER_ID}    version=${CMAKE_CXX_COMPILER_VERSION}")

dataStructure_cmake_variables_config()

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
option(DATASTRUCTURE_BUILD_EXAMPLES   "Set to ON to build examples"       ${DATASTRUCTURE_STANDALONE_PROJECT})
option(DATASTRUCTURE_BUILD_TESTS      "Set to ON to build tests"          ${DATASTRUCTURE_STANDALONE_PROJECT})
option(DATASTRUCTURE_ENABLE_INSTALL   "Generate the install target"       ${DATASTRUCTURE_STANDALONE_PROJECT})
option(DATASTRUCTURE_ENABLE_WARNINGS  "Add warnings to CMAKE_CXX_FLAGS"   ${DATASTRUCTURE_STANDALONE_PROJECT})
#option(DATASTRUCTURE_USE_UNICODE_HELP "Use ICU Unicode library"           OFF)

set(_DEBUG_LOG OFF)
if (DATASTRUCTURE_STANDALONE_PROJECT AND
    CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(_DEBUG_LOG ON)
endif()
#message(STATUS "_DEBUG_LOG=${_DEBUG_LOG}")
# for _DEBUG_LOG can't use generator-expression as its computed during build-stage, but we need it during config-stage
option(OPTION_VERSIONEDOBJECT_debug_log  "Set to ON for VersionedObject debugging logs"  ${_DEBUG_LOG})
message(STATUS "OPTION_VERSIONEDOBJECT_debug_log=${OPTION_VERSIONEDOBJECT_debug_log}")
#option(OPTION_BIMAP_debug_log  "Set to ON for BiMap debugging logs"  ${_DEBUG_LOG})
#message(STATUS "OPTION_BIMAP_debug_log=${OPTION_BIMAP_debug_log}")
#[===========[  donot use generator-expressions in option() functions
# option(OPTION_VERSIONEDOBJECT_debug_log  "Set to ON for debugging logs"   "$<AND:$<CONFIG:Debug>,$<DATASTRUCTURE_STANDALONE_PROJECT>>")
#]===========]

if (DATASTRUCTURE_STANDALONE_PROJECT)
    dataStructure_set_cxx_standard()
endif()




file(TO_NATIVE_PATH "/" _path_separator)
message(STATUS "The systems's path separator is '${_path_separator}'")


#[===================================================================[
   dataStructure (header only) library
#]===================================================================]
add_library(dataStructure INTERFACE)
add_library(dataStructure::dataStructure ALIAS dataStructure)

fetch_dependencies()



dataStructure_check_cxx_compiler_flag_file_prefix_map()


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
    if(CMAKE_BUILD_TYPE MATCHES "MemCheck")
        find_program(MEMORYCHECK_COMMAND  NAMES valgrind REQUIRED)

        # Set Valgrind options for memcheck
        #set(CTEST_MEMORYCHECK_TYPE "Valgrind")    CTEST_* is meant to be for variables defined in 'memchec.cmake' and called so -> "ctest -S memcheck.cmake ...."
        #set(CTEST_MEMORYCHECK_COMMAND "valgrind --log-file=MemoryChecker.%p.log")
        #set(CTEST_MEMORYCHECK_COMMAND_OPTIONS ...) doesn't work
        #set(VALGRIND_COMMAND "valgrind --log-file=memCheck.%p.log")  # doesnot work
        #set(VALGRIND_COMMAND_OPTIONS "--leak-check=full --show-leak-kinds=all --track-origins=yes --verbose")     # call before `include(CTest)

        set(MEMORYCHECK_TYPE Valgrind)
        #set(MEMORYCHECK_COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/tests/valgrind_wrapper_memcheck.sh")
        set(MEMORYCHECK_COMMAND /usr/bin/valgrind)
        set(MEMORYCHECK_COMMAND_OPTIONS "--tool=memcheck --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose")     # call before `include(CTest)
        #set(MEMORYCHECK_SUPPRESSIONS_FILE "${PROJECT_SOURCE_DIR}/.valgrind-suppressions")  # must go before `include(CTest)`
    endif()

    if(CMAKE_BUILD_TYPE MATCHES "Coverage")
        find_program(COVERAGE_COMMAND  NAMES gcov REQUIRED)
        find_program(COVER_COMMAND  NAMES gcovr REQUIRED)
        find_program(LCOV_COMMAND  NAMES lcov REQUIRED)
        find_program(GENHTML_COMMAND  NAMES genhtml REQUIRED) # genhtml gets installed along with lcov, no need to install seperately
    endif()

    if(CMAKE_BUILD_TYPE MATCHES "CpuProf")
        find_program(CPUPROFILING_COMMAND  NAMES valgrind REQUIRED)

        set(MEMORYCHECK_TYPE Valgrind)
        set(MEMORYCHECK_COMMAND /usr/bin/valgrind)
        set(MEMORYCHECK_COMMAND_OPTIONS "--tool=callgrind --dump-instr=yes --collect-jumps=yes")     # call before `include(CTest)
    endif()

    if(CMAKE_BUILD_TYPE MATCHES "MemProf")
        find_program(MEMPROFILING_COMMAND  NAMES valgrind REQUIRED)

        set(MEMORYCHECK_TYPE Valgrind)
        set(MEMORYCHECK_COMMAND /usr/bin/valgrind)
        set(MEMORYCHECK_COMMAND_OPTIONS "--tool=massif")     # call before `include(CTest)
    endif()

    if(CMAKE_BUILD_TYPE MATCHES "CppCheck")
        find_program(CPPCHECK_COMMAND NAMES cppcheck REQUIRED)

        set(CPPCHECK_COMMAND_OPTIONS --enable=all --inline-suppr --quiet --std=c++20 # -i/home/vishnu/repositories/DataStructureLib/build-cppcheck-gpp/_deps/*
                                     --output-file=${PROJECT_BINARY_DIR}/cppcheck_staticAnalysis/cppcheck_output.txt
                                     --suppressions-list=${PROJECT_BINARY_DIR}/cppcheck_staticAnalysis/CppCheckSuppressions.txt)
        #set(CMAKE_C_CPPCHECK ${CPPCHECK_COMMAND} --std=c11 ${CPPCHECK_COMMAND_OPTIONS})
        set(CMAKE_CXX_CPPCHECK ${CPPCHECK_COMMAND} --std=c++20 ${CPPCHECK_COMMAND_OPTIONS})
    endif()




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

