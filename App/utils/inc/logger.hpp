#pragma once

// Uncomment the next line to enable glog logging
// #define ENABLE_GLOG_LOGGING

#ifdef ENABLE_GLOG_LOGGING
#include <glog/logging.h>
#define CM_LOG_INFO   LOG(INFO)
#define CM_LOG_WARN   LOG(WARNING)
#define CM_LOG_ERROR  LOG(ERROR)
#define CM_LOG_FATAL  LOG(FATAL)
#else
#include <iostream>
#define CM_LOG_INFO   std::cout
#define CM_LOG_WARN   std::cerr
#define CM_LOG_ERROR  std::cerr
#define CM_LOG_FATAL  std::cerr
#endif