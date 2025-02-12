#include "chip8.h"
#include "raylib.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>
#include <time.h>

#define C8_BEGIN_ADDRESS    0x200

#define C8_INS_HI(ins)      (((ins) & 0xF000U) >> 12)
#define C8_INS_LO(ins)      ((ins) & 0x000FU)

#define C8_EXTR_ADDR(ins)   ((ins) & 0x0FFFU)
#define C8_EXTR_NIBBLE(ins) ((ins) & 0x000FU)
#define C8_EXTR_X(ins)      (((ins) & 0x0F00U) >> 8)
#define C8_EXTR_Y(ins)      (((ins) & 0x00F0U) >> 4)
#define C8_EXTR_BYTE(ins)   ((ins) & 0x00FFU)


static const uint8_t _chip8FontSet[80] = { 
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

static void _op_unknown(Chip8* c8);
static void _op_nop(Chip8* c8);

static void _op0_cls(Chip8* c8);        /* 00E0 */
static void _op0_ret(Chip8* c8);        /* 00EE */
static void _op0_sys(Chip8* c8);        /* 0nnn */

static void _op1_jump_addr(Chip8* c8);          /* 1nnn */
static void _op2_call(Chip8* c8);               /* 2nnn */
static void _op3_skip_eq_byte(Chip8* c8);       /* 3xkk */
static void _op4_skip_neq_byte(Chip8* c8);      /* 4xkk */
static void _op5_skip_eq_reg(Chip8* c8);        /* 5xy0 */
static void _op6_load_byte(Chip8* c8);          /* 6xkk */
static void _op7_add_byte(Chip8* c8);           /* 7xkk  */

static void _op8_load_reg(Chip8* c8);           /* 8xy0 */
static void _op8_or(Chip8* c8);                 /* 8xy1 */
static void _op8_and(Chip8* c8);                /* 8xy2 */
static void _op8_xor(Chip8* c8);                /* 8xy3 */
static void _op8_add_reg(Chip8* c8);            /* 8xy4 */
static void _op8_sub_reg(Chip8* c8);            /* 8xy5 */
static void _op8_shiftr_reg(Chip8* c8);         /* 8xy6 */
static void _op8_sub_reversed_reg(Chip8* c8);   /* 8xy7 */
static void _op8_shiftl_reg(Chip8* c8);         /* 8xyE*/

static void _op9_skip_neq_reg(Chip8* c8);       /* 9xy0 */
static void _opA_load_I(Chip8* c8);             /* Annn */
static void _opB_jump_reg(Chip8* c8);           /* Bnnn */
static void _opC_rand(Chip8* c8);               /* Cxkk */
static void _opD_draw_sprite(Chip8* c8);        /* Dxyn */

static void _opE_skip_on_keypress(Chip8* c8);       /* Ex9E */
static void _opE_skip_on_keyrelease(Chip8* c8);     /* ExA1 */

static void _opF_load_delay_timer_toreg(Chip8* c8);             /* Fx07 */
static void _opF_load_keypress_and_wait(Chip8* c8);             /* Fx0A */
static void _opF_load_delay_timer_set(Chip8* c8);               /* Fx15 */
static void _opF_load_sound_timer_set(Chip8* c8);               /* Fx18 */
static void _opF_add_I_reg(Chip8* c8);                          /* Fx1E */
static void _opF_load_hex_sprite_for_value(Chip8* c8);          /* Fx29 */
static void _opF_store_bcd_rep_of_reg(Chip8* c8);               /* Fx33 */
static void _opF_store_regs_to_mem_starting_at_I(Chip8* c8);    /* Fx55 */
static void _opF_load_regs_from_mem_starting_at_I(Chip8* c8);   /* Fx65 */

static void _0prefix_ins(Chip8* c8);            /* 00E0 and 00EE - if statement */
static void _8prefix_ins(Chip8* c8);        
static void _Eprefix_ins(Chip8* c8);            /* Ex9E and ExA1 - if statement */ 
static void _Fprefix_ins(Chip8* c8);            /* if statement for functions that end on 5 */

static void (*_ins_arr[16])(Chip8*) = {
    _0prefix_ins,                   // 0

    _op1_jump_addr,                 // 1
    _op2_call,                      // 2
    _op3_skip_eq_byte,              // 3
    _op4_skip_neq_byte,             // 4
    _op5_skip_eq_reg,               // 5
    _op6_load_byte,                 // 6
    _op7_add_byte,                  // 7

    _8prefix_ins,           

    _op9_skip_neq_reg,              // 9
    _opA_load_I,                    // A
    _opB_jump_reg,                  // B
    _opC_rand,                      // C
    _opD_draw_sprite,               // D

    _Eprefix_ins,                   // E
    _Fprefix_ins                    // F
};  

static void (*_8ins_arr[16])(Chip8*) = {
    _op8_load_reg,                  // 0
    _op8_or,                        // 1
    _op8_and,                       // 2
    _op8_xor,                       // 3
    _op8_add_reg,                   // 4
    _op8_sub_reg,                   // 5
    _op8_shiftr_reg,                // 6
    _op8_sub_reversed_reg,          // 7
    _op_unknown,                    // 8
    _op_unknown,                    // 9
    _op_unknown,                    // A
    _op_unknown,                    // B
    _op_unknown,                    // C
    _op_unknown,                    // D
    _op8_shiftl_reg,                // E
    _op_unknown,                    // F
};

static void (*_Fins_arr[16])(Chip8*) = {
    _op_unknown,                    // 0
    _op_unknown,                    // 1
    _op_unknown,                    // 2
    _opF_store_bcd_rep_of_reg ,     // 3
    _op_unknown,                    // 4
    _op_unknown,                    // 5
    _op_unknown,                    // 6
    _opF_load_delay_timer_toreg,    // 7
    _opF_load_sound_timer_set,      // 8
    _opF_load_hex_sprite_for_value, // 9
    _opF_load_keypress_and_wait,    // A
    _op_unknown,                    // B
    _op_unknown,                    // C
    _op_unknown,                    // D
    _opF_add_I_reg,                 // E
    _op_unknown,                    // F
};

// ----------------------------------------------------------------------

/**
 * 
 * IMPLEMENTATION OF CHIP8 VM
 * 
 */

static size_t _dump_memory_arr(const uint8_t* mem, size_t memcap, char** out);
static size_t _dump_stack(const uint16_t* stack, size_t stacksize, char** out);
static size_t _dump_regs(const uint8_t* V, size_t registerAmount, char** out);
static size_t _dump_keys(const uint8_t* V, size_t registerAmount, char** out);
static size_t _dump_internal_regs(const Chip8* c8, char** out);

/**
 * Initializes the Chip8 virtual machine
 */
int chip8Init(Chip8* chip8) {
    if (!chip8) {
        return 0;
    }
    // TODO - consider malloc'ing the memory and gfx arrays
    memset(chip8, 0, sizeof(Chip8));
    chip8->pc = C8_BEGIN_ADDRESS;
    chip8->err = C8_ERR_NO_ROM_LOADED;
    memcpy(chip8->memory, _chip8FontSet, 80); /* initialize fontset */
    return 1;
}

/**
 * Loads ROM into VM's RAM
 */
int chip8LoadRom(Chip8* chip8, const char* filename) {
    if (!chip8 || !filename) {
        return 0;
    }
    FILE* file = fopen(filename, "rb");
    if (file == NULL) {
        return 0;
    }

    fseek(file, 0L, SEEK_END);
    long size = ftell(file);
    fseek(file, 0L, SEEK_SET);
    // rewind(file);

    if (size >= C8_MEMORY_SIZE) {
        fclose(file);
        return 0;
    }


    chip8Init(chip8); /* clean up the vm */
    chip8->running = 1;
    chip8->err = C8_ERR_NONE;

    uint8_t* buf = malloc(size);
    fread(buf, 1, size, file);
    memcpy(chip8->memory + C8_BEGIN_ADDRESS, buf, size);

    free(buf);
    fclose(file);
    return 1; 
}

/**
 * Loads program from a uint8_t array. Mostly useful for debugging.
 * Big endian pls
 */
int chip8LoadFromArray(Chip8* chip8, uint8_t* data, size_t size) {
    if (!chip8 || !data || size == 0 || size >= C8_MEMORY_SIZE) {
        return 0;
    }
    chip8Init(chip8);
    chip8->running = 1;
    chip8->err = C8_ERR_NONE;
    memcpy(chip8->memory + C8_BEGIN_ADDRESS, data, size);
    return 1;
}

/* stages: fetch, decode, execute */
int chip8EmulateCycle(Chip8* chip8) {
    if (!chip8 || !chip8->running) {
        return 0;
    }

    if (!chip8->waitingForKey) {
        /* fetch! */
        chip8->opcode = chip8->memory[chip8->pc] << 8 | chip8->memory[chip8->pc + 1];

        /* decode and exec! */
        _ins_arr[C8_INS_HI(chip8->opcode)](chip8);

        if (chip8->incPcFlag) {
            chip8->pc += 2;
            chip8->incPcFlag = 0;
        }
    }
    return 1;
}

int chip8DecrTimers(Chip8* chip8) {
    if (!chip8) {
        return 0;
    }
    if (chip8->delayTimer > 0) {
        chip8->delayTimer--;
    }
    if (chip8->soundTimer > 0) {
        // TODO -- handle sound, maybe somewhere else
        // printf("beep!!\\n");
        chip8->soundTimer--;
    }
    return 1;
}

void chip8Destroy(Chip8* chip8) {
    // TODO - consider implementing this
    if (chip8) {

    }
}

/* 0-F = keys, lsb to msb */
int chip8PressKeys(Chip8* chip8, uint16_t keysMask) {
    if (!chip8) {
        return 0;
    }
    uint8_t firstKeyPressRegistered = UINT8_MAX;
    uint8_t anyPressed = 0;
    for (int i = 0; i < C8_KEYS_AMOUNT; i++) {
        if ((keysMask & (0x8000 >> i))) {
            anyPressed = 1;
            if (firstKeyPressRegistered == UINT8_MAX) {
                firstKeyPressRegistered = 0xF - i;
            }
            chip8->key[0xF - i] = 1; /* key press */
        } else {
            chip8->key[0xF - i] = 0; /* key release */
        }
    }
    if (chip8->waitingForKey && anyPressed) {
        chip8->waitingForKey = 0;
        chip8->V[C8_EXTR_X(chip8->opcode)] = firstKeyPressRegistered;
    }
    return 1;
}

int chip8VMDump(const Chip8* chip8, FILE* outFile) {
    if (!chip8) {
        return 0;
    }

    char* memorybuf;
    char* gfxbuf;
    char* regsbuf;
    char* keysbuf;
    char* stackbuf;
    char* extrabuf;
    size_t memsize = _dump_memory_arr(chip8->memory, C8_MEMORY_SIZE, &memorybuf);
    size_t gfxsize = _dump_memory_arr(chip8->gfx, C8_SCREEN_SIZE, &gfxbuf);
    size_t regsize = _dump_regs(chip8->V, C8_REGISTER_AMOUNT, &regsbuf);
    size_t keyssize = _dump_keys(chip8->key, C8_KEYS_AMOUNT, &keysbuf);
    size_t stacksize = _dump_stack(chip8->stack, C8_STACK_SIZE, &stackbuf);
    size_t extrasize = _dump_internal_regs(chip8, &extrabuf);

    // fputs(stackbuf, outFile);
    const char* vmmem = "VM Memory:\n";
    const char* scr = "Screen Buffer:\n";
    const char* regs = "Registers [V0-VF]:\n";
    const char* keys = "Keys [0-F]:\n";
    const char* stack = "Program Stack:\n";
    const char* regsAndFlags = "Other registers and flags:\n";
    const char* newlines = "\n\n";

    fputs(vmmem, outFile);
    fputs(memorybuf, outFile);
    fputs(newlines, outFile);

    fputs(stack, outFile);
    fputs(stackbuf, outFile);
    fputs(newlines, outFile);

    fputs(regs, outFile);
    fputs(regsbuf, outFile);
    fputs(newlines, outFile);

    fputs(keys, outFile);
    fputs(keysbuf, outFile);
    fputs(newlines, outFile);

    fputs(scr, outFile);
    fputs(gfxbuf, outFile);
    fputs(newlines, outFile);

    fputs(regsAndFlags, outFile);
    fputs(extrabuf, outFile);

    if (memsize) free(memorybuf);
    if (gfxsize) free(gfxbuf);
    if (regsize) free(regsbuf);
    if (keyssize) free(keysbuf);
    if (stacksize) free(stackbuf);
    if (extrasize) free(extrabuf);
    return 1;
}

// ----------------------------------------------------------------------

/**
 * Auxiliary
 */

static size_t _dump_internal_regs(const Chip8* c8, char** out) {
    char* buf = calloc(150, sizeof(char));
    if (!buf) {
        *out = "";
        return 0;
    }
    char tp[40] = {[39] = 0};
    int i = snprintf(tp, 40, "PC=0x%04X\n", c8->pc);
    strcat(buf, tp);

    i += snprintf(tp, 40, "opcode=0x%04X\n", c8->opcode);
    strcat(buf, tp);

    i += snprintf(tp, 40, "SP=0x%04X\n", c8->sp);
    strcat(buf, tp);

    i += snprintf(tp, 40, "I=0x%04X\n", c8->I);
    strcat(buf, tp);

    i += snprintf(tp, 40, "DT=0x%04X\n", c8->delayTimer);
    strcat(buf, tp);

    i += snprintf(tp, 40, "ST=0x%04X\n", c8->soundTimer);
    strcat(buf, tp);

    i += snprintf(tp, 40, "err=%d\n", c8->err);
    strcat(buf, tp);

    char* tmp = realloc(buf, i);
    if (!tmp) {
        free(buf);
        *out = "";
        return 0;
    }
    *out = tmp;
    return (size_t) i;
}

static size_t _dump_regs(const uint8_t* V, size_t registerAmount, char** out) {
    size_t written = 0;
    size_t cap = (registerAmount * 10);
    char* regbuf = calloc(cap, sizeof(char));
    if (!regbuf) {
        *out = "";
        return 0;
    }

    for (int i = 0; i < registerAmount; i++) {
        char tmp[20] = {[19] = 0};
        int c = snprintf(tmp, 20, "[V%X]=0x%02X\n", i, V[i]);
        written += c;
        if (i == 0xF) {
            tmp[c] = '\0';
        }
        strncat(regbuf, tmp, c);
    }
    regbuf[cap - 1] = '\0';
    written -= 1;
    *out = regbuf;
    return written;
}

static size_t _dump_keys(const uint8_t* V, size_t registerAmount, char** out) {
    size_t written = 0;
    size_t cap = (registerAmount * 2) + (registerAmount * 5);
    char* regbuf = calloc(cap, 1);
    if (!regbuf) {
        *out = "";
        return 0;
    }

    for (int i = 0; i < registerAmount; i++) {
        char tmp[9] = {[8] = 0};
        written += snprintf(tmp, 9, "[k%X]=%d\n", i, V[i]);
        if (i == 0xF) {
            tmp[7] = '\0';
        }
        strncat(regbuf, tmp, 9);
    }
    regbuf[cap - 1] = '\0';
    written -= 1;
    *out = regbuf;
    return written;
}

static size_t _dump_stack(const uint16_t* stack, size_t stacksize, char** out) {
    size_t written = 0;
    size_t cap = (stacksize * 11);
    char* stackbuf = calloc(cap, sizeof(char));
    if (!stackbuf) {
        *out = "";
        return 0;
    }

    for (int i = stacksize - 1; i >= 0; i--) {
        char tmp[20] = {[19] = 0};
        int c = snprintf(tmp, 20, "[%X]=0x%04X\n", i, stack[i]);
        written += c;
        if (i == 0) {
            tmp[c] = '\0';
        }
        strncat(stackbuf, tmp, c);
    }

    stackbuf[cap - 1] = '\0';
    written -= 1;
    *out = stackbuf;
    return written;
}

// horrible. could've been written better
static size_t _dump_memory_arr(const uint8_t* mem, size_t memcap, char** out) {
    size_t written = 0;
    size_t cap = (memcap * 3) + (memcap / 16 * 5) + (5 + 3 * 16) + 1;
    char* memorybuf = calloc(cap, sizeof(char));
    if (!memorybuf) {
        *out = "";
        return 0;
    }

    written += sprintf(memorybuf, "     ");
    for (int i = 0; i < 16; i++) {
        char tmp[4] = {[3] = 0};
        written += snprintf(tmp, 4, "%02X ", i);
        strcat(memorybuf, tmp);
    }
    written += 1;
    strcat(memorybuf, "\n");
    int k = 0;
    for (int i = 0; i < memcap; i++) {
        k++;
        char tmp[5] = {[4] = 0};
        char addr[6] = {[5] = 0};
        if (i % 16 == 0) {
            written += snprintf(addr, 6, "%03X  ", i);
            strcat(memorybuf, addr);
        }
        written += snprintf(tmp, 5, "%02X ", mem[i]);
        if (k == 16) {
            tmp[2] = '\n';
            k = 0;
        }
        strcat(memorybuf, tmp);
    }
    memorybuf[cap - 1] = '\0';
    written -= 1;
    *out = memorybuf;
    return written;
}

/**
 * Instructions implementation
 * 
 */

static void _0prefix_ins(Chip8* c8) {
    switch (C8_EXTR_NIBBLE(c8->opcode)) {
        case 0x0: _op0_cls(c8); break;
        case 0xE: _op0_ret(c8); break;
        default:
            _op0_sys(c8);
    }
} 

static void _8prefix_ins(Chip8* c8) {
    _8ins_arr[C8_EXTR_NIBBLE(c8->opcode)](c8);
} 

static void _Eprefix_ins(Chip8* c8) {
    switch (C8_EXTR_NIBBLE(c8->opcode)) {
        case 0xE: _opE_skip_on_keypress(c8);    break;
        case 0x1: _opE_skip_on_keyrelease(c8);  break;
        default:
            _op_unknown(c8);
    }
}  

static void _Fprefix_ins(Chip8* c8) {
    uint16_t lo = C8_EXTR_NIBBLE(c8->opcode);
    if (lo == 0x5) {
        switch (C8_EXTR_Y(c8->opcode)) {
            case 1: _opF_load_delay_timer_set(c8);              break;
            case 5: _opF_store_regs_to_mem_starting_at_I(c8);   break;
            case 6: _opF_load_regs_from_mem_starting_at_I(c8);  break;
            default:
                _op_unknown(c8);
        }
    } else {
        _Fins_arr[lo](c8);
    }
}

static void _op_unknown(Chip8* c8) {
    printf("Unknown opcode. [0x%X]\n", c8->opcode);
    c8->err = C8_ERR_UNKNOWN_INS;
    c8->running = 0;
    c8->incPcFlag = 0;
}

static void _op_nop(Chip8* c8) {
    printf("nop... [0x%X]\n", c8->opcode);
    c8->incPcFlag = 1;
}

static void _op0_cls(Chip8* c8) {
    memset(c8->gfx, 0, C8_SCREEN_SIZE);
    c8->incPcFlag = 1;
}

static void _op0_ret(Chip8* c8) {
    if (c8->sp == 0) {
        c8->err = C8_ERR_STACK_UNDERFLOW;
        c8->running = 0;
        c8->incPcFlag = 0;
    } else {
        c8->sp -= 1;
        c8->pc = c8->stack[c8->sp];
        c8->incPcFlag = 1;
    }
}

static void _op0_sys(Chip8* c8) {
    printf("nop... [0x%X]\n", c8->opcode);
    c8->incPcFlag = 1;
}

static void _op1_jump_addr(Chip8* c8) {
    uint16_t addr = C8_EXTR_ADDR(c8->opcode);
    if (addr >= C8_MEMORY_SIZE) {
        c8->running = 0;
        c8->err = C8_ERR_ADDR_OUT_OF_BOUNDS;
        c8->incPcFlag = 0;
        return;
    }
    c8->pc = addr;
    c8->incPcFlag = 0;
}

static void _op2_call(Chip8* c8) {
    if (c8->sp == C8_STACK_SIZE) {
        c8->err = C8_ERR_STACK_OVERFLOW;
        c8->running = 0;
    } else {
        uint16_t addr = C8_EXTR_ADDR(c8->opcode);
        if (addr >= C8_MEMORY_SIZE) {
            c8->running = 0;
            c8->err = C8_ERR_ADDR_OUT_OF_BOUNDS;
        } else {
            c8->stack[c8->sp] = c8->pc;
            c8->sp += 1;
            c8->pc = addr;
        }
    }
    c8->incPcFlag = 0;
}

static void _op3_skip_eq_byte(Chip8* c8) {
    uint8_t byte = C8_EXTR_BYTE(c8->opcode);
    uint8_t vx = c8->V[C8_EXTR_X(c8->opcode)];
    if (vx == byte) {
        c8->pc += 2;
    }
    c8->incPcFlag = 1;
}

static void _op4_skip_neq_byte(Chip8* c8) {
    uint8_t byte = C8_EXTR_BYTE(c8->opcode);
    uint8_t vx = c8->V[C8_EXTR_X(c8->opcode)];
    if (vx != byte) {
        c8->pc += 2;
    }
    c8->incPcFlag = 1;
}

static void _op5_skip_eq_reg(Chip8* c8) {
    uint8_t vx = c8->V[C8_EXTR_X(c8->opcode)];
    uint8_t vy = c8->V[C8_EXTR_Y(c8->opcode)];
    if (vx == vy) {
        c8->pc += 2;
    }
    c8->incPcFlag = 1;
}

static void _op6_load_byte(Chip8* c8) {
    uint8_t byte = C8_EXTR_BYTE(c8->opcode);
    c8->V[C8_EXTR_X(c8->opcode)] = byte;
    c8->incPcFlag = 1;
}

static void _op7_add_byte(Chip8* c8) {
    uint8_t byte = C8_EXTR_BYTE(c8->opcode);
    c8->V[C8_EXTR_X(c8->opcode)] += byte;
    c8->incPcFlag = 1;
}


static void _op8_load_reg(Chip8* c8) {
    c8->V[C8_EXTR_X(c8->opcode)] = c8->V[C8_EXTR_Y(c8->opcode)];
    c8->incPcFlag = 1;
}

static void _op8_or(Chip8* c8) {
    c8->V[C8_EXTR_X(c8->opcode)] |= c8->V[C8_EXTR_Y(c8->opcode)];
    c8->incPcFlag = 1;
}

static void _op8_and(Chip8* c8) {
    c8->V[C8_EXTR_X(c8->opcode)] &= c8->V[C8_EXTR_Y(c8->opcode)];
    c8->incPcFlag = 1;
}

static void _op8_xor(Chip8* c8) {
    c8->V[C8_EXTR_X(c8->opcode)] ^= c8->V[C8_EXTR_Y(c8->opcode)];
    c8->incPcFlag = 1;
}

static void _op8_add_reg(Chip8* c8) {
    if (c8->V[C8_EXTR_X(c8->opcode)] > UINT8_MAX - c8->V[C8_EXTR_Y(c8->opcode)]) {
        c8->V[0xF] = 1;
    } else {
        c8->V[0xF] = 0;
    }
    c8->V[C8_EXTR_X(c8->opcode)] += c8->V[C8_EXTR_Y(c8->opcode)];
    c8->incPcFlag = 1;
}

static void _op8_sub_reg(Chip8* c8) {
    if (c8->V[C8_EXTR_X(c8->opcode)] > c8->V[C8_EXTR_Y(c8->opcode)]) {
        c8->V[0xF] = 1;
    } else {
        c8->V[0xF] = 0;
    }
    c8->V[C8_EXTR_X(c8->opcode)] -= c8->V[C8_EXTR_Y(c8->opcode)];
    c8->incPcFlag = 1;
}

static void _op8_shiftr_reg(Chip8* c8) {
    c8->V[0xF] = c8->V[C8_EXTR_X(c8->opcode)] & 0x1;
    c8->V[C8_EXTR_X(c8->opcode)] >>= 1;
    c8->incPcFlag = 1;
}

static void _op8_sub_reversed_reg(Chip8* c8) {
    if (c8->V[C8_EXTR_Y(c8->opcode)] > c8->V[C8_EXTR_X(c8->opcode)] ) {
        c8->V[0xF] = 1;
    } else {
        c8->V[0xF] = 0;
    }
    c8->V[C8_EXTR_X(c8->opcode)] = c8->V[C8_EXTR_Y(c8->opcode)] - c8->V[C8_EXTR_X(c8->opcode)] ;
    c8->incPcFlag = 1;
}

static void _op8_shiftl_reg(Chip8* c8) {
    c8->V[0xF] = (c8->V[C8_EXTR_X(c8->opcode)] & 0x80) >> 7;
    c8->V[C8_EXTR_X(c8->opcode)] <<= 1;
    c8->incPcFlag = 1;
}


static void _op9_skip_neq_reg(Chip8* c8) {
    uint8_t vx = c8->V[C8_EXTR_X(c8->opcode)];
    uint8_t vy = c8->V[C8_EXTR_Y(c8->opcode)];
    if (vx != vy) {
        c8->pc += 2;
    }
    c8->incPcFlag = 1;
}

static void _opA_load_I(Chip8* c8) {
    uint16_t addr = C8_EXTR_ADDR(c8->opcode);
    if (addr >= C8_MEMORY_SIZE) {
        c8->running = 0;
        c8->err = C8_ERR_ADDR_OUT_OF_BOUNDS;
        c8->incPcFlag = 0;
        return;
    }
    c8->I = addr;
    c8->incPcFlag = 1;
}

static void _opB_jump_reg(Chip8* c8) {
    uint16_t addr = C8_EXTR_ADDR(c8->opcode) + c8->V[0];
    if (addr >= C8_MEMORY_SIZE) {
        c8->running = 0;
        c8->err = C8_ERR_ADDR_OUT_OF_BOUNDS;
        c8->incPcFlag = 0;
        return;
    }
    c8->pc = addr;
    c8->incPcFlag = 0;
}

static void _opC_rand(Chip8* c8) {
    SetRandomSeed(time(NULL));
    c8->V[C8_EXTR_X(c8->opcode)] =((uint8_t) GetRandomValue(0, 255)) & C8_EXTR_BYTE(c8->opcode);
    c8->incPcFlag = 1;
}

static void _opD_draw_sprite(Chip8* c8) {
    uint8_t x = c8->V[C8_EXTR_X(c8->opcode)];
    uint8_t y = c8->V[C8_EXTR_Y(c8->opcode)];
    uint8_t height = C8_EXTR_NIBBLE(c8->opcode);

    c8->V[0xF] = 0;
    for (int yln = 0; yln < height && y + yln < C8_SCREEN_HEIGHT; yln++) {
        uint8_t pixel = c8->memory[c8->I + yln];

        for (int xln = 0; xln < 8 && x + xln < C8_SCREEN_WIDTH; xln++) {
            int idx = (x + xln + ((y + yln) * 64));

            if ((pixel & (0x80U >> xln))) {
                if (c8->gfx[idx]) {
                    c8->V[0xF] = 1;
                }
                c8->gfx[idx] ^= 1;
            }
        }
    }
    c8->drawFlag = 1;
    c8->incPcFlag = 1;
}


static void _opE_skip_on_keypress(Chip8* c8) {
    if (c8->key[c8->V[C8_EXTR_X(c8->opcode)]] == 1) {
        c8->pc += 2;
    }
    c8->incPcFlag = 1;
}

static void _opE_skip_on_keyrelease(Chip8* c8) {
    if (c8->key[c8->V[C8_EXTR_X(c8->opcode)]] == 0) {
        c8->pc += 2;
    }
    c8->incPcFlag = 1;
}


static void _opF_load_delay_timer_toreg(Chip8* c8) {
    c8->V[C8_EXTR_X(c8->opcode)] = c8->delayTimer;
    c8->incPcFlag = 1;
}

static void _opF_load_keypress_and_wait(Chip8* c8) {
    c8->waitingForKey = 1;
    c8->incPcFlag = 1;
}

static void _opF_load_delay_timer_set(Chip8* c8) {
    c8->delayTimer = c8->V[C8_EXTR_X(c8->opcode)];
    c8->incPcFlag = 1;
}

static void _opF_load_sound_timer_set(Chip8* c8) {
    c8->soundTimer = c8->V[C8_EXTR_X(c8->opcode)];
    c8->incPcFlag = 1;
}

static void _opF_add_I_reg(Chip8* c8) {
    if (c8->I + c8->V[C8_EXTR_X(c8->opcode)] > 0xFFF) {
        c8->V[0xF] = 1;
    } else {
        c8->V[0xF] = 0;
    }
    c8->I += c8->V[C8_EXTR_X(c8->opcode)];
    c8->incPcFlag = 1;
}

static void _opF_load_hex_sprite_for_value(Chip8* c8) {
    c8->I = c8->V[C8_EXTR_X(c8->opcode)] * 5;
    c8->incPcFlag = 1;
}

static void _opF_store_bcd_rep_of_reg(Chip8* c8) {
    uint8_t vx = c8->V[C8_EXTR_X(c8->opcode)];
    c8->memory[c8->I]       = vx / 100;         /* hundreds */
    c8->memory[c8->I + 1]   = vx % 100 / 10;    /* tens */
    c8->memory[c8->I + 2]   = vx % 10;          /* ones */
    c8->incPcFlag = 1;
}

static void _opF_store_regs_to_mem_starting_at_I(Chip8* c8) {
    memcpy(c8->memory + c8->I, c8->V, C8_EXTR_X(c8->opcode) + 1);
    c8->I = c8->I + C8_EXTR_X(c8->opcode) + 1;
    c8->incPcFlag = 1;
}

static void _opF_load_regs_from_mem_starting_at_I(Chip8* c8) {
    memcpy(c8->V, c8->memory + c8->I, C8_EXTR_X(c8->opcode) + 1);    
    c8->I = c8->I + C8_EXTR_X(c8->opcode) + 1;
    c8->incPcFlag = 1;
}
