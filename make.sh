#!/usr/bin/env bash

# make.sh
#
# Copyright (C) 2023-2025 Gautam Dhar
# All rights reserved.
#
# See LICENSE for redistribution information.

# exiterr
exiterr()
{
  >&2 echo "${1}"
  exit 1
}



#############  build config parameters    START

##   https://stackoverflow.com/questions/74123491/cmake-alternate-option-to-fetchcontent-when-the-source-file-already-exists-loc
LOCAL_DATELIB=""                                                   # when using github/date
#LOCAL_DATELIB="-DFETCHCONTENT_SOURCE_DIR_DATE=$(pwd)/../date"       # when using local source

LOCAL_CONVERTERLIB=""                                                           # when using github/converter
#LOCAL_CONVERTERLIB="-DFETCHCONTENT_SOURCE_DIR_CONVERTER=$(pwd)/../converter"     # when using local source


CXX_COMPILER=g++
#CXX_COMPILER=clang++
# on clang++ error "/usr/bin/ld: cannot find -lstdc++: No such file or directory"
# $ clang++ --verbose
# Ubuntu clang version 18.1.3 (1ubuntu1)
# Target: x86_64-pc-linux-gnu
# Thread model: posix
# InstalledDir: /usr/bin
# Found candidate GCC installation: /usr/bin/../lib/gcc/x86_64-linux-gnu/13
# Found candidate GCC installation: /usr/bin/../lib/gcc/x86_64-linux-gnu/14
# Selected GCC installation: /usr/bin/../lib/gcc/x86_64-linux-gnu/14          <<<<<<<<<<<<   install this
# Candidate multilib: .;@m64
# Selected multilib: .;@m64
#
# $ sudo apt install libstdc++-14-dev

#############  build config parameters    END



# process arguments
DEPS="0"
INCLUDE="0"
BUILD="0"
TESTS="0"
MEMCHECK="0"
COVERAGE="0"
CPUPROF="0"
MEMPROF="0"
CPPCHECK="0"
DOC="0"
INSTALL="0"
case "${1%/}" in
  deps)
    DEPS="1"
    ;;

  include)
    INCLUDE="1"
    ;;

  build)
    BUILD="1"
    ;;

  tests)
    BUILD="1"
    TESTS="1"
    ;;

  memCheck)
    MEMCHECK="1"
    ;;

  coverage)
    COVERAGE="1"
    ;;

  cpuProf)
    CPUPROF="1"
    ;;

  memProf)
    MEMPROF="1"
    ;;

  cppCheck)
    CPPCHECK="1"
    ;;

  doc)
    DOC="1"
    ;;

  install)
    BUILD="1"
    INSTALL="1"
    ;;

  all)
    DEPS="1"
    INCLUDE="1"
    BUILD="1"
    TESTS="1"
    MEMCHECK="1"
    COVERAGE="1"
    CPUPROF="1"
    MEMPROF="1"
    CPPCHECK="1"
    DOC="1"
    INSTALL="1"
    ;;

  *)
    echo "usage: make.sh <deps|include|build|tests|memCheck|coverage|cpuProf|memProf|cppCheck|doc|install|all>"
    echo "  deps                - install project dependencies"
    echo "  include             - reformat source code"
    echo "  build               - perform build"
    echo "  tests    [testName] - perform build and run tests. optional [testName] to run a single test."
    echo "  memCheck [testName] - perform  memory-check tests. optional [testName] to run a single test."
    echo "  coverage [testName] - perform code-coverage tests. optional [testName] to run a single test."
    echo "  cpuProf  [testName] - perform    cpu-profiling on tests. optional [testName] to run a single test."
    echo "  memProf  [testName] - perform memory-profiling on tests. optional [testName] to run a single test."
    echo "  cppCheck            - perform static analysis for C/C++ code."
    echo "  doc                 - perform build and generate documentation"
    echo "  install             - perform build and install"
    echo "  all                 - perform all actions above"
	echo ""
	echo "macro LOCAL_DATELIB            - can be set/unset to use local-repo/github:date"
	echo "macro LOCAL_CONVERTERLIB       - can be set/unset to use local-repo/github:converter"
	echo "macro CXX_COMPILER             - can be used to select between g++ OR clang++ compiler"
    exit 1
    ;;
esac

