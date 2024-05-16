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
macro(add_unit_test testname)
  if(CMAKE_BUILD_TYPE MATCHES "Debug")
  #if(BUILD_TESTING)
    FILE(GLOB sources ${testname}*.cpp)
    add_executable(${testname} ${sources})
    add_test(NAME ${testname} COMMAND "${PROJECT_BINARY_DIR}/tests/versionedObject/${testname}")
    target_link_libraries(${testname} dataStructure) # donot specify PUBLIC here,
    # executable ${testname} has to have transitive association to all its dependencies(aka library chain)
    # refer   https://cmake.org/cmake/help/latest/manual/cmake-buildsystem.7.html#target-usage-requirements
    # refer   https://cmake.org/cmake/help/latest/command/target_link_libraries.html#libraries-for-a-target-and-or-its-dependents
    # refer   https://cmake.org/cmake/help/latest/command/target_link_libraries.html#libraries-for-a-target-and-or-its-dependents
    set_target_properties(${testname} PROPERTIES  LINKER_LANGUAGE CXX)
  endif()
endmacro(add_unit_test)

# Test macro add_perf_test
macro(add_perf_test testname)
  if(CMAKE_BUILD_TYPE MATCHES "Release")
    FILE(GLOB sources ${testname}*.cpp)
    add_executable(${testname} ${sources})
    add_test(NAME ${testname} COMMAND "${PROJECT_BINARY_DIR}/tests/versionedObject/${testname}")
    target_link_libraries(${testname} dataStructure) # donot specify PUBLIC here,
    # executable ${testname} has to have transitive association to all its dependencies(aka library chain)
    # refer   https://cmake.org/cmake/help/latest/manual/cmake-buildsystem.7.html#target-usage-requirements
    # refer   https://cmake.org/cmake/help/latest/command/target_link_libraries.html#libraries-for-a-target-and-or-its-dependents
    # refer   https://cmake.org/cmake/help/latest/command/target_link_libraries.html#libraries-for-a-target-and-or-its-dependents
  endif()
endmacro(add_perf_test)




# Unit tests
add_unit_test(testBuildForwardTimelineMetaData)
add_unit_test(testBuildReverseTimelineMetaData)
add_unit_test(testBuildBidirectionTimelineMetaData_forward)  # on test failure cross check with test_VO_BuildForwardTimelineMetaData
add_unit_test(testBuildBidirectionTimelineMetaData_reverse)  # on test failure cross check with test_VO_BuildReverseTimelineMetaData
add_unit_test(testBuildBidirectionTimelineMetaData_bidirection_DELISTED)  # on test failure cross check with test_VO_BuildReverseTimelineMetaData
add_unit_test(testBuildBidirectionTimelineMetaData_bidirection_RELISTED_1)
add_unit_test(testBuildBidirectionTimelineMetaData_bidirection_RELISTED_2)
add_unit_test(test_VO_BuildForwardTimelineMetaData)
add_unit_test(test_VO_BuildReverseTimelineMetaData)
add_unit_test(testPriorityMergeMetaData)

add_unit_test(testBuildForwardTimelineNoMetaData)
add_unit_test(testBuildReverseTimelineNoMetaData)
add_unit_test(testBuildBidirectionTimelineNoMetaData_forward)  # on test failure cross check with test_VO_BuildForwardTimelineMetaData
add_unit_test(testBuildBidirectionTimelineNoMetaData_reverse)  # on test failure cross check with test_VO_BuildReverseTimelineMetaData
add_unit_test(testBuildBidirectionTimelineNoMetaData_bidirection_DELISTED)  # on test failure cross check with test_VO_BuildReverseTimelineMetaData
add_unit_test(testBuildBidirectionTimelineNoMetaData_bidirection_RELISTED_1)
add_unit_test(testBuildBidirectionTimelineNoMetaData_bidirection_RELISTED_2)
add_unit_test(test_VO_BuildForwardTimelineNoMetaData)
add_unit_test(test_VO_BuildReverseTimelineNoMetaData)
add_unit_test(testPriorityMergeNoMetaData)


add_unit_test(test_caseInsensitive_companyName_BuildReverseTimelineNoMetaData)


#[======================[
# perf tests
add_perf_test(ptest001)
add_perf_test(ptest002)
#]======================]


