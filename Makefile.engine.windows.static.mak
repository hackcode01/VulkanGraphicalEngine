DIR := $(subst /,\,${CURDIR})

BUILD_DIR := bin_static
OBJ_DIR := obj
LIB_DIR := bin_static

ASSEMBLY := engine
EXTENSION := .lib
COMPILER_FLAGS := -g -MD -Werror=vla -fdeclspec
INCLUDE_FLAGS := -Iengine\src -I$(VULKAN_SDK)\include
LINKER_FLAGS := -g -L$(LIB_DIR)
DEFINES := -D_DEBUG -DENGINE_EXPORT -D_CRT_SECURE_NO_WARNINGS

# Make does not offer a recursive wildcard function, so here's one:
rwildcard=$(wildcard $1$2) $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2))

SRC_FILES := $(call rwildcard,$(ASSEMBLY)/,*.c) # Get all .c files
DIRECTORIES := \$(ASSEMBLY)\src $(subst $(DIR),,$(shell dir $(ASSEMBLY)\src /S /AD /B | findstr /i src)) # Get all directories under src.
OBJ_FILES := $(SRC_FILES:%=$(OBJ_DIR)/%.o) # Get all compiled .c.o objects for engine

all: scaffold compile lib

.PHONY: scaffold
scaffold: # create build directory
	@echo Scaffolding folder structure...
	-@setlocal enableextensions enabledelayedexpansion && mkdir $(addprefix $(OBJ_DIR), $(DIRECTORIES)) 2>NUL || cd .
	-@setlocal enableextensions enabledelayedexpansion && mkdir $(BUILD_DIR) 2>NUL || cd .
	-@setlocal enableextensions enabledelayedexpansion && mkdir $(LIB_DIR) 2>NUL || cd .
	@echo Done.

.PHONY: lib
lib: $(OBJ_FILES) # create static library
	@echo Creating static library $(ASSEMBLY)$(EXTENSION)...
	@llvm-ar rcs $(LIB_DIR)/$(ASSEMBLY)$(EXTENSION) $(OBJ_FILES)

.PHONY: compile
compile: #compile .c files
	@echo Compiling engine...

.PHONY: clean
clean: # clean build directory
	if exist $(LIB_DIR)\$(ASSEMBLY)$(EXTENSION) del $(LIB_DIR)\$(ASSEMBLY)$(EXTENSION)
	rmdir /s /q $(OBJ_DIR)\$(ASSEMBLY)
	rmdir /s /q $(LIB_DIR)

$(OBJ_DIR)/%.c.o: %.c # compile .c to .c.o object
	@echo   $<...
	@clang $< $(COMPILER_FLAGS) -c -o $@ $(DEFINES) $(INCLUDE_FLAGS)

-include $(OBJ_FILES:.o=.d)
