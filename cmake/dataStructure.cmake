#
# URL:      https://github.com/panchaBhuta/dataStructure
#
# Copyright (c) 2023-2023 Gautam Dhar
# All rights reserved.
#
# dataStructure is distributed under the BSD 3-Clause license, see LICENSE for details. 
#

if (CMAKE_VERSION VERSION_GREATER 3.10 OR CMAKE_VERSION VERSION_EQUAL 3.10)
    # Use include_guard() added in cmake 3.10
    include_guard()
endif()

include(CMakePackageConfigHelpers)

set(clang_cxx "$<COMPILE_LANG_AND_ID:CXX,Clang>")
set(clang_like_cxx "$<COMPILE_LANG_AND_ID:CXX,ARMClang,AppleClang,Clang>")
set(gcc_cxx "$<COMPILE_LANG_AND_ID:CXX,GNU>")
set(gcc_like_cxx "$<OR:$<COMPILE_LANG_AND_ID:CXX,GNU,LCC>,${clang_like_cxx}>")
set(msvc_cxx "$<COMPILE_LANG_AND_ID:CXX,MSVC>")
set(windows_os "$<BOOL:${WIN32}>")



function(dataStructure_getversion version_arg)
    # Parse the current version from the dataStructure header
    file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/include/dataStructure.h" dataStructure_version_defines
         REGEX "#define DATASTRUCTURE_VERSION_(MAJOR|MINOR|PATCH)")
    foreach(ver ${dataStructure_version_defines})
        if(ver MATCHES "#define DATASTRUCTURE_VERSION_(MAJOR|MINOR|PATCH) +([^ ]+)$")
            set(DATASTRUCTURE_VERSION_${CMAKE_MATCH_1} "${CMAKE_MATCH_2}" CACHE INTERNAL "")
        endif()
    endforeach()
    set(VERSION ${DATASTRUCTURE_VERSION_MAJOR}.${DATASTRUCTURE_VERSION_MINOR}.${DATASTRUCTURE_VERSION_PATCH})

    # Give feedback to the user. Prefer DEBUG when available since large projects tend to have a lot
    # going on already
    if (CMAKE_VERSION VERSION_GREATER 3.15 OR CMAKE_VERSION VERSION_EQUAL 3.15)
        message(DEBUG "dataStructure version ${VERSION}")
    else()
        message(STATUS "dataStructure version ${VERSION}")
    endif()

    # Return the information to the caller
    set(${version_arg} ${VERSION} PARENT_SCOPE)
endfunction()

#[==================================================================================[
# Optionally, enable unicode support using the ICU library
function(dataStructure_use_unicode)
    find_package(PkgConfig)
    pkg_check_modules(ICU REQUIRED icu-uc)

    target_link_libraries(dataStructure INTERFACE ${ICU_LDFLAGS})
    target_compile_options(dataStructure INTERFACE ${ICU_CFLAGS})
    target_compile_definitions(dataStructure INTERFACE DATASTRUCTURE_USE_UNICODE)
endfunction()
#]==================================================================================]

# Request C++20 without gnu extension for the whole project and enable more warnings
macro(dataStructure_set_cxx_standard)
    # DOUBT : Q  : where does DATASTRUCTURE_CXX_STANDARD get set?
    #         Ans: as this macro gets called only in standalone-mode,
    #              DATASTRUCTURE_CXX_STANDARD might be set on command line
    #              while invoking cmake !
    if (DATASTRUCTURE_CXX_STANDARD)
        set(CMAKE_CXX_STANDARD ${DATASTRUCTURE_CXX_STANDARD})
    else()
        #  --std=gnu++2a
        set(CMAKE_CXX_STANDARD 20)
    endif()

    #set(CMAKE_CXX_EXTENSIONS OFF)
endmacro()


