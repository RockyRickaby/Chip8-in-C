#include "chip8.h"
#include "chip8gui.h"

#include <stdio.h>
#include <string.h>
#include <stdint.h>

int main(int argc, char const *argv[])
{
    if (argc != 2) {
        printf("Usage: ./Chip8Win.exe path_to_game");
        return 1;
    }
    if (argc == 2) {
        guiInitAndRun(argv[1]);
    }
    return 0;
}
