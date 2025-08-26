#include "logger.h"
#include "asserts.h"
#include "../platform/platform.h"
#include "../platform/filesystem.h"
#include "../engine_memory/engine_string.h"
#include "../engine_memory/engine_memory.h"

#include <stdarg.h>

typedef struct LoggerSystemState {
    FileHandle logFileHandle;
} LoggerSystemState;

static LoggerSystemState* statePtr;

void appendToLogFile(const char *message) {
    if (statePtr && statePtr->logFileHandle.isValid) {
        /** Since the message already contains a '\n', just write the bytes directly. */
        u64 length = stringLength(message);
        u64 written = 0;

        if (!filesystemWrite(&statePtr->logFileHandle, length, message, &written)) {
            platformConsoleWriteError("ERROR: writting to console.log.", LOG_LEVEL_ERROR);
        }
    }
}

b8 initializeLogging(u64* memoryRequirement, void* state) {
    *memoryRequirement = sizeof(LoggerSystemState);
    if (state == 0) {
        return true;
    }

    statePtr = state;

    /** Create new/wipe existing log file, then open it. */
    if (!filesystemOpen("console.log", FILE_MODE_WRITE, false, &statePtr->logFileHandle)) {
        platformConsoleWriteError("ERROR: Unable to open console.log for writting.", LOG_LEVEL_ERROR);
        return false;
    }

    /** Create log file. */
    return true;
}

void shutdownLogging(void* state) {
    /** Cleanup logging/write queued entries. */
    statePtr = 0;
}

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

    char outMessage[32000ULL];
    engineZeroMemory(outMessage, sizeof(outMessage));

    va_list argPtr;
    va_start(argPtr, message);
    stringFormatV(outMessage, message, argPtr);
    va_end(argPtr);

    /** Prepend log level to message. */
    stringFormat(outMessage, "%s%s\n", levelStrings[level], outMessage);

    if (isError) {
        platformConsoleWriteError(outMessage, level);
    } else {
        platformConsoleWrite(outMessage, level);
    }

    /** Queue a copy to be written to the log file. */
    appendToLogFile(outMessage);
}

void reportAssertionFailure(const char* expression, const char* message,
                            const char* file, i32 line) {
    logOutput(LOG_LEVEL_FATAL,
        "Assertion failure: %s, message: '%s', in file: %s, line: %d\n",
        expression, message, file, line
    );
}
