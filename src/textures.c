#include <math.h>
#include <stdio.h>
#include <string.h>

#include <SDL2/SDL_image.h>

#include "../headers/global.h"

static SDL_Surface *loadSurface(const Instance *instance, const char *relativePath)
{
    SDL_Surface *surface = NULL;
    char fullPath[1536];

    if (instance->basePath[0] != '\0') {
        int written = snprintf(fullPath, sizeof(fullPath), "%s%s",
                               instance->basePath, relativePath);
        if (written > 0 && (size_t)written < sizeof(fullPath)) {
            surface = IMG_Load(fullPath);
        }
        /* The sanitizer binary lives under build/debug during development. */
        if (!surface) {
            written = snprintf(fullPath, sizeof(fullPath), "%s../../%s",
                               instance->basePath, relativePath);
            if (written > 0 && (size_t)written < sizeof(fullPath)) {
                surface = IMG_Load(fullPath);
            }
        }
    }
    if (!surface) surface = IMG_Load(relativePath);

    if (!surface) {
        fprintf(stderr, "Unable to load '%s': %s\n", relativePath, IMG_GetError());
    }
    return surface;
}

static Uint32 surfacePixelAt(const SDL_Surface *surface, int x, int y)
{
    const Uint8 *pixel = (const Uint8 *)surface->pixels + y * surface->pitch +
                         x * surface->format->BytesPerPixel;

    switch (surface->format->BytesPerPixel) {
        case 1:
            return *pixel;
        case 2:
            return *(const Uint16 *)pixel;
        case 3:
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
            return ((Uint32)pixel[0] << 16) | ((Uint32)pixel[1] << 8) | pixel[2];
#else
            return pixel[0] | ((Uint32)pixel[1] << 8) | ((Uint32)pixel[2] << 16);
#endif
        default:
            return *(const Uint32 *)pixel;
    }
}

static void findVisibleBounds(SDL_Surface *surface, TextureAsset *asset,
                              int blackColorKey)
{
    int found = 0;

    asset->contentLeft = surface->w;
    asset->contentTop = surface->h;
    asset->contentRight = -1;
    asset->contentBottom = -1;

    if (SDL_MUSTLOCK(surface) && SDL_LockSurface(surface) != 0) {
        asset->contentLeft = 0;
        asset->contentTop = 0;
        asset->contentRight = surface->w - 1;
        asset->contentBottom = surface->h - 1;
        return;
    }

    for (int y = 0; y < surface->h; ++y) {
        for (int x = 0; x < surface->w; ++x) {
            Uint8 red;
            Uint8 green;
            Uint8 blue;
            Uint8 alpha;
            Uint32 pixel = surfacePixelAt(surface, x, y);
            int visible;

            SDL_GetRGBA(pixel, surface->format, &red, &green, &blue, &alpha);
            visible = blackColorKey ? (red != 0 || green != 0 || blue != 0) :
                                      alpha > 8;
            if (!visible) continue;
            if (x < asset->contentLeft) asset->contentLeft = x;
            if (y < asset->contentTop) asset->contentTop = y;
            if (x > asset->contentRight) asset->contentRight = x;
            if (y > asset->contentBottom) asset->contentBottom = y;
            found = 1;
        }
    }

    if (SDL_MUSTLOCK(surface)) SDL_UnlockSurface(surface);
    if (!found) {
        asset->contentLeft = 0;
        asset->contentTop = 0;
        asset->contentRight = surface->w - 1;
        asset->contentBottom = surface->h - 1;
    }
}

