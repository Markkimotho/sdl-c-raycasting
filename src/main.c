#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../headers/global.h"

static void printUsage(const char *program)
{
    printf("Usage: %s [--map 1|2|3] [--torch on|off] [--smoke-test]\n", program);
    printf("          [--screenshot FILE.bmp] [--help]\n");
    printf("\nControls:\n");
    printf("  WASD          Move / strafe\n");
    printf("  Mouse/arrows  Look\n");
    printf("  Shift         Sprint\n");
    printf("  E             Interact\n");
    printf("  F             Toggle torch\n");
    printf("  M             Toggle minimap\n");
    printf("  N             Toggle day/night\n");
    printf("  Escape        Pause\n");
}

static int saveScreenshot(SDL_Renderer *renderer, const char *path)
{
    SDL_Surface *surface;
    int result;
    int outputWidth;
    int outputHeight;

    if (SDL_GetRendererOutputSize(renderer, &outputWidth, &outputHeight) != 0) {
        fprintf(stderr, "Unable to query screenshot size: %s\n", SDL_GetError());
        return -1;
    }
    surface = SDL_CreateRGBSurfaceWithFormat(0, outputWidth, outputHeight,
                                             32, SDL_PIXELFORMAT_RGBA32);
    if (!surface) {
        fprintf(stderr, "Unable to allocate screenshot surface: %s\n", SDL_GetError());
        return -1;
    }
    result = SDL_RenderReadPixels(renderer, NULL, SDL_PIXELFORMAT_RGBA32,
                                  surface->pixels, surface->pitch);
    if (result == 0) result = SDL_SaveBMP(surface, path);
    if (result != 0) {
        fprintf(stderr, "Unable to save screenshot '%s': %s\n", path, SDL_GetError());
    }
    SDL_FreeSurface(surface);
    return result;
}

static int parseMapArgument(const char *value)
{
    char *end = NULL;
    long number = strtol(value, &end, 10);
    if (!value[0] || !end || *end != '\0' || number < 1 || number > NUM_MAPS) {
        return -1;
    }
    return (int)number - 1;
}

int main(int argc, char **argv)
{
    Instance instance;
    int requestedMap = -1;
    int requestedTorch = -1;
    int smokeTest = 0;
    const char *screenshotPath = NULL;
    SDL_RendererInfo rendererInfo;
    int rendererHasVsync = 0;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            printUsage(argv[0]);
            return 0;
        }
        if (strcmp(argv[i], "--smoke-test") == 0) {
            smokeTest = 1;
            continue;
        }
        if (strcmp(argv[i], "--torch") == 0 && i + 1 < argc) {
            const char *value = argv[++i];
            if (strcmp(value, "on") == 0) requestedTorch = 1;
            else if (strcmp(value, "off") == 0) requestedTorch = 0;
            else {
                fprintf(stderr, "--torch expects 'on' or 'off'\n");
                return 2;
            }
            continue;
        }
        if (strcmp(argv[i], "--screenshot") == 0 && i + 1 < argc) {
            screenshotPath = argv[++i];
            smokeTest = 1;
            continue;
        }
        if (strcmp(argv[i], "--map") == 0 && i + 1 < argc) {
            requestedMap = parseMapArgument(argv[++i]);
            if (requestedMap < 0) {
                fprintf(stderr, "--map expects 1, 2, or 3\n");
                return 2;
            }
            continue;
        }
        fprintf(stderr, "Unknown option: %s\n", argv[i]);
        printUsage(argv[0]);
        return 2;
    }

    if (screenshotPath) {
        SDL_SetHint(SDL_HINT_RENDER_DRIVER, "software");
    }
    if (initializeSDL(&instance) != 0) return 1;
    if (loadTextures(&instance) != 0) {
        cleanupSDL(&instance);
        return 1;
    }

    gameReturnToTitle(&instance);
    if (requestedMap >= 0 && gameStart(&instance, requestedMap) != 0) {
        destroyTextures(&instance);
        cleanupSDL(&instance);
        return 1;
    }
    if (requestedTorch >= 0 && instance.game.mode == GAME_MODE_PLAYING) {
        instance.game.player.flashlightOn = requestedTorch;
    }

    if (smokeTest) {
        const Uint8 *keys;
        if (instance.game.mode == GAME_MODE_TITLE && gameStart(&instance, 0) != 0) {
            destroyTextures(&instance);
            cleanupSDL(&instance);
            return 1;
        }
        if (requestedTorch >= 0) {
            instance.game.player.flashlightOn = requestedTorch;
        }
        keys = SDL_GetKeyboardState(NULL);
        gameUpdate(&instance, keys, FIXED_TIMESTEP);
        if (screenshotPath) {
            renderDraw(&instance);
            if (saveScreenshot(instance.renderer, screenshotPath) != 0) {
                destroyTextures(&instance);
                cleanupSDL(&instance);
                return 1;
            }
            SDL_RenderPresent(instance.renderer);
        } else {
            renderFrame(&instance);
        }
        printf("Smoke test passed: SDL, assets, map, update, and renderer initialized.\n");
        destroyTextures(&instance);
        cleanupSDL(&instance);
        return 0;
    }

    if (SDL_GetRendererInfo(instance.renderer, &rendererInfo) == 0) {
        rendererHasVsync = (rendererInfo.flags & SDL_RENDERER_PRESENTVSYNC) != 0;
    }

    Uint64 frequency = SDL_GetPerformanceFrequency();
    Uint64 previousCounter = SDL_GetPerformanceCounter();
    double accumulator = 0.0;

    while (!instance.game.requestQuit) {
        Uint64 frameCounter = SDL_GetPerformanceCounter();
        double frameTime = (double)(frameCounter - previousCounter) / (double)frequency;
        const Uint8 *keys;
        SDL_Event event;

        previousCounter = frameCounter;
        if (frameTime > MAX_FRAME_TIME) frameTime = MAX_FRAME_TIME;

        while (SDL_PollEvent(&event)) {
            gameHandleEvent(&instance, &event);
            if (instance.game.requestQuit) break;
        }
        if (instance.game.requestQuit) break;
        keys = SDL_GetKeyboardState(NULL);
        if (!keys) {
            fprintf(stderr, "Unable to read keyboard state: %s\n", SDL_GetError());
            break;
        }

        if (instance.game.mode == GAME_MODE_PLAYING) {
            accumulator += frameTime;
            while (accumulator >= FIXED_TIMESTEP) {
                gameUpdate(&instance, keys, FIXED_TIMESTEP);
                accumulator -= FIXED_TIMESTEP;
            }
        } else {
            accumulator = 0.0;
        }

        renderFrame(&instance);
        if (!rendererHasVsync) SDL_Delay(1);
    }

    destroyTextures(&instance);
    cleanupSDL(&instance);
    return 0;
}
