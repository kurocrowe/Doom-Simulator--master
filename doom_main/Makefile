# Compiler and flags
CC = clang-cl
CFLAGS = /nologo /Zi /W3 /Iglad /Iinclude
LDFLAGS = user32.lib opengl32.lib glfw3.lib /m64

# File discovery
SRC_DIRS = src glad
SRCS = $(foreach dir, $(SRC_DIRS), $(wildcard $(dir)/*.c))
OBJS = $(patsubst %.c, %.obj, $(SRCS))

# Output target
TARGET = build/app.exe

# Default rule
all: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir build 2>nul || exit 0
	$(CC) $(OBJS) $(LDFLAGS)

# Compile all .c files to .obj
%.obj: %.c
	$(CC) $(CFLAGS) -c $< -Fo:$@

# Clean rule
clean:
	del /Q $(OBJS) $(TARGET) 2>nul || exit 0
