#include "logger.h"
#include "asserts.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

b8 initializeLogging() {
    return TRUE;
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

    char outMessage[32000];
    memset(outMessage, 0, sizeof(outMessage));

    va_list argPtr;
    va_start(argPtr, message);
    vsnprintf(outMessage, 32000, message, argPtr);
    va_end(argPtr);

    char outMessageResult[32000];
    sprintf(outMessageResult, "%s%s\n", levelStrings[level], outMessage);

    printf("%s", outMessageResult);
}

void reportAssertionFailure(const char* expression, const char* message,
                            const char* file, i32 line) {
    logOutput(LOG_LEVEL_FATAL,
        "Assertion failure: %s, message: '%s', in file: %s, line: %d\n",
        expression, message, file, line
    );
}
