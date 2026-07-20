CC ?= cc
PKG_CONFIG ?= pkg-config

SDL_PACKAGES := sdl2 SDL2_image
SDL_CFLAGS := $(shell $(PKG_CONFIG) --cflags $(SDL_PACKAGES) 2>/dev/null)
SDL_LIBS := $(shell $(PKG_CONFIG) --libs $(SDL_PACKAGES) 2>/dev/null)

ifeq ($(OS),Windows_NT)
EXEEXT := .exe
endif

TARGET ?= game$(EXEEXT)
BUILD_DIR ?= build
RELEASE_DIR := $(BUILD_DIR)/release
DEBUG_DIR := $(BUILD_DIR)/debug
DEBUG_TARGET := $(DEBUG_DIR)/$(TARGET)

SOURCES := $(sort $(wildcard src/*.c))
RELEASE_OBJECTS := $(patsubst src/%.c,$(RELEASE_DIR)/%.o,$(SOURCES))
DEBUG_OBJECTS := $(patsubst src/%.c,$(DEBUG_DIR)/%.o,$(SOURCES))
DEPENDENCIES := $(RELEASE_OBJECTS:.o=.d) $(DEBUG_OBJECTS:.o=.d)

COMMON_CFLAGS := -std=c99 -Wall -Wextra -Wpedantic
RELEASE_CFLAGS := -O3 -DNDEBUG
SANITIZERS := -fsanitize=address,undefined
DEBUG_CFLAGS := -O0 -g3 -DDEBUG $(SANITIZERS)

.DEFAULT_GOAL := all

.PHONY: all release debug run test clean help check-deps

all: release

release: $(TARGET)

debug: $(DEBUG_TARGET)

$(TARGET): $(RELEASE_OBJECTS) | check-deps
	$(CC) $(LDFLAGS) $(RELEASE_OBJECTS) -o $@ $(SDL_LIBS) $(LDLIBS) -lm
	@echo "Built release: $@"

$(DEBUG_TARGET): $(DEBUG_OBJECTS) | check-deps
	@mkdir -p $(@D)
	$(CC) $(LDFLAGS) $(SANITIZERS) $(DEBUG_OBJECTS) -o $@ $(SDL_LIBS) $(LDLIBS) -lm
	@echo "Built debug: $@"

$(RELEASE_DIR)/%.o: src/%.c | check-deps
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) -Iheaders $(SDL_CFLAGS) $(COMMON_CFLAGS) $(CFLAGS) $(RELEASE_CFLAGS) \
		-MMD -MP -MF $(@:.o=.d) -c $< -o $@

$(DEBUG_DIR)/%.o: src/%.c | check-deps
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) -Iheaders $(SDL_CFLAGS) $(COMMON_CFLAGS) $(CFLAGS) $(DEBUG_CFLAGS) \
		-MMD -MP -MF $(@:.o=.d) -c $< -o $@

check-deps:
	@$(PKG_CONFIG) --exists $(SDL_PACKAGES) || { \
		echo "error: SDL2 and SDL2_image development packages are required (pkg-config: $(SDL_PACKAGES))" >&2; \
		exit 1; \
	}

run: release
	./$(TARGET)

test: release debug
	SDL_VIDEODRIVER=dummy SDL_RENDER_DRIVER=software ./$(TARGET) --smoke-test
	SDL_VIDEODRIVER=dummy SDL_RENDER_DRIVER=software \
		ASAN_OPTIONS=halt_on_error=1:detect_leaks=0 UBSAN_OPTIONS=halt_on_error=1 \
		./$(DEBUG_TARGET) --smoke-test

clean:
	$(RM) -r $(BUILD_DIR)
	$(RM) src/*.o game game.exe

help:
	@echo "Available targets:"
	@echo "  all       Build the release executable (default)"
	@echo "  release   Build $(TARGET) with optimization"
	@echo "  debug     Build $(DEBUG_TARGET) with ASan and UBSan"
	@echo "  run       Build and run the release executable"
	@echo "  test      Run release and sanitizer headless smoke tests"
	@echo "  clean     Remove generated objects, dependencies, and executables"

-include $(DEPENDENCIES)
