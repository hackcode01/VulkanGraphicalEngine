#include "logger.h"
#include "asserts.h"
#include "../platform/platform.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

b8 initializeLogging() {
    return true;
}

void shutdownLogging() {}

void logOutput(LogLevel level, const char* message, ...) {
    const char* levelStrings[6] = {
        "[FATAL]",
        "[ERROR]",
        "[WARNING]",
        "[INFO]",
        "[DEBUG]",
        "[TRACE]"
    };
    b8 isError = level < LOG_LEVEL_WARNING;

    char outMessage[32000ull];
    memset(outMessage, 0, sizeof(outMessage));

    va_list argPtr;
    va_start(argPtr, message);
    vsnprintf(outMessage, 32000ull, message, argPtr);
    va_end(argPtr);

    char outMessageResult[32000ull];
    sprintf(outMessageResult, "%s%s\n", levelStrings[level], outMessage);

    if (isError) {
        platformConsoleWriteError(outMessageResult, level);
    } else {
        platformConsoleWrite(outMessageResult, level);
    }
}

void reportAssertionFailure(const char* expression, const char* message,
                            const char* file, i32 line) {
    logOutput(LOG_LEVEL_FATAL,
        "Assertion failure: %s, message: '%s', in file: %s, line: %d\n",
        expression, message, file, line
    );
}
