CC = gcc
CFLAGS_BASE = -Wall -Wextra -Wpedantic -std=c99 -Iheaders $(shell pkg-config --cflags sdl2 SDL2_image)
LDFLAGS_BASE = $(shell pkg-config --libs sdl2 SDL2_image) -lm

# Debug vs Release builds
CFLAGS_DEBUG = $(CFLAGS_BASE) -g -O0 -DDEBUG -fsanitize=address -fsanitize=undefined
LDFLAGS_DEBUG = $(LDFLAGS_BASE) -fsanitize=address -fsanitize=undefined

CFLAGS_RELEASE = $(CFLAGS_BASE) -O3 -DNDEBUG
LDFLAGS_RELEASE = $(LDFLAGS_BASE)

# Default to release build
CFLAGS = $(CFLAGS_RELEASE)
LDFLAGS = $(LDFLAGS_RELEASE)

SRCS = src/main.c src/init.c src/textures.c
OBJS = $(SRCS:.c=.o)
TARGET = game

.PHONY: all run clean debug release help

help:
	@echo "Available targets:"
	@echo "  all       - Build release version (default)"
	@echo "  debug     - Build with debugging symbols and sanitizers"
	@echo "  release   - Build optimized release version"
	@echo "  run       - Build and run the game"
	@echo "  clean     - Remove all build artifacts"

all: $(TARGET)

debug: CFLAGS = $(CFLAGS_DEBUG)
debug: LDFLAGS = $(LDFLAGS_DEBUG)
debug: clean $(TARGET)
	@echo "Built DEBUG version with sanitizers"

release: CFLAGS = $(CFLAGS_RELEASE)
release: LDFLAGS = $(LDFLAGS_RELEASE)
release: clean $(TARGET)
	@echo "Built RELEASE version (optimized)"

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)

