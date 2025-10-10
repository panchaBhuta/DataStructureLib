#
# URL:      https://github.com/panchaBhuta/dataStructure
#
# Copyright (c) 2023-2025 Gautam Dhar
# All rights reserved.
#
# dataStructure is distributed under the BSD 3-Clause license, see LICENSE for details. 
#


# https://stackoverflow.com/questions/14446495/cmake-project-structure-with-unit-tests

function(add_test_lib filename ENABLE_METADATA)
  SET(testLibName "${filename}_${ENABLE_METADATA}")
  add_library(${testLibName} OBJECT ${filename}.cpp)
  target_include_directories(${testLibName} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
  target_compile_definitions(${testLibName} PUBLIC TEST_ENABLE_METADATA=${ENABLE_METADATA}
        $<$<CONFIG:Debug>:DEBUG_BUILD>
        $<$<CONFIG:Release>:RELEASE_BUILD>
        FLAG_VERSIONEDOBJECT_debug_log=$<BOOL:${OPTION_VERSIONEDOBJECT_debug_log}>)
  target_link_libraries(${testLibName} dataStructure) # donot specify PUBLIC here,
endfunction(add_test_lib)

add_test_lib(main 0)
add_test_lib(main 1)
add_test_lib(testForwardTimeLineCommon 0)
add_test_lib(testForwardTimeLineCommon 1)
add_test_lib(testReverseTimeLineCommon 0)
add_test_lib(testReverseTimeLineCommon 1)

# Test function add_unit_test
function(add_unit_test testname OPTION_ENABLE_METADATA)
  if(CMAKE_BUILD_TYPE MATCHES "Debug"     OR
     CMAKE_BUILD_TYPE MATCHES "MemCheck"  OR
     CMAKE_BUILD_TYPE MATCHES "Coverage"  OR
     CMAKE_BUILD_TYPE MATCHES "CpuProf"   OR
     CMAKE_BUILD_TYPE MATCHES "MemProf"   OR
     CMAKE_BUILD_TYPE MATCHES "CppCheck")
  #if(BUILD_TESTING)

    FILE(GLOB sourcesExe ${testname}.cpp)
    SET(sourcesFile main.cpp ${ARGN})   # ARGN here contains list of additional files if any
    LIST(TRANSFORM sourcesFile REPLACE [[\.cpp]] "_${OPTION_ENABLE_METADATA}"   OUTPUT_VARIABLE binaryLibs)
    message(STATUS "### TEST ${testname} :: sourcesFile[${sourcesFile}] -> libs[${binaryLibs}]")
    add_executable(${testname} ${sourcesExe})
    if (CMAKE_SYSTEM_NAME STREQUAL "Linux"   AND
        CMAKE_BUILD_TYPE MATCHES   "Debug")
      # Linux-specific stuff
      add_test(NAME ${testname} COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/testRun.sh"   "${PROJECT_BINARY_DIR}/tests/versionedObject/${testname}")
    else()
      add_test(NAME ${testname} COMMAND "${PROJECT_BINARY_DIR}/tests/versionedObject/${testname}")
    endif ()

    if(CMAKE_BUILD_TYPE MATCHES "MemCheck"  OR
       CMAKE_BUILD_TYPE MATCHES "CpuProf"   OR
       CMAKE_BUILD_TYPE MATCHES "MemProf")
      # https://stackoverflow.com/questions/44102050/valgrind-leading-to-numeric-issues-with-long-doubles
      #
      target_compile_definitions(${testname} PRIVATE USE_VALGRIND)
    endif()
    #target_compile_definitions(${testname}
    #            gives error as COMPILE_LANG_AND_ID is empty
    #            $<$<COMPILE_LANG_AND_ID:CXX,ARMClang,AppleClang,Clang>:COMPILING_CXX_WITH_CLANG>
    #            $<$<COMPILE_LANG_AND_ID:CXX,Intel>:COMPILING_CXX_WITH_INTEL>)

    target_include_directories(${testname} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
    target_link_libraries(${testname} PRIVATE dataStructure ${binaryLibs}) # donot specify PUBLIC here,
    target_compile_definitions(${testname}    PRIVATE TEST_ENABLE_METADATA=$<BOOL:${OPTION_ENABLE_METADATA}>)
    # executable ${testname} has to have transitive association to all its dependencies(aka library chain)
    # refer   https://cmake.org/cmake/help/latest/manual/cmake-buildsystem.7.html#target-usage-requirements
    # refer   https://cmake.org/cmake/help/latest/command/target_link_libraries.html#libraries-for-a-target-and-or-its-dependents


    if(CMAKE_BUILD_TYPE MATCHES "Coverage")
      #refer   https://github.com/bilke/cmake-modules/blob/master/CodeCoverage.cmake
      # compile with coverage options
      target_compile_options(${testname} PRIVATE --coverage -O0)
      target_link_options(${testname} PRIVATE --coverage)

      # How to use gcovr for code coverage?
      # If you have multiple compilers installed or if you are using Clang,
      # you will likely need to tell gcovr which gcov executable to use.
      # By default, gcovr just uses the program named gcov .
      # This is fine for the default GCC compiler, e.g. gcc or g++ .
      # Otherwise, you must use the --gcov-executable to tell gcovr what to use.

      #  doesn't  work as expected, instead directly calling gcovr in make.sh
      # add coverage target
#      add_custom_target(coverage_${testname}
        # gather data
#        COMMAND ${LCOV_COMMAND} --directory . --capture --output-file coverage.info
        # generate report
#        COMMAND ${GENHTML_COMMAND} --demangle-cpp -o coverage coverage.info
#        WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

    endif()

    set_target_properties(${testname} PROPERTIES  LINKER_LANGUAGE CXX)
  endif()
endfunction(add_unit_test)

#[======================[
# Test macro add_perf_test
macro(add_perf_test testname OPTION_ENABLE_METADATA)
  if(CMAKE_BUILD_TYPE MATCHES "Release")
    message(STATUS "### TEST ${testname}")
    # https://cmake.org/cmake/help/latest/manual/cmake-generator-expressions.7.html#genex:condition
    FILE(GLOB sourcesExe ${testname}.cpp)
    FILE(GLOB sourcesLib main.cpp  ${ARGN})
    add_executable(${testname} ${sourcesExe})
    LIST(JOIN ${sourcesLib} "_" testlibname)
    SET(testlibname "${testlibname}_${OPTION_ENABLE_METADATA}")
    add_library(${testlibname} STATIC ${sourcesLib})
    add_test(NAME ${testname} COMMAND "${PROJECT_BINARY_DIR}/tests/versionedObject/${testname}")
    target_include_directories(${testname} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
    target_include_directories(${testlibname} INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
    target_link_libraries(${testname} dataStructure ${testlibname}) # donot specify PUBLIC here,
    target_compile_definitions(${testname}    PRIVATE TEST_ENABLE_METADATA=$<BOOL:${OPTION_ENABLE_METADATA}>)
    target_compile_definitions(${testlibname} PUBLIC  TEST_ENABLE_METADATA=$<BOOL:${OPTION_ENABLE_METADATA}>)
    # executable ${testname} has to have transitive association to all its dependencies(aka library chain)
    # refer   https://cmake.org/cmake/help/latest/manual/cmake-buildsystem.7.html#target-usage-requirements
    # refer   https://cmake.org/cmake/help/latest/command/target_link_libraries.html#libraries-for-a-target-and-or-its-dependents
    set_target_properties(${testname} PROPERTIES  LINKER_LANGUAGE CXX)
    set_target_properties(${testlibname} PROPERTIES  LINKER_LANGUAGE CXX)
  endif()
endmacro(add_perf_test)
#]======================]



# Unit tests
add_unit_test(testVersionObject  0)
add_unit_test(test_caseInsensitive_companyName_BuildReverseTimelineNoMetaData  0)


add_unit_test(testBuildForwardTimelineNoMetaData_ChangeFullList                0  testForwardTimeLineCommon.cpp)
add_unit_test(testBuildForwardTimelineNoMetaData_ChangeSplitList               0  testForwardTimeLineCommon.cpp)

add_unit_test(testBuildReverseTimelineNoMetaData_ChangeFullList                0  testReverseTimeLineCommon.cpp)
add_unit_test(testBuildReverseTimelineNoMetaData_ChangeSplitList               0  testReverseTimeLineCommon.cpp)

add_unit_test(testBuildBidirectionTimelineNoMetaData_changeFullList_Forward    0  testForwardTimeLineCommon.cpp)
add_unit_test(testBuildBidirectionTimelineNoMetaData_changeSplitList_Forward   0  testForwardTimeLineCommon.cpp)

add_unit_test(testBuildBidirectionTimelineNoMetaData_changeFullList_Reverse    0  testReverseTimeLineCommon.cpp)
add_unit_test(testBuildBidirectionTimelineNoMetaData_changeSplitList_Reverse   0  testReverseTimeLineCommon.cpp)

add_unit_test(testBuildBidirectionTimelineNoMetaData_reverse_FirstDeltaChangeBeforeStartDate   0)   # error testing

add_unit_test(testBuildBidirectionTimelineNoMetaData_multiForward              0  testForwardTimeLineCommon.cpp)
add_unit_test(testBuildBidirectionTimelineNoMetaData_multiReverse_v1           0  testReverseTimeLineCommon.cpp)
add_unit_test(testBuildBidirectionTimelineNoMetaData_multiReverse_v2           0  testReverseTimeLineCommon.cpp)


add_unit_test(testBuildBidirectionTimelineNoMetaData_bidirection_DELISTED      0  testReverseTimeLineCommon.cpp)
add_unit_test(testBuildBidirectionTimelineNoMetaData_bidirection_RELISTED_v1   0  testReverseTimeLineCommon.cpp)
add_unit_test(testBuildBidirectionTimelineNoMetaData_bidirection_RELISTED_v2   0  testReverseTimeLineCommon.cpp)


add_unit_test(testPriorityMergeNoMetaData  0)


#[=====[
add_unit_test(test_removeHistoricalDeltaVersionArtifacts_NoMetaData            testReverseTimeLineCommon.cpp)
#]=====]


add_unit_test(testBuildForwardTimelineMetaData_ChangeFullList                1  testForwardTimeLineCommon.cpp)
add_unit_test(testBuildForwardTimelineMetaData_ChangeSplitList               1  testForwardTimeLineCommon.cpp)

add_unit_test(testBuildReverseTimelineMetaData_ChangeFullList                1  testReverseTimeLineCommon.cpp)
add_unit_test(testBuildReverseTimelineMetaData_ChangeSplitList               1  testReverseTimeLineCommon.cpp)

add_unit_test(testBuildBidirectionTimelineMetaData_changeFullList_Forward    1  testForwardTimeLineCommon.cpp)
add_unit_test(testBuildBidirectionTimelineMetaData_changeSplitList_Forward   1  testForwardTimeLineCommon.cpp)

add_unit_test(testBuildBidirectionTimelineMetaData_changeFullList_Reverse    1  testReverseTimeLineCommon.cpp)
add_unit_test(testBuildBidirectionTimelineMetaData_changeSplitList_Reverse   1  testReverseTimeLineCommon.cpp)

add_unit_test(testBuildBidirectionTimelineMetaData_reverse_FirstDeltaChangeBeforeStartDate   1)   # error testing

add_unit_test(testBuildBidirectionTimelineMetaData_multiForward              1  testForwardTimeLineCommon.cpp)
add_unit_test(testBuildBidirectionTimelineMetaData_multiReverse_v1           1  testReverseTimeLineCommon.cpp)
add_unit_test(testBuildBidirectionTimelineMetaData_multiReverse_v2           1  testReverseTimeLineCommon.cpp)


add_unit_test(testBuildBidirectionTimelineMetaData_bidirection_DELISTED      1  testReverseTimeLineCommon.cpp)
add_unit_test(testBuildBidirectionTimelineMetaData_bidirection_RELISTED_v1   1  testReverseTimeLineCommon.cpp)
add_unit_test(testBuildBidirectionTimelineMetaData_bidirection_RELISTED_v2   1  testReverseTimeLineCommon.cpp)


add_unit_test(testPriorityMergeMetaData  1)




#[======================[
# perf tests
add_perf_test(ptest001)
add_perf_test(ptest002)
#]======================]