# fetch dependencies of rapidcsv
macro(fetch_dependencies)

    set(CONVERTERLIB "converter")  # local-variable
    # https://stackoverflow.com/questions/29892929/variables-set-with-parent-scope-are-empty-in-the-corresponding-child-scope-why
    #set(CONVERTERLIB ${CONVERTERLIB} PARENT_SCOPE)  # global-variable
    set_target_properties(dataStructure PROPERTIES CONVERTERLIB ${CONVERTERLIB})  # global-variable

    include( FetchContent )
    FetchContent_Declare( ${CONVERTERLIB}
		          GIT_REPOSITORY https://github.com/panchaBhuta/converter.git
		          GIT_TAG        v1.2.13)  # adjust tag/branch/commit as needed
    FetchContent_MakeAvailable(${CONVERTERLIB})

    #[==================[
    FetchContent_GetProperties(${CONVERTERLIB})
    if(NOT ${CONVERTERLIB}_POPULATED)
      FetchContent_Populate(${CONVERTERLIB})
    #  add_subdirectory(${${CONVERTERLIB}_SOURCE_DIR} ${${CONVERTERLIB}_BINARY_DIR} EXCLUDE_FROM_ALL)
    endif()
    #]==================]

    #if (NOT TARGET converter::converter)
    #    find_package(converter REQUIRED)
    #endif()
endmacro()



#[===========[
  Check if compiler supports '-fmacro-prefix-map=old=new'  option
  refer ::: https://fossies.org/linux/bareos/core/CMakeLists.txt
  also refer the links  below ...
  https://reproducible-builds.org/docs/build-path/
  https://blog.conan.io/2019/09/02/Deterministic-builds-with-C-C++.html
  https://gcc.gnu.org/onlinedocs/gcc/Debugging-Options.html#index-fdebug-prefix-map
  https://gcc.gnu.org/onlinedocs/gcc/Preprocessor-Options.html#index-fmacro-prefix-map
  https://gcc.gnu.org/onlinedocs/gcc/Overall-Options.html#index-ffile-prefix-map
#]===========]
macro(dataStructure_check_cxx_compiler_flag_file_prefix_map)
    include(CheckCXXCompilerFlag)
    check_cxx_compiler_flag(-ffile-prefix-map=${CMAKE_CURRENT_SOURCE_DIR}=.
                            cxx_compiler_file_prefix_map
                           )
    if(cxx_compiler_file_prefix_map)
        message(STATUS "dataStructure : compiler option '-ffile-prefix-map=old=new' SUPPORTED")
        target_compile_definitions(dataStructure INTERFACE
                                        DATASTRUCTURE_USE_FILEPREFIXMAP=1)

        target_compile_options(dataStructure INTERFACE
            "-ffile-prefix-map=${CMAKE_CURRENT_SOURCE_DIR}${_path_separator}=")
    else()
        # as of writing this code, clang does not support option '-ffile-prefix-map=...'
        message(STATUS "dataStructure : compiler option '-ffile-prefix-map=old=new' NOT SUPPORTED")
        string(LENGTH "${CMAKE_CURRENT_SOURCE_DIR}/" DATASTRUCTURE_SOURCE_PATH_SIZE)
        target_compile_definitions(dataStructure INTERFACE
                                        DATASTRUCTURE_USE_FILEPREFIXMAP=0
        # https://stackoverflow.com/questions/8487986/file-macro-shows-full-path/40947954#40947954
                                        DATASTRUCTURE_SOURCE_PATH_SIZE=${DATASTRUCTURE_SOURCE_PATH_SIZE})
    endif()
endmacro()