# deps
if [[ "${DEPS}" == "1" ]]; then
  OS="$(uname)"
  if [ "${OS}" == "Linux" ]; then
    DISTRO="$(lsb_release -i | awk -F':\t' '{print $2}')"
    if [[ "${DISTRO}" == "Ubuntu" ]]; then
      if [[ "${GITHUB_ACTIONS}" == "true" ]]; then
        # ensure de_DE locale is present when running CI tests
        locale -a | grep -qi "de_DE"
        if [[ "${?}" != "0" ]]; then
          sudo locale-gen "de_DE" || exiterr "deps failed (${DISTRO}), exiting."
        fi
        locale -a | grep -qi "de_DE.utf8"
        if [[ "${?}" != "0" ]]; then
          sudo locale-gen "de_DE.UTF-8" || exiterr "deps failed (${DISTRO}), exiting."
        fi
      fi
    else
      exiterr "deps failed (unsupported linux distro ${DISTRO}), exiting."
    fi
  elif [ "${OS}" == "Darwin" ]; then
    true || exiterr "deps failed (mac), exiting."
  else
    exiterr "deps failed (unsupported os ${OS}), exiting."
  fi
fi

# include
if [[ "${INCLUDE}" == "1" ]]; then
  if [[ -x "$(command -v uncrustify)" ]]; then
    TMPDIR=$(mktemp -d)
    for SRC in examples/*.cpp; do
      DST="${TMPDIR}/$(basename ${SRC})"
      printf "\n\n\n\n\n\n"  > ${DST}         # add 6 blank lines
      tail -n +8 ${SRC}     >> ${DST}         # skip header (first 7 lines) 
      head -7 ${SRC}         > ${DST}.header  # store header separately (first 7 lines)
    done
    
    uncrustify -c uncrustify.cfg --replace --no-backup include/versionedObject/*.h tests/*.cpp tests/*.h ${TMPDIR}/*.cpp
    if [[ "${?}" != "0" ]]; then
      rm -rf ${TMPDIR}
      echo "include failed, exiting."
      exit 1
    fi

    for DST in examples/*.cpp; do
      SRC="${TMPDIR}/$(basename ${DST})"
      cat ${SRC}.header      > ${DST}.tmp
      cat ${SRC}            >> ${DST}.tmp

      cmp --silent ${DST} ${DST}.tmp
      if [[ "${?}" != "0" ]]; then
        cat ${DST}.tmp > ${DST}
      fi
      rm ${DST}.tmp
    done

    rm -rf ${TMPDIR}
  fi
fi

OS="$(uname)"
PARALLEL_PROCESS_ARGS=""
if [ "${OS}" == "Linux" ]; then
  PARALLEL_PROCESS_ARGS="-j$(nproc)"
elif [ "${OS}" == "Darwin" ]; then
  PARALLEL_PROCESS_ARGS="-j$(sysctl -n hw.ncpu)"
fi


BUILD_DEBUG_DIR="build-debug-$(echo ${CXX_COMPILER} | sed 's/+/p/g')"
BUILD_RELEASE_DIR="build-release-$(echo ${CXX_COMPILER} | sed 's/+/p/g')"
BUILD_MEMCHECK_DIR="build-memcheck-$(echo ${CXX_COMPILER} | sed 's/+/p/g')"
BUILD_COVERAGE_DIR="build-coverage-$(echo ${CXX_COMPILER} | sed 's/+/p/g')"
BUILD_CPUPROF_DIR="build-cpuprof-$(echo ${CXX_COMPILER} | sed 's/+/p/g')"
BUILD_MEMPROF_DIR="build-memprof-$(echo ${CXX_COMPILER} | sed 's/+/p/g')"
BUILD_CPPCHECK_DIR="build-cppcheck-$(echo ${CXX_COMPILER} | sed 's/+/p/g')"

SOURCE_DIR="$(pwd)"

# build
if [[ "${BUILD}" == "1" ]]; then
  mkdir -p ${BUILD_DEBUG_DIR} && cd ${BUILD_DEBUG_DIR} && echo "##### START(debug) : processing/configure of CMakeLists.txt #####" && \
  cmake -DDATASTRUCTURE_BUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER=${CXX_COMPILER}  ${LOCAL_DATELIB} ${LOCAL_CONVERTERLIB} .. && \
  echo "##### START(debug) : project build #####" && cmake --build . ${PARALLEL_PROCESS_ARGS} && cd .. && \
  mkdir -p ${BUILD_RELEASE_DIR} && cd ${BUILD_RELEASE_DIR} && echo "##### START(release) : processing/configure of CMakeLists.txt #####" && \
  cmake -DDATASTRUCTURE_BUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=${CXX_COMPILER}  ${LOCAL_DATELIB} ${LOCAL_CONVERTERLIB} .. && \
  echo "##### START(release) : project build #####" && cmake --build . ${PARALLEL_PROCESS_ARGS} && cd .. || \
  exiterr "build failed, exiting."
fi

#  https://stackoverflow.com/questions/28678505/add-command-arguments-using-inline-if-statement-in-bash/28678587#28678587
OPTIONAL_TEST_NAME=$( (( $# == 2 )) && printf "%s %s" "-R" "${2}" )

# tests
if [[ "${TESTS}" == "1" ]]; then
  cd ${BUILD_DEBUG_DIR} && echo "##### START(debug) : project Test #####" && \
  ctest --verbose ${PARALLEL_PROCESS_ARGS} --test-action Test ${OPTIONAL_TEST_NAME} && cd .. && \
  cd ${BUILD_RELEASE_DIR} && echo "##### START(release) : project Test #####" && \
  ctest --verbose ${PARALLEL_PROCESS_ARGS} --test-action Test ${OPTIONAL_TEST_NAME} && cd .. || \
  exiterr "tests failed, exiting."
fi

#memory-check
if [[ "${MEMCHECK}" == "1" ]]; then
  mkdir -p ${BUILD_MEMCHECK_DIR} && cd ${BUILD_MEMCHECK_DIR} && \
  echo "##### START(memCheck) : processing/configure of CMakeLists.txt #####" && \
  cmake -DDATASTRUCTURE_BUILD_TESTS=ON -DCMAKE_BUILD_TYPE=MemCheck -DCMAKE_CXX_COMPILER=${CXX_COMPILER}  ${LOCAL_DATELIB} ${LOCAL_CONVERTERLIB} .. && \
  echo "##### START(memCheck) : project build #####" && cmake --build . ${PARALLEL_PROCESS_ARGS} || \
  exiterr "build failed, exiting."

  echo "##### START(memCheck) : project Test #####" && \
  ctest --verbose ${PARALLEL_PROCESS_ARGS} --test-action memcheck ${OPTIONAL_TEST_NAME} || \
  exiterr "memCheck tests failed, exiting."

  mkdir -p ./Testing/MemoryChecker/
  #find ./Testing/Temporary/ -type f -regex '.*/MemoryChecker\.[0-9]+\.log$'
  for FILE in $(find ./Testing/Temporary/ -type f -regex '.*/MemoryChecker\.[0-9]+\.log$'); do
    #echo ">>>>>>>>>>>>>>>>>>>  ${FILE}"
    TEST_NAME=$(grep -E "==[0-9]+== Command: .*" ${FILE} | sed 's|.*/\([^/]\+\)$|\1|g')
    #echo "<<<<<<<<<<<<<<<<<<<  ${TEST_NAME}"
    cp ${FILE} ./Testing/MemoryChecker/${TEST_NAME}.log
  done

  cd ..
  echo "-----------------------------------"
  echo "report-summary of 'total heap usage'"
  find ./${BUILD_MEMCHECK_DIR}/Testing/MemoryChecker/ -name "*.log" -exec grep -Hn "total heap usage" {} \;
  echo ""
  echo "#### check for  allocs != frees  ####"
  find ./${BUILD_MEMCHECK_DIR}/Testing/MemoryChecker/ -name "*.log" -exec grep -Hn "total heap usage" {} \; | grep -v -E "total heap usage: ([0-9,]+) allocs, \1 frees,"
  echo "-----------------------------------"
  echo ""
  echo ""
  echo "==================================="
  echo "report-summary of 'ERROR'"
  find ./${BUILD_MEMCHECK_DIR}/Testing/MemoryChecker/ -name "*.log" -exec grep -Hn "ERROR" {} \;
  echo ""
  echo "#### check for ERROR > 1 ####"
  find ./${BUILD_MEMCHECK_DIR}/Testing/MemoryChecker/ -name "*.log" -exec grep -Hn "ERROR" {} \; | grep -v "ERROR SUMMARY: 0 errors"
  echo "==================================="
  echo ""
  echo ""
  echo "+++++++++++++++++++++++++++++++++++"
  echo "report-summary of 'WARNING'"
  find ./${BUILD_MEMCHECK_DIR}/Testing/MemoryChecker/ -name "*.log" -exec grep -Hn "WARNING" {} \;
  echo ""
  echo "#### check for WARNING not ignored ####"
  find ./${BUILD_MEMCHECK_DIR}/Testing/MemoryChecker/ -name "*.log" -exec grep -Hn "WARNING" {} \; | grep -v -e '-- ignoring it$'
  echo "+++++++++++++++++++++++++++++++++++"