static int loadTextureAsset(Instance *instance, TextureAsset *asset,
                            const char *path, int blackColorKey)
{
    SDL_Surface *surface;

    surface = loadSurface(instance, path);
    if (!surface) return -1;

    if (blackColorKey) {
        Uint32 key = SDL_MapRGB(surface->format, 0, 0, 0);
        if (SDL_SetColorKey(surface, SDL_TRUE, key) != 0) {
            fprintf(stderr, "Unable to set color key for '%s': %s\n",
                    path, SDL_GetError());
            SDL_FreeSurface(surface);
            return -1;
        }
    }

    asset->width = surface->w;
    asset->height = surface->h;
    findVisibleBounds(surface, asset, blackColorKey);
    asset->texture = SDL_CreateTextureFromSurface(instance->renderer, surface);
    SDL_FreeSurface(surface);
    if (!asset->texture) {
        fprintf(stderr, "Unable to create texture for '%s': %s\n", path, SDL_GetError());
        memset(asset, 0, sizeof(*asset));
        return -1;
    }

    SDL_SetTextureBlendMode(asset->texture, SDL_BLENDMODE_BLEND);
    return 0;
}

static SDL_Texture *createLightingMask(SDL_Renderer *renderer, int vignetteOnly)
{
    SDL_Surface *surface;
    SDL_Texture *texture;

    surface = SDL_CreateRGBSurfaceWithFormat(0, LOGICAL_WIDTH, LOGICAL_HEIGHT,
                                             32, SDL_PIXELFORMAT_RGBA32);
    if (!surface) return NULL;

    if (SDL_MUSTLOCK(surface) && SDL_LockSurface(surface) != 0) {
        SDL_FreeSurface(surface);
        return NULL;
    }

    for (int y = 0; y < LOGICAL_HEIGHT; ++y) {
        Uint32 *row = (Uint32 *)((Uint8 *)surface->pixels + y * surface->pitch);
        for (int x = 0; x < LOGICAL_WIDTH; ++x) {
            float normalizedX = ((float)x - LOGICAL_WIDTH * 0.5f) /
                                (LOGICAL_WIDTH * (vignetteOnly ? 0.64f : 0.31f));
            float normalizedY = ((float)y - LOGICAL_HEIGHT * (vignetteOnly ? 0.48f : 0.47f)) /
                                (LOGICAL_HEIGHT * (vignetteOnly ? 0.72f : 0.43f));
            float radius = sqrtf(normalizedX * normalizedX + normalizedY * normalizedY);
            float alpha;

            if (vignetteOnly) {
                alpha = (radius - 0.55f) * 155.0f;
                if (alpha < 0.0f) alpha = 0.0f;
                if (alpha > 92.0f) alpha = 92.0f;
            } else {
                float edge = (radius - 0.18f) / 0.72f;
                if (edge < 0.0f) edge = 0.0f;
                if (edge > 1.0f) edge = 1.0f;
                alpha = 236.0f * edge * edge * (3.0f - 2.0f * edge);
            }
            row[x] = SDL_MapRGBA(surface->format, 0, 0, 0, (Uint8)alpha);
        }
    }

    if (SDL_MUSTLOCK(surface)) SDL_UnlockSurface(surface);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    if (texture) SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    return texture;
}

static SDL_Texture *createTorchGlow(SDL_Renderer *renderer)
{
    SDL_Surface *surface;
    SDL_Texture *texture;

    surface = SDL_CreateRGBSurfaceWithFormat(0, LOGICAL_WIDTH, LOGICAL_HEIGHT,
                                             32, SDL_PIXELFORMAT_RGBA32);
    if (!surface) return NULL;
    if (SDL_MUSTLOCK(surface) && SDL_LockSurface(surface) != 0) {
        SDL_FreeSurface(surface);
        return NULL;
    }

    for (int y = 0; y < LOGICAL_HEIGHT; ++y) {
        Uint32 *row = (Uint32 *)((Uint8 *)surface->pixels + y * surface->pitch);
        for (int x = 0; x < LOGICAL_WIDTH; ++x) {
            float normalizedX = ((float)x - LOGICAL_WIDTH * 0.5f) /
                                (LOGICAL_WIDTH * 0.28f);
            float normalizedY = ((float)y - LOGICAL_HEIGHT * 0.48f) /
                                (LOGICAL_HEIGHT * 0.38f);
            float radius = sqrtf(normalizedX * normalizedX + normalizedY * normalizedY);
            float intensity = 1.0f - radius;
            Uint8 alpha;

            if (intensity < 0.0f) intensity = 0.0f;
            intensity *= intensity;
            alpha = (Uint8)(intensity * 52.0f);
            row[x] = SDL_MapRGBA(surface->format, 255, 191, 105, alpha);
        }
    }

    if (SDL_MUSTLOCK(surface)) SDL_UnlockSurface(surface);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    if (texture) SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_ADD);
    return texture;
}