# Helper function to enable warnings
macro(dataStructure_enable_warnings)
    set(gcc_warnings "-Wextra;-Wpedantic;-Wshadow;-Wpointer-arith")
    set(gcc_warnings "${gcc_warnings};-Wcast-qual;-Wno-missing-braces;-Wswitch-default;-Wcast-align;-Winit-self")
    set(gcc_warnings "${gcc_warnings};-Wunreachable-code;-Wundef;-Wuninitialized;-Wold-style-cast;-Wwrite-strings")
    set(gcc_warnings "${gcc_warnings};-Wsign-conversion;-Weffc++")

    # https://cmake.org/cmake/help/v3.27/manual/cmake-generator-expressions.7.html
    #set(is_gnu "$<CXX_COMPILER_ID:GNU>")
    set(v5_or_later "$<VERSION_GREATER_EQUAL:$<CXX_COMPILER_VERSION>,5>")
    set(gcc_cxx_v5_or_later "$<AND:${gcc_cxx},${v5_or_later}>")
    set(windows_os_clang_cxx "$<AND:${windows_os},${clang_cxx}>")  # used when Windows-ClangCl toolchain
    #[==================================================================================[
    # we only want these warning flags to be used during builds.
    # Consumers of our installed project should not inherit our warning flags.
    # To specify this, we wrap our flags in a generator expression using the BUILD_INTERFACE condition.
    #]==================================================================================]
    target_compile_options(dataStructure INTERFACE
        "$<${gcc_like_cxx}:$<BUILD_INTERFACE:${gcc_warnings}>>"
        "$<$<AND:${gcc_like_cxx},$<NOT:${windows_os_clang_cxx}>>:$<BUILD_INTERFACE:-Wall>>" # -Wall for 'windows_os_clang_cxx' gives lot of warnings
        "$<${gcc_cxx_v5_or_later}:$<BUILD_INTERFACE:-Wsuggest-override>>"
        "$<$<NOT:${windows_os}>:$<BUILD_INTERFACE:-g>>"  # for linux and macOS
        "$<${windows_os}:$<BUILD_INTERFACE:-Z7>>"  # -Z7 is equivalent for -g
        #"$<${windows_os_clang_cxx}:$<BUILD_INTERFACE:-Wno-c++98-compat;-Wno-c++98-compat-pedantic>>"
        #"$<${windows_os_clang_cxx}:$<BUILD_INTERFACE:-Wno-global-constructors;-Wno-exit-time-destructors>>"
        #"$<${windows_os_clang_cxx}:$<BUILD_INTERFACE:-Wno-extra-semi-stmt;-Wno-string-plus-int>>"
        "$<${msvc_cxx}:$<BUILD_INTERFACE:-W4>>")
    #add_compile_options("/utf-8")  for msvc  -> check in cxxopts.cmake
endmacro()

