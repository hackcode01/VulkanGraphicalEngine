#ifndef __LOGGER_H__
#define __LOGGER_H__

#include "../defines.h"

#define LOG_WARNING_ENABLED 1
#define LOG_INFO_ENABLED 1
#define LOG_DEBUG_ENABLED 1
#define LOG_TRACE_ENABLED 1

/* Disable debug and trace logging for release builds. */
#if RELEASE == 1
#define LOG_DEBUG_ENABLED 0
#define LOG_TRACE_ENABLED 0
#endif

typedef enum logLevel {
    LOG_LEVEL_FATAL = 0,
    LOG_LEVEL_ERROR = 1,
    LOG_LEVEL_WARNING = 2,
    LOG_LEVEL_INFO = 3,
    LOG_LEVEL_DEBUG = 4,
    LOG_LEVEL_TRACE = 5
} logLevel_t;

b8 initializeLogging();
void shutdownLogging();

API void logOutput(logLevel_t level, const char* message, ...);

/* Logs a fatal-level message. */
#define FATAL(message, ...) logOutput(LOG_LEVEL_FATAL, message, ##__VA_ARGS__);

#ifndef ERROR
/* Logs an error-level message. */
#define ERROR(message, ...) logOutput(LOG_LEVEL_ERROR, message, ##__VA_ARGS__);
#endif

#if LOG_WARNING_ENABLED == 1
/* Logs a warning-level message. */
#define WARNING(message, ...) logOutput(LOG_LEVEL_WARNING, message, ##__VA_ARGS__);

#else
/* Does nothing when LOG_WARN_ENABLED != 1. */
#define WARNING(message, ...)
#endif

#if LOG_INFO_ENABLED == 1
/* Logs a info-level message. */
#define INFO(message, ...) logOutput(LOG_LEVEL_INFO, message, ##__VA_ARGS__);

#else
/* Does nothing when LOG_INFO_ENABLED != 1. */
#define INFO(message, ...)
#endif

#if LOG_DEBUG_ENABLED == 1
/* Logs a debug-level message. */
#define DEBUG(message, ...) logOutput(LOG_LEVEL_DEBUG, message, ##__VA_ARGS__);

#else
/* Does nothing when LOG_DEBUG_ENABLED != 1. */
#define DEBUG(message, ...)
#endif

#if LOG_TRACE_ENABLED == 1
/* Logs a trace-level message. */
#define TRACE(message, ...) logOutput(LOG_LEVEL_TRACE, message, ##__VA_ARGS__);

#else
/* Does nothing when LOG_TRACE_ENABLED != 1. */
#define TRACE(message, ...)
#endif

#endif
