#pragma once

// easylogging includes
#ifdef NDEBUG
#undef NDEBUG
#define ELPP_THREAD_SAFE
#define ELPP_NO_DEFAULT_LOG_FILE
#include <easylogging++.h>
#define NDEBUG
#else
#define ELPP_THREAD_SAFE
#define ELPP_NO_DEFAULT_LOG_FILE
#include <easylogging++.h>
#endif