# Helper function to configure, include, compile, link, build
macro(dataStructure_build)

    # Build type
    set(DEFAULT_BUILD_TYPE "Release")
    if (DATASTRUCTURE_STANDALONE_PROJECT)
        set(DEFAULT_BUILD_TYPE "Debug")
    endif()
    if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
        message(STATUS "Using build type '${DEFAULT_BUILD_TYPE}' (default).")
        set(CMAKE_BUILD_TYPE "${DEFAULT_BUILD_TYPE}")
    else()
        message(STATUS "Using build type '${CMAKE_BUILD_TYPE}'.")
    endif()

    set(_DEBUG_LOG OFF)
    if (DATASTRUCTURE_STANDALONE_PROJECT AND
        CMAKE_BUILD_TYPE STREQUAL "Debug")
            set(_DEBUG_LOG ON)
    endif()
    #message(STATUS "_DEBUG_LOG=${_DEBUG_LOG}")
    # for _DEBUG_LOG can't use generator-expression as its computed during build-stage, but we need it during config-stage
    option(OPTION_VERSIONEDOBJECT_debug_log  "Set to ON for debugging logs"  ${_DEBUG_LOG})
    message(STATUS "OPTION_VERSIONEDOBJECT_debug_log=${OPTION_VERSIONEDOBJECT_debug_log}")
    #[===========[  donot use generator-expressions in option() functions
    # option(OPTION_VERSIONEDOBJECT_debug_log  "Set to ON for debugging logs"   "$<AND:$<CONFIG:Debug>,$<DATASTRUCTURE_STANDALONE_PROJECT>>")
    #]===========]

    #[==================================================================================[
    add_subdirectory(include)  ??? is it needed ; if so then with include/dataStructure
    # NOT needed, as target_include_directories() can be called here, instead of
    # being called from include/CMakeLists.txt (refer cxxopts which which does this)
    #]==================================================================================]

    #[==================================================================================[
    # refer https://cmake.org/cmake/help/v3.27/manual/cmake-buildsystem.7.html
    # The BUILD_INTERFACE expression wraps requirements which are only used when consumed
    # from a target in the same buildsystem, or when consumed from a target exported to
    # the build directory using the export() command. The INSTALL_INTERFACE expression
    # wraps requirements which are only used when consumed from a target which has been
    # installed and exported with the install(EXPORT) command
    #]==================================================================================]
    target_include_directories(dataStructure INTERFACE
        # BUILD_INTERFACE: Content of ... when the property is exported using export(), or when the
        # target is used by another target in the same buildsystem. Expands to the empty string otherwise.
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        # https://cmake.org/cmake/help/v3.27/manual/cmake-packages.7.html#creating-relocatable-packages
        # INSTALL_INTERFACE: Content of ... when the property is exported using install(EXPORT), and empty otherwise.
        $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/include>)
    # refer https://cmake.org/cmake/help/latest/manual/cmake-generator-expressions.7.html#introduction
    #target_include_directories(dataStructure INTERFACE
    #    "$<$<BOOL:${CMAKE_HOST_UNIX}>:/opt/include/$<CXX_COMPILER_ID>>")

    target_compile_definitions(dataStructure INTERFACE
        $<$<CONFIG:Debug>:DEBUG_BUILD>
        $<$<CONFIG:Release>:RELEASE_BUILD>
        FLAG_VERSIONEDOBJECT_debug_log=$<BOOL:${OPTION_VERSIONEDOBJECT_debug_log}>)
    #[==================================================================================[
    # refer https://cmake.org/cmake/help/v3.27/manual/cmake-generator-expressions.7.html#genex:COMPILE_LANG_AND_ID
    # This specifies the use of different compile definitions based on both the compiler id and compilation language.
    # This example will have a COMPILING_CXX_WITH_CLANG compile definition when Clang is the CXX compiler, and
    # COMPILING_CXX_WITH_INTEL when Intel is the CXX compiler.
    target_compile_definitions(dataStructure INTERFACE
                $<$<COMPILE_LANG_AND_ID:CXX,ARMClang,AppleClang,Clang>:COMPILING_CXX_WITH_CLANG>
                $<$<COMPILE_LANG_AND_ID:CXX,Intel>:COMPILING_CXX_WITH_INTEL>)
    #]==================================================================================]

    target_compile_features(dataStructure INTERFACE
        cxx_constexpr
        cxx_variadic_templates
        cxx_long_long_type)

    #[==================================================================================[
    #add_compile_options("--std=gnu++2a")
    #set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lstdc++") # -lc++abi")

    add_library(libCXX_Clang STATIC IMPORTED GLOBAL)

    # refer https://cmake.org/cmake/help/latest/manual/cmake-generator-expressions.7.html#genex:LINK_LANG_AND_ID
    # This specifies the use of different link libraries based on both the compiler id and link language.
    # This example will have target libCXX_Clang as link dependency when Clang or AppleClang is the CXX linker,
    # and libCXX_Intel when Intel is the CXX linker.
    target_link_libraries(dataStructure INTERFACE
                $<$<LINK_LANG_AND_ID:CXX,Clang,AppleClang>:libCXX_Clang>
                $<$<LINK_LANG_AND_ID:CXX,Intel>:libCXX_Intel>)

        # https://cmake.org/cmake/help/latest/command/target_link_libraries.html#libraries-for-both-a-target-and-its-dependents
        # Library dependencies are transitive by default with this signature. When this target is linked into another target
        # then the libraries linked to this target will appear on the link line for the other target too.
        #
        # https://cmake.org/cmake/help/latest/manual/cmake-buildsystem.7.html#target-usage-requirements
        # add_executable(consumer consumer.cpp)
        # target_link_libraries(consumer archiveExtras)    <<<< NOTE: for executable no scope specified, and the dependency becomes 'transitive'
    #]==================================================================================]
    message(STATUS "'dataStructure' linking to '${CONVERTERLIB}'")
        #[======================[
        # https://cmake.org/cmake/help/latest/command/target_link_libraries.html#libraries-for-a-target-and-or-its-dependents
        # The PUBLIC, PRIVATE and INTERFACE scope keywords can be used to specify both the
        # link dependencies and the link interface in one command.
        #
        # when <target> is a library (i.e NOT an executable), then specify the scope.
            target_link_libraries(<target>
                        <PRIVATE|PUBLIC|INTERFACE> <item>...
                        [<PRIVATE|PUBLIC|INTERFACE> <item>...]...)
        #]======================]
    target_link_libraries(dataStructure INTERFACE ${CONVERTERLIB})
endmacro()

