#
# URL:      https://github.com/panchaBhuta/dataStructure
#
# Copyright (c) 2023-2023 Gautam Dhar
# All rights reserved.
#
# dataStructure is distributed under the BSD 3-Clause license, see LICENSE for details. 
#


# https://stackoverflow.com/questions/14446495/cmake-project-structure-with-unit-tests


# Test macro add_unit_test
function(add_unit_test testname)
  # https://cmake.org/cmake/help/latest/command/cmake_parse_arguments.html#command:cmake_parse_arguments
  set(options SIMPLE_COMPILE)
  set(oneValueArgs "")
  set(multiValueArgs "")
  cmake_parse_arguments(arg_add_unit_test "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
  if(CMAKE_BUILD_TYPE MATCHES "Debug")
  #if(BUILD_TESTING)
    message(STATUS "### TEST ${testname}")
    # https://cmake.org/cmake/help/latest/manual/cmake-generator-expressions.7.html#genex:condition
    FILE(GLOB sources ${testname}*.cpp main.cpp $<$<NOT:arg_add_unit_test_SIMPLE_COMPILE>:"testHelper.cpp">)
    add_executable(${testname} ${sources})
    #add_test(NAME ${testname} COMMAND "${PROJECT_BINARY_DIR}/tests/versionedObject/${testname}")
    add_test(NAME ${testname} COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/testRun.sh"   "${PROJECT_BINARY_DIR}/tests/versionedObject/${testname}")
    target_include_directories(${testname} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
    target_link_libraries(${testname} dataStructure) # donot specify PUBLIC here,
    # executable ${testname} has to have transitive association to all its dependencies(aka library chain)
    # refer   https://cmake.org/cmake/help/latest/manual/cmake-buildsystem.7.html#target-usage-requirements
    # refer   https://cmake.org/cmake/help/latest/command/target_link_libraries.html#libraries-for-a-target-and-or-its-dependents
    set_target_properties(${testname} PROPERTIES  LINKER_LANGUAGE CXX)
  endif()
endfunction(add_unit_test)

# Test macro add_perf_test
macro(add_perf_test testname)
  if(CMAKE_BUILD_TYPE MATCHES "Release")
    message(STATUS "### TEST ${testname}")
    # https://cmake.org/cmake/help/latest/manual/cmake-generator-expressions.7.html#genex:condition
    FILE(GLOB sources ${testname}*.cpp main.cpp )
    add_executable(${testname} ${sources})
    add_test(NAME ${testname} COMMAND "${PROJECT_BINARY_DIR}/tests/versionedObject/${testname}")
    target_link_libraries(${testname} dataStructure) # donot specify PUBLIC here,
    # executable ${testname} has to have transitive association to all its dependencies(aka library chain)
    # refer   https://cmake.org/cmake/help/latest/manual/cmake-buildsystem.7.html#target-usage-requirements
    # refer   https://cmake.org/cmake/help/latest/command/target_link_libraries.html#libraries-for-a-target-and-or-its-dependents
  endif()
endmacro(add_perf_test)




# Unit tests
add_unit_test(testVersionObject SIMPLE_COMPILE)


#[=====[
add_unit_test(testBuildForwardTimelineNoMetaData_ChangeFullList)
add_unit_test(testBuildForwardTimelineNoMetaData_ChangeSplitList)

add_unit_test(testBuildReverseTimelineNoMetaData_ChangeFullList)
add_unit_test(testBuildReverseTimelineNoMetaData_ChangeSplitList)

add_unit_test(testBuildBidirectionTimelineNoMetaData_changeFullList_Forward)
add_unit_test(testBuildBidirectionTimelineNoMetaData_changeSplitList_Forward)


add_unit_test(testBuildBidirectionTimelineNoMetaData_changeFullList_Reverse)
add_unit_test(testBuildBidirectionTimelineNoMetaData_changeSplitList_Reverse)


add_unit_test(testBuildBidirectionTimelineNoMetaData_bidirection_DELISTED)

add_unit_test(testBuildBidirectionTimelineNoMetaData_bidirection_RELISTED_1)
add_unit_test(testBuildBidirectionTimelineNoMetaData_bidirection_RELISTED_2)
add_unit_test(testPriorityMergeNoMetaData)

add_unit_test(test_removeHistoricalDeltaVersionArtifacts)

add_unit_test(test_caseInsensitive_companyName_BuildReverseTimelineNoMetaData)
#]=====]


#[=====[
add_unit_test(testBuildForwardTimelineMetaData)
add_unit_test(testBuildReverseTimelineMetaData)
add_unit_test(testBuildBidirectionTimelineMetaData_forward)  # on test failure cross check with testBuildForwardTimelineMetaData
add_unit_test(testBuildBidirectionTimelineMetaData_reverse)  # on test failure cross check with testBuildReverseTimelineMetaData
add_unit_test(testBuildBidirectionTimelineMetaData_bidirection_DELISTED)  # on test failure cross check with testBuildReverseTimelineMetaData
add_unit_test(testBuildBidirectionTimelineMetaData_bidirection_RELISTED_1)
add_unit_test(testBuildBidirectionTimelineMetaData_bidirection_RELISTED_2)
add_unit_test(testPriorityMergeMetaData)
#]=====]



#[======================[
# perf tests
add_perf_test(ptest001)
add_perf_test(ptest002)
#]======================]


