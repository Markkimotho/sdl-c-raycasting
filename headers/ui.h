#ifndef UI_H
#define UI_H

#include <SDL2/SDL.h>

void uiDrawText(SDL_Renderer *renderer, int x, int y, int scale,
                const char *text, SDL_Color color);
int uiTextWidth(const char *text, int scale);
void uiDrawTextCentered(SDL_Renderer *renderer, int centerX, int y, int scale,
                        const char *text, SDL_Color color);

#endif /* UI_H */
