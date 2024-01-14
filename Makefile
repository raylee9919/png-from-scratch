TARGET_EXEC := program

BUILD_DIR := ./build
SRC_DIRS := ./src

SRCS := $(shell find $(SRC_DIRS) -name '*.c')
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
INC_DIRS := $(shell find $(SRC_DIRS) -type d)

CC := cc
CFLAGS := -fPIC -Wall -g -DPNG_DEBUG
LFLAGS := -lSDL2

$(BUILD_DIR)/$(TARGET_EXEC): $(OBJS)
	$(CC) $(OBJS) $(LFLAGS) -o $@

$(BUILD_DIR)/%.c.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: run clean

run:
	$(BUILD_DIR)/$(TARGET_EXEC)
clean:
	rm -r $(BUILD_DIR)