fi

#coverage
if [[ "${COVERAGE}" == "1" ]]; then
  mkdir -p ${BUILD_COVERAGE_DIR}
  # Wipe out any previous coverage information
  find ${BUILD_COVERAGE_DIR} -name '*.gcda' -delete

  cd ${BUILD_COVERAGE_DIR}

  echo "##### START(coverage) : processing/configure of CMakeLists.txt #####" && \
  cmake -DDATASTRUCTURE_BUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Coverage -DCMAKE_CXX_COMPILER=${CXX_COMPILER}  ${LOCAL_DATELIB} ${LOCAL_CONVERTERLIB} .. && \
  echo "##### START(coverage) : project build #####" && cmake --build . ${PARALLEL_PROCESS_ARGS} || \
  exiterr "build failed, exiting."

  # --test-action coverage : even if it executes, incorrectly shows for all tests 0% coverage
  echo "##### START(coverage) : project Test #####" && \
  ctest --verbose --test-action Test ${OPTIONAL_TEST_NAME}   || \
  exiterr "coverage tests failed, exiting."

# && \
#  echo "##### START(coverage) : project Coverage #####" && \
#  ctest --verbose --test-action Coverage ${OPTIONAL_TEST_NAME}  || \  doesn't work as expected
#  exiterr "coverage tests failed, exiting."

  cd ..
  mkdir -p ${BUILD_COVERAGE_DIR}/codeCoverage
  echo "##### START(coverage) : Generate the coverage report #####"
  gcovr -r .  --exclude ${BUILD_COVERAGE_DIR}/_deps/ --exclude tests --html --html-details ${BUILD_COVERAGE_DIR}/codeCoverage/index-coverage.html  && \
  echo "" && echo "" && echo "code-coverage report : $(pwd)/${BUILD_COVERAGE_DIR}/codeCoverage/index-coverage.html" || \
  exiterr "code-coverage failed, exiting."
