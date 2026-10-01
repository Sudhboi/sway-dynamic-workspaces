# 1. Configuration variables
CC         := gcc
CFLAGS     := -Wall -Wextra -O2 -MMD -MP
SRC_DIR    := src
BUILD_DIR  := build
TARGET     := $(BUILD_DIR)/dynwork

# 2. Automatically find sources and map them to the build directory
SRCS       := $(wildcard $(SRC_DIR)/*.c)
OBJS       := $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRCS))
DEPS       := $(OBJS:.o=.d)

# 3. Default target
.PHONY: all clean

all: $(TARGET)

# 4. Link the executable
$(TARGET): $(OBJS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o $@ $^

# 5. Compile source files to object files in the build directory
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

# 6. Include automatically generated dependency files (.d)
-include $(DEPS)

# 7. Clean up the build directory
clean:
	rm -rf $(BUILD_DIR)
