#include "filesystem.h"

#include "../core/logger.h"
#include "../engine_memory/engine_memory.h"

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

b8 filesystemExists(const char *path) {
#ifdef _MSC_VER
    struct _stat buffer;
    return _stat(path, &buffer);
#else
    struct stat buffer;
    return stat(path, &buffer) == 0;
#endif
}

b8 filesystemOpen(const char *path, FileModes mode, b8 binary, FileHandle *outHandle) {
    outHandle->isValid = false;
    outHandle->handle = 0;
    const char *modeStr;

    if ((mode & FILE_MODE_READ) != 0 && (mode & FILE_MODE_WRITE) != 0) {
        modeStr = binary ? "w+b" : "w+";
    } else if ((mode & FILE_MODE_READ) != 0 && (mode & FILE_MODE_WRITE) == 0) {
        modeStr = binary ? "rb" : "r";
    } else if ((mode & FILE_MODE_READ) == 0 && (mode & FILE_MODE_WRITE) != 0) {
        modeStr = binary ? "wb" : "w";
    } else {
        ENGINE_ERROR("Invalid mode passed while trying to open file: '%s'", path);

        return false;
    }

    /** Attempt to open the file. */
    FILE *file = fopen(path, modeStr);
    if (!file) {
        ENGINE_ERROR("Error opening file: '%s'", path);

        return false;
    }

    outHandle->handle = file;
    outHandle->isValid = true;

    return true;
}

void filesystemClose(FileHandle *handle) {
    if (handle->handle) {
        fclose((FILE*)handle->handle);
        handle->handle = 0;
        handle->isValid = false;
    }
}

b8 filesystemReadLine(FileHandle *handle, u64 maxLength, char **lineBuffer, u64 *outLineLength) {
    if (handle->handle && lineBuffer && outLineLength && maxLength > 0) {
        char *buffer = *lineBuffer;
        if (fgets(buffer, maxLength, (FILE*)handle->handle) != 0) {
            *outLineLength = strlen(*lineBuffer);

            return true;
        }
    }

    return false;
}

b8 filesystemWriteLine(FileHandle *handle, const char *text) {
    if (handle->handle) {
        i32 result = fputs(text, (FILE*)handle->handle);
        if (result != EOF) {
            result = fputc('\n', (FILE*)handle->handle);
        }

        /**
         * Make sure to flush the stream so it is written to the file immediately.
         * This prevents data loss in the event of a crash.
         */
        fflush((FILE*)handle->handle);

        return result != EOF;
    }

    return false;
}

b8 filesystemRead(FileHandle *handle, u64 dataSize, void *outData, u64 *outBytesRead) {
    if (handle->handle && outData) {
        *outBytesRead = fread(outData, 1, dataSize, (FILE*)handle->handle);

        if (*outBytesRead != dataSize) {
            return false;
        }

        return true;
    }

    return false;
}

b8 filesystemReadAllBytes(FileHandle *handle, u8 **outBytes, u64 *outBytesRead) {
    if (handle->handle) {
        /** File size. */
        fseek((FILE*)handle->handle, 0, SEEK_END);
        u64 size = ftell((FILE*)handle->handle);
        rewind((FILE*)handle->handle);

        *outBytes = engineAllocate(sizeof(u8) * size, MEMORY_TAG_STRING);
        *outBytesRead = fread(*outBytes, 1, size, (FILE*)handle->handle);

        if (*outBytesRead != size) {
            return false;
        }

        return true;
    }

    return false;
}

b8 filesystemWrite(FileHandle *handle, u64 dataSize, const void *data, u64 *outBytesWritten) {
    if (handle->handle) {
        *outBytesWritten = fwrite(data, 1, dataSize, (FILE*)handle->handle);

        if (*outBytesWritten != dataSize) {
            return false;
        }

        fflush((FILE*)handle->handle);
        return true;
    }

    return false;
}
