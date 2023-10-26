#
# Copyright (c) 2023-2023 Gautam Dhar
# All rights reserved.
#
# versionedObject is private and NOT licensed for public use.
#


# https://stackoverflow.com/questions/14446495/cmake-project-structure-with-unit-tests


# Test macro add_unit_test
macro(add_unit_test testname)
  if(CMAKE_BUILD_TYPE MATCHES "Debug")
  #if(BUILD_TESTING)
    FILE(GLOB sources ${testname}*.cpp)
    add_executable(${testname} ${sources})
    add_test(NAME ${testname} COMMAND "${PROJECT_BINARY_DIR}/tests/${testname}")
    target_link_libraries(${testname} PUBLIC versionedObject)
    set_target_properties(${testname} PROPERTIES  LINKER_LANGUAGE CXX)
  endif()
endmacro(add_unit_test)

# Test macro add_perf_test
macro(add_perf_test testname)
  if(CMAKE_BUILD_TYPE MATCHES "Release")
    FILE(GLOB sources ${testname}*.cpp)
    add_executable(${testname} ${sources})
    add_test(NAME ${testname} COMMAND "${PROJECT_BINARY_DIR}/tests/${testname}")
    target_link_libraries(${testname} PUBLIC versionedObject)
  endif()
endmacro(add_perf_test)




# Unit tests
add_unit_test(testBasicTimeline)
#[===========================================[
add_unit_test(testAllTemplateInstantiation)
add_unit_test(testBasicConversions)
add_unit_test(testFloatingPointNaN)
add_unit_test(testIntegerNaN)

add_unit_test(testFloatingPointPrecision)
add_unit_test(testUserDefinedConverter_lowerPrecision)
add_unit_test(testUserDefinedConverter_locale)
add_unit_test(testFloatingPointLocale)

add_unit_test(testDateConversions)
add_unit_test(testDateConversionFormat)
add_unit_test(testDateConversionWindowsHeader)
add_unit_test(testTupleConversions)
#]===========================================]




#[======================[
# perf tests
add_perf_test(ptest001)
add_perf_test(ptest002)
#]======================]


