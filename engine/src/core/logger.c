#include "../defines.h"
#include "./logger.h"
#include "../platforms/includes/platform.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

/* Creare log. */
b8 initializeLogging() {
    return TRUE;
}

/* Cleanup logging/write queued entries. */
void shutdownLogging() {}

void logOutput(logLevel_t level, const char* message, ...) {
    const char* levelStrings[6] = {
        "[FATAL]: ", "[ERROR]: ", "[WARN]:  ", "[INFO]:  ", "[DEBUG]: ", "[TRACE]: "
    };

    b8 isError = level < LOG_LEVEL_WARNING;

    /* Technically imposes a 32k character limit on a single log entry, but...
     * DON'T DO THAT!
     */
    const i32 messageLength = 32000;
    char inputMessage[messageLength];
    memset(inputMessage, 0, sizeof(inputMessage));

    /* Format original message.
     * NOTE: Oddly enough, MS's headers override the GCC/Clang va_list type with a "typedef char* va_list" in some
     * cases, and as a result throws a strange error here. The workaround for now is to just use __builtin_va_list,
     * which is the type GCC/Clang's va_start expects.
     */
    __builtin_va_list argPtr;
    va_start(argPtr, message);
    vsnprintf(inputMessage, messageLength, message, argPtr);
    va_end(argPtr);

    char outMessage[messageLength];
    sprintf(outMessage, "%s%s\n", levelStrings[level], inputMessage);

    /* platform-specific output. */
    if (isError) {
        platformConsoleWriteError(outMessage, level);
    } else {
        platformConsoleWrite(outMessage, level);
    }
}

void reportAssertionFailure(const char* expression, const char* message,
                            const char* file, i32 lineNumber) {
    logOutput(LOG_LEVEL_FATAL, "Assertion Failure: %s, message: '%s',"
                               "in file: %s, line: %d\n",
                               expression, message, file, lineNumber);
}
