#ifndef CHIP8_H
#define CHIP8_H

#include <stdio.h>
#include <stdint.h>

#define C8_ERR_NONE                 0
#define C8_ERR_STACK_OVERFLOW       1
#define C8_ERR_STACK_UNDERFLOW      2
#define C8_ERR_UNKNOWN_INS          3
#define C8_ERR_NO_ROM_LOADED        4
#define C8_ERR_ADDR_OUT_OF_BOUNDS   5

#define C8_STACK_SIZE               16
#define C8_MEMORY_SIZE              4096
#define C8_REGISTER_AMOUNT          16
#define C8_KEYS_AMOUNT              16

#define C8_SCREEN_HEIGHT            32
#define C8_SCREEN_WIDTH             64
#define C8_SCREEN_SIZE              (C8_SCREEN_WIDTH * C8_SCREEN_HEIGHT)

#define C8_CLOCK_SPEED              600
#define C8_TIMER_SPEED              60
#define C8_DEFAULT_CLOCK_SPEED      (1.0 / C8_CLOCK_SPEED)
#define C8_TIMER_CLOCK_SPEED        (1.0 / C8_TIMER_SPEED)

/**
 * 0x000 - 0x1FF = Chip 8 interpreter (will contain font set)
 * 0x050 - 0x0A0 = Used for the built in 4x5 pixel font set (0-F)
 * 0x200 - 0xFFF = Program ROM and working RAM
 */
typedef struct Chip8 {
    uint8_t err;
    uint8_t running;
    uint8_t waitingForKey;
    uint8_t delayTimer;             /* 60hz (60 times per second) timers */
    uint8_t soundTimer; 
    uint16_t opcode;                /* current opcode */
    uint16_t I;                     /* index register */
    uint16_t sp;                    /* stack pointer */ 
    uint16_t stack[C8_STACK_SIZE];  /* stack memory */ 
    uint8_t V[C8_REGISTER_AMOUNT];  /* V0-F registers */
    uint8_t incPcFlag;              /* flag that tells to increment the program counter */
    uint16_t pc;                    /* program counter */
    uint8_t memory[C8_MEMORY_SIZE]; /* ROM + RAM*/      // TODO - consider malloc'ing
    uint8_t drawFlag;               /* tells when to draw on the "screen" */
    uint8_t gfx[C8_SCREEN_SIZE];    /* screen */        // TODO - consider malloc'ing
    uint8_t key[C8_KEYS_AMOUNT];    /* keypad keys */
} Chip8;

int chip8Init(Chip8* chip8);
int chip8LoadRom(Chip8* chip8, const char* filename);
int chip8LoadFromArray(Chip8* chip8, uint8_t* data, size_t size);
int chip8EmulateCycle(Chip8* chip8);
int chip8DecrTimers(Chip8* chip8);
void chip8Destroy(Chip8* chip8);

/* 0-F = keys, lsb to msb. should be updated on both press and release*/
int chip8PressKeys(Chip8* chip8, uint16_t keysMask);
int chip8VMDump(const Chip8* chip8, FILE* outFile);

#endif /* CHIP8_H */