fi

#cpu-profiling
if [[ "${CPUPROF}" == "1" ]]; then
  mkdir -p ${BUILD_CPUPROF_DIR} && cd ${BUILD_CPUPROF_DIR} && \
  echo "##### START(cpuProf) : processing/configure of CMakeLists.txt #####" && \
  cmake -DDATASTRUCTURE_BUILD_TESTS=ON -DCMAKE_BUILD_TYPE=CpuProf -DCMAKE_CXX_COMPILER=${CXX_COMPILER}  ${LOCAL_DATELIB} ${LOCAL_CONVERTERLIB} .. && \
  echo "##### START(cpuProf) : project build #####" && cmake --build . ${PARALLEL_PROCESS_ARGS} || \
  exiterr "build failed, exiting."

  echo "##### START(cpuProf) : project Test #####" && \
  ctest --verbose ${PARALLEL_PROCESS_ARGS} --test-action memcheck ${OPTIONAL_TEST_NAME} || \
  exiterr "memCheck tests failed, exiting."

  mkdir -p ./Testing/CpuProfiling/
  #find ./tests/ -type f -regex '.*/callgrind\.out\.[0-9]+$'
  for FILE in $(find ./tests/ -type f -regex '.*/callgrind\.out\.[0-9]+$'); do
    #echo ">>>>>>>>>>>>>>>>>>>  ${FILE}"
    TEST_NAME=$(grep -E '^cmd: ' ${FILE} | sed 's|.*/\([^/]\+\)$|\1|g')
    #echo "<<<<<<<<<<<<<<<<<<<  ${TEST_NAME}"
    cp ${FILE} ./Testing/CpuProfiling/callgrind.out.${TEST_NAME}
  done

  cd ..
  echo "" && echo ""
  echo "cpu-profiling report :  kcachegrind   $(pwd)/${BUILD_CPUPROF_DIR}/Testing/CpuProfiling/callgrind.out.<TESTNAME>"
fi

