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

typedef enum {
    LOG_LEVEL_FATAL = 0,
    LOG_LEVEL_ERROR = 1,
    LOG_LEVEL_WARNING = 2,
    LOG_LEVEL_INFO = 3,
    LOG_LEVEL_DEBUG = 4,
    LOG_LEVEL_TRACE = 5
} LogLevel;

typedef enum {
    FAILED_CREATE_GAME = -1,
    FAILED_ASSIGNED_FUNCTION_GAME = -2,
    FAILED_CREATE_APPLICATION = -3,
    FAILED_APPLICATION_SHUTDOWN_GRACEFULLY = -4
} TypeErrors_t;

b8 initializeLogging();
void shutdownLogging();

ENGINE_API void logOutput(LogLevel level, const char* message, ...);

/* Logs a fatal-level message. */
#define ENGINE_FATAL(message, ...) logOutput(LOG_LEVEL_FATAL, message, ##__VA_ARGS__);

#ifndef ENGINE_ERROR
/* Logs a error-level message. */
#define ENGINE_ERROR(message, ...) logOutput(LOG_LEVEL_ERROR, message, ##__VA_ARGS__);
#endif

#if LOG_WARNING_ENABLED == 1
/* Logs a warning-level message. */
#define ENGINE_WARNING(message, ...) logOutput(LOG_LEVEL_WARNING, message, ##__VA_ARGS__);
#else
/* Does nothing when LOG_WARN_ENABLED != 1 */
#define ENGINE_WARNING(message, ...)
#endif

#if LOG_INFO_ENABLED == 1
/* Logs a info-level message. */
#define ENGINE_INFO(message, ...) logOutput(LOG_LEVEL_INFO, message, ##__VA_ARGS__);
#else
/* Does nothing when LOG_INFO_ENABLED != 1 */
#define ENGINE_INFO(message, ...)
#endif

#if LOG_DEBUG_ENABLED == 1
/* Logs a debug-level message. */
#define ENGINE_DEBUG(message, ...) logOutput(LOG_LEVEL_DEBUG, message, ##__VA_ARGS__);
#else
/* Does nothing when LOG_DEBUG_ENABLED != 1 */
#define ENGINE_DEBUG(message, ...)
#endif

#if LOG_TRACE_ENABLED == 1
/* Logs a trace-level message. */
#define ENGINE_TRACE(message, ...) logOutput(LOG_LEVEL_TRACE, message, ##__VA_ARGS__);
#else
/* Does nothing when LOG_TRACE_ENABLED != 1 */
#define ENGINE_TRACE(message, ...)
#endif

#endif
