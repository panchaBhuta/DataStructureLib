/*
 * dataStructure.h
 *
 * URL:      https://github.com/panchaBhuta/dataStructure
 * Version:  v3.5
 *
 * Copyright (C) 2023-2024 Gautam Dhar
 * All rights reserved.
 *
 * dataStructure is distributed under the BSD 3-Clause license, see LICENSE for details.
 *
 */

#pragma once

#define DATASTRUCTURE_VERSION_MAJOR 3
#define DATASTRUCTURE_VERSION_MINOR 5
#define DATASTRUCTURE_VERSION_PATCH 19

#include <filesystem>

//  Project path is removed from the __FILE__
//  Resulting file-path is relative path from project-root-folder.
#if  DATASTRUCTURE_USE_FILEPREFIXMAP == 1
  // the project-prefix-path is removed via compilation directive file-prefix-map
  #define DATASTRUCTURE_FILE    __FILE__
#else
  // https://stackoverflow.com/questions/8487986/file-macro-shows-full-path/40947954#40947954
  // the project-prefix-path is skipped by offsetting to length of project-prefix-path
  //#define DATASTRUCTURE_FILE   (__FILE__ + DATASTRUCTURE_SOURCE_PATH_SIZE)  // gives lot of warnings on windows:clangCL
  #define DATASTRUCTURE_FILE   &(__FILE__[DATASTRUCTURE_SOURCE_PATH_SIZE])
#endif

// to handle windows back-slash path seperator
#define DATASTRUCTURE_PREFERRED_PATH    std::filesystem::path(DATASTRUCTURE_FILE).make_preferred().string()

namespace datastructure {
  /**
   * @brief     Class representing Version number of the project.
  */
  static constexpr struct {
    uint16_t major, minor, patch;
  } version = {
    DATASTRUCTURE_VERSION_MAJOR,
    DATASTRUCTURE_VERSION_MINOR,
    DATASTRUCTURE_VERSION_PATCH
  };
}