#memory-profiling
if [[ "${MEMPROF}" == "1" ]]; then
  mkdir -p ${BUILD_MEMPROF_DIR} && cd ${BUILD_MEMPROF_DIR} && \
  echo "##### START(memProf) : processing/configure of CMakeLists.txt #####" && \
  cmake -DDATASTRUCTURE_BUILD_TESTS=ON -DCMAKE_BUILD_TYPE=MemProf -DCMAKE_CXX_COMPILER=${CXX_COMPILER}  ${LOCAL_DATELIB} ${LOCAL_CONVERTERLIB} .. && \
  echo "##### START(memProf) : project build #####" && cmake --build . ${PARALLEL_PROCESS_ARGS} || \
  exiterr "build failed, exiting."

  echo "##### START(memProf) : project Test #####" && \
  ctest --verbose ${PARALLEL_PROCESS_ARGS} --test-action memcheck ${OPTIONAL_TEST_NAME} || \
  exiterr "memCheck tests failed, exiting."

  mkdir -p ./Testing/MemProfiling/
  #find ./tests/ -type f -regex '.*/massif\.out\.[0-9]+$'
  for FILE in $(find ./tests/ -type f -regex '.*/massif\.out\.[0-9]+$'); do
    #echo ">>>>>>>>>>>>>>>>>>>  ${FILE}"
    TEST_NAME=$(grep -E '^cmd: ' ${FILE} | sed 's|.*/\([^/]\+\)$|\1|g')
    #echo "<<<<<<<<<<<<<<<<<<<  ${TEST_NAME}"
    cp ${FILE} ./Testing/MemProfiling/massif.out.${TEST_NAME}
  done

  cd ..
  echo "" && echo ""
  echo "mem-profiling report :  massif-visualizer   $(pwd)/${BUILD_MEMPROF_DIR}/Testing/MemProfiling/massif.out.<TESTNAME>"
fi

#cppcheck
if [[ "${CPPCHECK}" == "1" ]]; then
  mkdir -p ${BUILD_CPPCHECK_DIR}/cppcheck_staticAnalysis && cd ${BUILD_CPPCHECK_DIR}
  SUPPRESS_FILE="./cppcheck_staticAnalysis/CppCheckSuppressions.txt"
  echo "*:$(pwd)/_deps/*"           > ${SUPPRESS_FILE}
  echo "*:${SOURCE_DIR}/tests/*"   >> ${SUPPRESS_FILE}
  echo ""                          >> ${SUPPRESS_FILE}
  echo "missingIncludeSystem"      >> ${SUPPRESS_FILE}
  echo "#constParameter"           >> ${SUPPRESS_FILE}
  echo "#unusedFunction"           >> ${SUPPRESS_FILE}
  echo "#unmatchedSuppression"     >> ${SUPPRESS_FILE}

  echo "##### START(cppCheck) : processing/configure of CMakeLists.txt #####" && \
  cmake -DDATASTRUCTURE_BUILD_TESTS=ON -DCMAKE_BUILD_TYPE=CppCheck -DCMAKE_CXX_COMPILER=${CXX_COMPILER}  ${LOCAL_DATELIB} ${LOCAL_CONVERTERLIB} .. && \
  echo "##### START(cppCheck) : project build #####" && cmake --build . ${PARALLEL_PROCESS_ARGS} || \
  exiterr "build failed, exiting."

  echo "##### START(cppCheck) : project Test #####" && \
  ctest --verbose ${PARALLEL_PROCESS_ARGS} --test-action test ${OPTIONAL_TEST_NAME} || \
  exiterr "cppCheck tests failed, exiting."

  cd ..
  echo "" && echo ""
  echo "cppcheck report :  $(pwd)/${BUILD_CPPCHECK_DIR}/cppcheck_staticAnalysis/CppCheckSuppressions.txt"
fi


# doc
if [[ "${DOC}" == "1" ]]; then
  if [[ -x "$(command -v doxygenmd)" ]]; then
    doxygenmd include doc || exiterr "doc failed, exiting."
  fi
fi

# install
if [[ "${INSTALL}" == "1" ]]; then
  OS="$(uname)"
  if [ "${OS}" == "Linux" ]; then
    cd ${BUILD_RELEASE_DIR} && sudo cmake --install . && cd .. || exiterr "install failed (linux), exiting."
  elif [ "${OS}" == "Darwin" ]; then
    GHSUDO=""
    if [[ "${GITHUB_ACTIONS}" == "true" ]]; then
      GHSUDO="sudo"
    fi
    cd ${BUILD_RELEASE_DIR} && ${GHSUDO} cmake --install . && cd .. || exiterr "install failed (mac), exiting."
  else
    exiterr "install failed (unsupported os ${OS}), exiting."
  fi
fi

# exit
exit 0
