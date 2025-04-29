#
# URL:      https://github.com/panchaBhuta/dataStructure
#
# Copyright (c) 2023-2023 Gautam Dhar
# All rights reserved.
#
# dataStructure is distributed under the BSD 3-Clause license, see LICENSE for details. 
#


# https://stackoverflow.com/questions/14446495/cmake-project-structure-with-unit-tests


# Test function add_unit_test
function(add_unit_test testname OPTION_ENABLE_METADATA)
  if(CMAKE_BUILD_TYPE MATCHES "Debug")
  #if(BUILD_TESTING)
    message(STATUS "### TEST ${testname}")
    FILE(GLOB sources ${testname}.cpp main.cpp  ${ARGN})
    add_executable(${testname} ${sources})
    #add_test(NAME ${testname} COMMAND "${PROJECT_BINARY_DIR}/tests/versionedObject/${testname}")
    add_test(NAME ${testname} COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/testRun.sh"   "${PROJECT_BINARY_DIR}/tests/versionedObject/${testname}")
    target_include_directories(${testname} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
    target_link_libraries(${testname} dataStructure) # donot specify PUBLIC here,
    target_compile_definitions(${testname} PRIVATE TEST_ENABLE_METADATA=$<BOOL:${OPTION_ENABLE_METADATA}>)
    # executable ${testname} has to have transitive association to all its dependencies(aka library chain)
    # refer   https://cmake.org/cmake/help/latest/manual/cmake-buildsystem.7.html#target-usage-requirements
    # refer   https://cmake.org/cmake/help/latest/command/target_link_libraries.html#libraries-for-a-target-and-or-its-dependents
    set_target_properties(${testname} PROPERTIES  LINKER_LANGUAGE CXX)
  endif()
endfunction(add_unit_test)

# Test macro add_perf_test
macro(add_perf_test testname OPTION_ENABLE_METADATA)
  if(CMAKE_BUILD_TYPE MATCHES "Release")
    message(STATUS "### TEST ${testname}")
    # https://cmake.org/cmake/help/latest/manual/cmake-generator-expressions.7.html#genex:condition
    FILE(GLOB sources ${testname}.cpp main.cpp  ${ARGN})
    add_executable(${testname} ${sources})
    add_test(NAME ${testname} COMMAND "${PROJECT_BINARY_DIR}/tests/versionedObject/${testname}")
    target_link_libraries(${testname} dataStructure) # donot specify PUBLIC here,
    target_compile_definitions(${testname} PRIVATE TEST_ENABLE_METADATA=$<BOOL:${OPTION_ENABLE_METADATA}>)
    # executable ${testname} has to have transitive association to all its dependencies(aka library chain)
    # refer   https://cmake.org/cmake/help/latest/manual/cmake-buildsystem.7.html#target-usage-requirements
    # refer   https://cmake.org/cmake/help/latest/command/target_link_libraries.html#libraries-for-a-target-and-or-its-dependents
  endif()
endmacro(add_perf_test)



# Unit tests
add_unit_test(testVersionObject  0)

add_unit_test(testBuildForwardTimelineNoMetaData_ChangeFullList                0  testForwardTimeLineCommon.cpp)
add_unit_test(testBuildForwardTimelineNoMetaData_ChangeSplitList               0  testForwardTimeLineCommon.cpp)

add_unit_test(testBuildReverseTimelineNoMetaData_ChangeFullList                0  testReverseTimeLineCommon.cpp)
add_unit_test(testBuildReverseTimelineNoMetaData_ChangeSplitList               0  testReverseTimeLineCommon.cpp)

add_unit_test(testBuildBidirectionTimelineNoMetaData_changeFullList_Forward    0  testForwardTimeLineCommon.cpp)
add_unit_test(testBuildBidirectionTimelineNoMetaData_changeSplitList_Forward   0  testForwardTimeLineCommon.cpp)

add_unit_test(testBuildBidirectionTimelineNoMetaData_changeFullList_Reverse    0  testReverseTimeLineCommon.cpp)
add_unit_test(testBuildBidirectionTimelineNoMetaData_changeSplitList_Reverse   0  testReverseTimeLineCommon.cpp)

add_unit_test(testBuildBidirectionTimelineNoMetaData_multiForward              0  testForwardTimeLineCommon.cpp)
add_unit_test(testBuildBidirectionTimelineNoMetaData_multiReverse_v1           0  testReverseTimeLineCommon.cpp)
add_unit_test(testBuildBidirectionTimelineNoMetaData_multiReverse_v2           0  testReverseTimeLineCommon.cpp)


add_unit_test(testBuildBidirectionTimelineNoMetaData_bidirection_DELISTED      0  testReverseTimeLineCommon.cpp)
add_unit_test(testBuildBidirectionTimelineNoMetaData_bidirection_RELISTED_v1   0  testReverseTimeLineCommon.cpp)
add_unit_test(testBuildBidirectionTimelineNoMetaData_bidirection_RELISTED_v2   0  testReverseTimeLineCommon.cpp)


add_unit_test(testPriorityMergeNoMetaData  0)
add_unit_test(test_caseInsensitive_companyName_BuildReverseTimelineNoMetaData  0)


#[=====[
add_unit_test(test_removeHistoricalDeltaVersionArtifacts_NoMetaData            testReverseTimeLineCommon.cpp)
#]=====]


add_unit_test(testBuildForwardTimelineMetaData_ChangeFullList                1  testForwardTimeLineCommon.cpp)
add_unit_test(testBuildForwardTimelineMetaData_ChangeSplitList               1  testForwardTimeLineCommon.cpp)

add_unit_test(testBuildReverseTimelineMetaData_ChangeFullList                1  testReverseTimeLineCommon.cpp)
add_unit_test(testBuildReverseTimelineMetaData_ChangeSplitList               1  testReverseTimeLineCommon.cpp)

add_unit_test(testBuildBidirectionTimelineMetaData_changeFullList_Forward    1  testForwardTimeLineCommon.cpp)

#[=====[
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


