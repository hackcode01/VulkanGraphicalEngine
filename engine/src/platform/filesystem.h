#ifndef __PLATFORM_FILESYSTEM_H__
#define __PLATFORM_FILESYSTEM_H__

#include "../defines.h"

/** Holds a handle to a file. */
typedef struct FileHandle {
    /** Opaque handle to interval file handle. */
    void *handle;
    b8 isValid;
} FileHandle;

typedef enum FileModes {
    FILE_MODE_READ = 0x1,
    FILE_MODE_WRITE = 0x2
} FileModes;

/**
 * Checks if a file with the given path exists.
 * @param path The path of the file to be checked.
 * @returns True of exists; otherwise false.
 */
ENGINE_API b8 filesystemExists(const char *path);

/** 
 * Attempt to open file located at path.
 * @param path The path of the file to be opened.
 * @param mode Mode flags for the file when opened (read/write). See file_modes enum in filesystem.h.
 * @param binary Indicates if the file should be opened in binary mode.
 * @param outHandle A pointer to a file_handle structure which holds the handle information.
 * @returns True if opened successfully; otherwise false.
 */
ENGINE_API bool filesystemOpen(const char *path, FileModes mode, b8 binary,
    FileHandle *outHandle);

/** 
 * Closes the provided handle to a file.
 * @param handle A pointer to a file_handle structure which holds the handle to be closed.
 */
ENGINE_API void filesystemClose(FileHandle *handle);

/** 
 * Reads up to a newline or EOF. Allocates *line_buf, which must be freed by the caller.
 * @param handle A pointer to a file_handle structure.
 * @param lineBuffer A pointer to a character array which will be allocated and populated by this method.
 * @returns True if successful; otherwise false.
 */
ENGINE_API b8 filesystemReadLine(FileHandle *handle, char **lineBuffer);

/** 
 * Writes text to the provided file, appending a '\n' afterward.
 * @param handle A pointer to a file_handle structure.
 * @param text The text to be written.
 * @returns True if successful; otherwise false.
 */
ENGINE_API b8 filesystemWriteLine(FileHandle *handle, const char *text);

/** 
 * Reads up to dataSize bytes of data into out_bytes_read. 
 * Allocates *out_data, which must be freed by the caller.
 * @param handle A pointer to a file_handle structure.
 * @param dataSize The number of bytes to read.
 * @param outData A pointer to a block of memory to be populated by this method.
 * @param outBytesRead A pointer to a number which will be populated with the number of bytes actually read from the file.
 * @returns True if successful; otherwise false.
 */
ENGINE_API b8 filesystemRead(FileHandle *handle, u64 dataSize, void *outData,
    u64 *outBytesRead);

/** 
 * Reads up to dataSize bytes of data into out_bytes_read. 
 * Allocates *out_bytes, which must be freed by the caller.
 * @param handle A pointer to a file_handle structure.
 * @param outBytes A pointer to a byte array which will be allocated and populated by this method.
 * @param outBytesRead A pointer to a number which will be populated with the number of bytes actually read from the file.
 * @returns True if successful; otherwise false.
 */
ENGINE_API b8 filesystemReadAllBytes(FileHandle *handle, u8 **outBytes, u64 *outBytesRead);

/** 
 * Writes provided data to the file.
 * @param handle A pointer to a file_handle structure.
 * @param dataSize The size of the data in bytes.
 * @param data The data to be written.
 * @param outBytesWritten A pointer to a number which will be populated with the number of bytes actually written to the file.
 * @returns True if successful; otherwise false.
 */
ENGINE_API b8 filesystemWrite(FileHandle *handle, u64 dataSize, const void *data,
    u64 *outBytesWritten);

#endif