# Helper function to ecapsulate install logic
function(dataStructure_install_logic)
    if(CMAKE_LIBRARY_ARCHITECTURE)
        string(REPLACE "/${CMAKE_LIBRARY_ARCHITECTURE}" "" CMAKE_INSTALL_LIBDIR_ARCHIND "${CMAKE_INSTALL_LIBDIR}")
    else()
        # On some systems (e.g. NixOS), `CMAKE_LIBRARY_ARCHITECTURE` can be empty
        set(CMAKE_INSTALL_LIBDIR_ARCHIND "${CMAKE_INSTALL_LIBDIR}")
    endif()
    set(DATASTRUCTURE_CMAKE_DIR "${CMAKE_INSTALL_LIBDIR_ARCHIND}/cmake/dataStructure"
        CACHE STRING "Installation directory for cmake files, relative to ${CMAKE_INSTALL_PREFIX}.")
    # PROJECT_BINARY_DIR : the binary directory of the most recent project() command.
    set(version_config "${PROJECT_BINARY_DIR}/dataStructure-config-version.cmake")
    set(project_config "${PROJECT_BINARY_DIR}/dataStructure-config.cmake")
    #[==================================================================================[
    # Note :The installed <export-name>.cmake file may come with additional
    # per-configuration <export-name>-*.cmake files to be loaded by globbing.
    # Do not use an export name that is the same as the package name in
    # combination with installing a <package-name>-config.cmake file or the
    # latter may be incorrectly matched by the glob and loaded.
    # https://cmake.org/cmake/help/v3.27/command/install.html#export
    #]==================================================================================]
    set(targets_export_name dataStructure-export)
    set(PackagingTemplatesDir "${PROJECT_SOURCE_DIR}/packaging")


    if(${CMAKE_VERSION} VERSION_GREATER "3.14")
        set(OPTIONAL_ARCH_INDEPENDENT "ARCH_INDEPENDENT")
    endif()

    # Generate the version, config and target files into the build directory.
    write_basic_package_version_file(
        ${version_config}
        VERSION       ${VERSION}
        #COMPATIBILITY AnyNewerVersion
        COMPATIBILITY SameMajorVersion
        ${OPTIONAL_ARCH_INDEPENDENT})
    #[==================================================================================[
    # configure_package_config_file() should be used instead of the plain configure_file()
    # command when creating the <PackageName>Config.cmake or <PackageName>-config.cmake file
    # for installing a project or library. It helps making the resulting package relocatable
    # by avoiding hardcoded paths in the installed Config.cmake file.
    # https://cmake.org/cmake/help/v3.27/module/CMakePackageConfigHelpers.html
    #]==================================================================================]
    configure_package_config_file(
        ${PackagingTemplatesDir}/dataStructure-config.cmake.in
        ${project_config}
        INSTALL_DESTINATION ${DATASTRUCTURE_CMAKE_DIR})

    # Install version, config and target files.
    install(
        FILES       ${project_config} ${version_config}
        DESTINATION ${DATASTRUCTURE_CMAKE_DIR})



    #[==================================================================================[
    # Add source to a target ( target_source -> cmake v3.23 )
    # File set(s) are defined here.
    # https://cmake.org/cmake/help/v3.27/command/target_sources.html#file-sets
    #]==================================================================================]
    target_sources(dataStructure
        INTERFACE
        FILE_SET   dataStructure_headers
        TYPE       HEADERS
        BASE_DIRS  include)
    #[==================================================================================[
    # https://cmake.org/cmake/help/v3.27/command/install.html#targets
    # Install the header files and export the target
    # EXPORT
    # This option associates(or defines) the installed target files with an export called <export-name>.
    # It must appear before any target options. To actually install the export file itself,
    # call install(EXPORT), documented below.
    # Here  ${targets_export_name} == <export-name>

    # If EXPORT is used and the targets include PUBLIC or INTERFACE file sets, all of them
    # must be specified with FILE_SET arguments. All PUBLIC or INTERFACE file sets associated
    # with a target are included in the export.
    #]==================================================================================]
    install(TARGETS      dataStructure
                         ${CONVERTERLIB} # not needed
        EXPORT           ${targets_export_name}
        FILE_SET         dataStructure_headers
        DESTINATION      ${CMAKE_INSTALL_INCLUDEDIR})
    #[==================================================================================[
    # https://cmake.org/cmake/help/v3.27/command/install.html#export
    # The EXPORT form generates and installs a CMake file containing code
    # to import targets from the installation tree into another project.
    # By default the generated file will be called <export-name>.cmake
    # but the FILE option may be used to specify a different name.

    # https://cmake.org/cmake/help/v3.27/manual/cmake-packages.7.html#creating-packages
    # This command generates the ClimbingStatsTargets.cmake file to contain IMPORTED
    # targets, suitable for use by downstreams and arranges to install it to
    # lib/cmake/ClimbingStats.
    # The generated ClimbingStatsConfigVersion.cmake and a cmake/ClimbingStatsConfig.cmake
    # are installed to the same location, completing the package.

    # A NAMESPACE with double-colons is specified when exporting the targets for installation.
    # This convention of double-colons gives CMake a hint that the name is an IMPORTED target
    # when it is used by downstreams with the target_link_libraries() command.
    # This way, CMake can issue a diagnostic if the package providing it has not yet been found.
    #]==================================================================================]
    install(EXPORT    ${targets_export_name}
        DESTINATION   ${DATASTRUCTURE_CMAKE_DIR}
        NAMESPACE     dataStructure::)
    #[==================================================================================[
    export(EXPORT ${targets_export_name} ....)  not needed to be called, reason as per below.
    # https://cmake.org/cmake/help/v3.27/command/export.html#exporting-targets-matching-install-export
    # Creates a file <filename> that may be included by outside projects to import targets
    # from the current project's build tree. This is the same as the export(TARGETS) signature,
    # except that the targets are not explicitly listed. Instead, it exports the targets
    # associated with the installation export <export-name>.
    #]==================================================================================]

    # https://cmake.org/cmake/help/v3.27/command/install.html
    install(TARGETS         dataStructure
        CONFIGURATIONS      Debug
        #RUNTIME DESTINATION Debug/bin
    	PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/dataStructure)
    install(TARGETS         dataStructure
        CONFIGURATIONS      Release
        #RUNTIME DESTINATION Release/bin
    	PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/dataStructure)


    # https://cmake.org/cmake/help/v3.27/command/export.html#command:export
    # Export targets or packages for outside projects to use them
    # directly from the current project's build tree, without installation.
    export(TARGETS dataStructure
                   ${CONVERTERLIB} # needed : donot comment
        NAMESPACE  dataStructure::
        FILE       ${PROJECT_BINARY_DIR}/${targets_export_name}.cmake)

    set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
    set(CPACK_PACKAGE_VENDOR "dataStructure developers")
    set(CPACK_PACKAGE_DESCRIPTION "${PROJECT_DESCRIPTION}")
    set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
    set(CPACK_PACKAGE_VERSION_MAJOR "${DATASTRUCTURE_VERSION_MAJOR}")
    set(CPACK_PACKAGE_VERSION_MINOR "${DATASTRUCTURE_VERSION_MINOR}")
    set(CPACK_SOURCE_GENERATOR "TGZ")

    set(CPACK_DEBIAN_PACKAGE_NAME "${CPACK_PACKAGE_NAME}")
    set(CPACK_RPM_PACKAGE_NAME "${CPACK_PACKAGE_NAME}")
    set(CPACK_PACKAGE_HOMEPAGE_URL "${PROJECT_HOMEPAGE_URL}")
    set(CPACK_PACKAGE_MAINTAINER "${CPACK_PACKAGE_VENDOR}")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "${CPACK_PACKAGE_MAINTAINER}")
    set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
    set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")

    set(CPACK_DEBIAN_PACKAGE_NAME "lib${PROJECT_NAME}-dev")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6-dev")
    set(CPACK_DEBIAN_PACKAGE_SUGGESTS "cmake, pkg-config, pkg-conf")

    set(CPACK_RPM_PACKAGE_NAME "lib${PROJECT_NAME}-devel")
    set(CPACK_RPM_PACKAGE_SUGGESTS "${CPACK_DEBIAN_PACKAGE_SUGGESTS}")

    set(CPACK_DEB_COMPONENT_INSTALL ON)
    set(CPACK_RPM_COMPONENT_INSTALL ON)
    set(CPACK_NSIS_COMPONENT_INSTALL ON)
    set(CPACK_DEBIAN_COMPRESSION_TYPE "xz")

    set(PKG_CONFIG_FILE_NAME "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.pc")
    configure_file("${PackagingTemplatesDir}/pkgconfig.pc.in" "${PKG_CONFIG_FILE_NAME}" @ONLY)
    install(FILES "${PKG_CONFIG_FILE_NAME}"
            DESTINATION "${CMAKE_INSTALL_LIBDIR_ARCHIND}/pkgconfig")

    include(CPack)

    # https://cmake.org/cmake/help/v3.27/command/export.html#exporting-packages
    #export(PACKAGE <PackageName>) ????? not needed for now as this deals with package-registry


    # Uninstall
    add_custom_target(uninstall COMMAND "${CMAKE_COMMAND}" -E remove "${CMAKE_INSTALL_PREFIX}/include/dataStructure")
endfunction()
