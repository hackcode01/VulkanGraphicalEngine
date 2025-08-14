# Asserts

## Definitions

Contains a function for generating a report on the name of the error and its content; macros that generate errors about the engine's operation, generate debugging information with a message, file name, and line in the file where the error occurred.

## Functions

- void reportAssertionFailure(const char* expression, const char* message, const char* file, i32 line);

## Macroses

- ENGINE_ASSERT(expression)

- ENGINE_ASSERT_MESSAGE(expression, message)

- ENGINE_ASSERT_DEBUG(expression)
