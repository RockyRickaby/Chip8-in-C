#ifndef CHIP8GUI_H
#define CHIP8GUI_H

#include "chip8.h"

typedef struct GameWindow GameWindow;

GameWindow* guiCreateGameWindow(Chip8* chip8, const char* windowName, const char* gamePath);
void guiFreeWindow(GameWindow* win);

void guiInitAndRun(const char* gamePath);
void guiRun(GameWindow* window);

#endif /* CHIP8GUI_H */