int loadTextures(Instance *instance)
{
    static const char *wallFiles[MAX_WALL_TYPES] = {
        NULL,
        "assets/colorstone.png",
        "assets/redbrick.png",
        "assets/purplestone.png",
        "assets/wood.png",
        "assets/rocky.png"
    };
    static const char *objectFiles[MAX_OBJECT_TEXTURES] = {
        "assets/generated/storage_barrel.png",
        "assets/generated/industrial_pillar.png",
        "assets/greenlight.png",
        "assets/generated/sentinel.png",
        "assets/generated/power_cell.png"
    };

    if (!instance || !instance->renderer) return -1;
    destroyTextures(instance);
    memset(instance->wallTextures, 0, sizeof(instance->wallTextures));
    memset(instance->objectTextures, 0, sizeof(instance->objectTextures));
    memset(&instance->povHands, 0, sizeof(instance->povHands));

    for (int i = 1; i < MAX_WALL_TYPES; ++i) {
        if (loadTextureAsset(instance, &instance->wallTextures[i], wallFiles[i], 0) != 0) {
            destroyTextures(instance);
            return -1;
        }
    }
    for (int i = 0; i < MAX_OBJECT_TEXTURES; ++i) {
        int legacyBlackBackground = i == OBJECT_TEXTURE_LIGHT;
        if (loadTextureAsset(instance, &instance->objectTextures[i], objectFiles[i],
                             legacyBlackBackground) != 0) {
            destroyTextures(instance);
            return -1;
        }
    }
    if (loadTextureAsset(instance, &instance->povHands,
                         "assets/generated/pov_hands_flashlight.png", 0) != 0) {
        destroyTextures(instance);
        return -1;
    }

    instance->flashlightMask = createLightingMask(instance->renderer, 0);
    instance->torchGlow = createTorchGlow(instance->renderer);
    instance->vignette = createLightingMask(instance->renderer, 1);
    if (!instance->flashlightMask || !instance->torchGlow || !instance->vignette) {
        fprintf(stderr, "Unable to create screen-space lighting textures: %s\n",
                SDL_GetError());
        destroyTextures(instance);
        return -1;
    }
    return 0;
}

void destroyTextures(Instance *instance)
{
    if (!instance) return;

    for (int i = 0; i < MAX_WALL_TYPES; ++i) {
        if (instance->wallTextures[i].texture) {
            SDL_DestroyTexture(instance->wallTextures[i].texture);
            memset(&instance->wallTextures[i], 0, sizeof(instance->wallTextures[i]));
        }
    }
    for (int i = 0; i < MAX_OBJECT_TEXTURES; ++i) {
        if (instance->objectTextures[i].texture) {
            SDL_DestroyTexture(instance->objectTextures[i].texture);
            memset(&instance->objectTextures[i], 0, sizeof(instance->objectTextures[i]));
        }
    }
    if (instance->povHands.texture) {
        SDL_DestroyTexture(instance->povHands.texture);
        memset(&instance->povHands, 0, sizeof(instance->povHands));
    }
    if (instance->flashlightMask) {
        SDL_DestroyTexture(instance->flashlightMask);
        instance->flashlightMask = NULL;
    }
    if (instance->torchGlow) {
        SDL_DestroyTexture(instance->torchGlow);
        instance->torchGlow = NULL;
    }
    if (instance->vignette) {
        SDL_DestroyTexture(instance->vignette);
        instance->vignette = NULL;
    }
}
