/**
 * @file logger.hpp
 * @brief Logging macros for the container monitor application.
 *
 * Provides macros for logging at different severity levels.
 * Uses Google glog if ENABLE_GLOG_LOGGING is defined, otherwise falls back to std::cout and std::cerr.
 */

#pragma once

// Uncomment the next line to enable glog logging
#define ENABLE_GLOG_LOGGING

#ifdef ENABLE_GLOG_LOGGING
#include <glog/logging.h>
/**
 * @def CM_LOG_INFO
 * @brief Info-level logging macro (glog).
 */
/**
 * @def CM_LOG_WARN
 * @brief Warning-level logging macro (glog).
 */
/**
 * @def CM_LOG_ERROR
 * @brief Error-level logging macro (glog).
 */
/**
 * @def CM_LOG_FATAL
 * @brief Fatal-level logging macro (glog).
 */
#define CM_LOG_INFO   LOG(INFO)
#define CM_LOG_WARN   LOG(WARNING)
#define CM_LOG_ERROR  LOG(ERROR)
#define CM_LOG_FATAL  LOG(FATAL)
#else
#include <iostream>
/**
 * @def CM_LOG_INFO
 * @brief Info-level logging macro (std::cout).
 */
/**
 * @def CM_LOG_WARN
 * @brief Warning-level logging macro (std::cerr).
 */
/**
 * @def CM_LOG_ERROR
 * @brief Error-level logging macro (std::cerr).
 */
/**
 * @def CM_LOG_FATAL
 * @brief Fatal-level logging macro (std::cerr).
 */
#define CM_LOG_INFO   std::cout
#define CM_LOG_WARN   std::cerr
#define CM_LOG_ERROR  std::cerr
#define CM_LOG_FATAL  std::cerr
#endif