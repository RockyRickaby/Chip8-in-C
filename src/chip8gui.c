#include "raylib.h"
#include "chip8.h"
#include "chip8gui.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <signal.h>

#define MAX(a, b) ((a)>(b)? (a) : (b))
#define MIN(a, b) ((a)<(b)? (a) : (b))

// Might be useful if I ever try using raygui
extern size_t _dump_memory_arr(const uint8_t* mem, size_t memcap, char** out);
extern size_t _dump_stack(const uint16_t* stack, size_t stacksize, char** out);
extern size_t _dump_regs(const uint8_t* V, size_t registerAmount, char** out);
extern size_t _dump_keys(const uint8_t* V, size_t registerAmount, char** out);
extern size_t _dump_internal_regs(const Chip8* c8, char** out);

static inline void _draw_screen(GameWindow* win, RenderTexture2D rTexture);
static inline void _window_init(GameWindow* win);
static inline uint16_t _get_pressed_keys(void);

typedef struct GameWindow {
    char* windowName;
    char* gamePath;
    int windowWidth;
    int windowHeight;
    int integerScalingFactor;
    int gameWidth;
    int gameHeight;
    Chip8* vm;
} GameWindow;

GameWindow* guiCreateGameWindow(Chip8* chip8, const char* windowName, const char* gamePath) {
    GameWindow* win = calloc(1, sizeof(GameWindow));
    win->windowName = windowName;
    win->gamePath = gamePath;
    win->gameWidth = C8_SCREEN_WIDTH;
    win->gameHeight = C8_SCREEN_HEIGHT;
    win->vm = chip8;
    chip8Init(win->vm);
    return win;
}

void guiFreeWindow(GameWindow* win) {
    free(win);
}

void guiInitAndRun(const char* gamePath) {
    Chip8* vm = calloc(1 ,sizeof(Chip8));
    GameWindow* w = guiCreateGameWindow(vm, "Chip-8", gamePath);
    guiRun(w);
    guiFreeWindow(w);
    chip8Destroy(vm);
    free(vm);
}

void guiRun(GameWindow* window) {
    InitWindow(1, 1, window->windowName);
    _window_init(window);
    RenderTexture2D actualGame = LoadRenderTexture(window->gameWidth, window->gameHeight);
    
    window->windowHeight = GetScreenHeight();
    window->windowWidth = GetScreenWidth();

    float keyboardAcc = 0;
    chip8Init(window->vm);
    chip8LoadRom(window->vm, window->gamePath);
    while (!WindowShouldClose()) {
        float delta = GetFrameTime();
        keyboardAcc += delta;
        if (keyboardAcc >= C8_DEFAULT_CLOCK_SPEED) {
            uint16_t keys = _get_pressed_keys();
            chip8PressKeys(window->vm, keys);
            keyboardAcc = 0;
        }
        int insNum = (int) ceilf(delta * C8_CLOCK_SPEED);
        for (int i = 0; i < insNum; i++) {
            chip8EmulateCycle(window->vm);
        }

        int decrNum = (int) ceilf(delta * C8_TIMER_SPEED);
        for (int i = 0; i < decrNum; i++) {
            chip8DecrTimers(window->vm);
        }
        _draw_screen(window, actualGame);
    }
    UnloadRenderTexture(actualGame);
    CloseWindow();
}

/**
 * 
 * 
 * 
 */

static inline uint16_t _get_pressed_keys(void) {
    uint16_t mask = 0;
    mask |=
        (IsKeyDown(KEY_X)       << 0x0) | // 0
        (IsKeyDown(KEY_ONE)     << 0x1) | // 1
        (IsKeyDown(KEY_TWO)     << 0x2) | // 2
        (IsKeyDown(KEY_THREE)   << 0x3) | // 3

        (IsKeyDown(KEY_Q)       << 0x4) | // 4
        (IsKeyDown(KEY_W)       << 0x5) | // 5
        (IsKeyDown(KEY_E)       << 0x6) | // 6
        (IsKeyDown(KEY_A)       << 0x7) | // 7

        (IsKeyDown(KEY_S)       << 0x8) | // 8
        (IsKeyDown(KEY_D)       << 0x9) | // 9
        (IsKeyDown(KEY_Z)       << 0xA) | // A
        (IsKeyDown(KEY_C)       << 0xB) | // B

        (IsKeyDown(KEY_FOUR)    << 0xC) | // C
        (IsKeyDown(KEY_R)       << 0xD) | // D
        (IsKeyDown(KEY_F)       << 0xE) | // E
        (IsKeyDown(KEY_V)       << 0xF);  // F
    return mask;
}

static inline void _window_init(GameWindow* win) {
    SetWindowState(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);

    const int screenHeight = GetMonitorHeight(0);
    const int screenWidth = GetMonitorWidth(0);

    win->integerScalingFactor = MIN(screenHeight / win->gameHeight, screenWidth / win->gameWidth) / 2;

    SetWindowMinSize(win->gameWidth * 2, win->gameHeight * 2);
    SetWindowSize(
        win->gameWidth *  win->integerScalingFactor,
        win->gameHeight *  win->integerScalingFactor
    );
    SetWindowPosition(
        (screenWidth - (win->gameWidth *  win->integerScalingFactor)) / 2,
        (screenHeight - (win->gameHeight *  win->integerScalingFactor)) / 2
    );
}

static inline void _draw_screen(GameWindow* win, RenderTexture2D rTexture) {
    if (IsWindowResized()) {
        win->windowHeight = GetScreenHeight();
        win->windowWidth = GetScreenWidth();
        win->integerScalingFactor = MIN(win->windowHeight / win->gameHeight, win->windowWidth / win->gameWidth);
    }

    BeginTextureMode(rTexture);
        ClearBackground(BLACK);
        if (win->vm->running) {
            for (int i = 0; i < win->gameHeight; i++) {
                for (int j = 0; j < win->gameWidth; j++) {
                    DrawRectangle(j, i, 1, 1, win->vm->gfx[i * win->gameWidth + j] ? RAYWHITE : BLACK);
                }   
            }
        } else {
            DrawRectangle(0, 0, win->gameWidth, win->gameHeight, RAYWHITE);
            DrawText("uh oh", 15,15,1, BLACK);
        }
    EndTextureMode();

    BeginDrawing();
        ClearBackground(BLACK);
        // NOTE: OpenGL's (0,0) point is at the bottom left of the screen (I didn't know that),
        // which explains why the height's sign has to be flipped
        DrawTexturePro(
            rTexture.texture,
            (Rectangle){0, 0, rTexture.texture.width, -rTexture.texture.height},
            (Rectangle){
                (win->windowWidth - (win->gameWidth * win->integerScalingFactor)) / 2,
                (win->windowHeight - (win->gameHeight * win->integerScalingFactor)) / 2,
                win->gameWidth * win->integerScalingFactor,
                win->gameHeight * win->integerScalingFactor
            },
            (Vector2){0, 0},
            0,
            WHITE
        );
    EndDrawing();
}