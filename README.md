# CHIP-8 implementation (in C)

I want to one day be able to create a NES emulator, even if not a good
one. Since that is pretty much not possible right now, I decided to implement something simpler. I followed [these](https://multigesture.net/articles/how-to-write-an-emulator-chip-8-interpreter/) [two](https://tobiasvl.github.io/blog/write-a-chip-8-emulator/) tutorials while building this implementation of the CHIP-8 plus [this](http://devernay.free.fr/hacks/chip8/C8TECH10.HTM) nice technical reference.

This implementation was made using a nice little library called [Raylib](https://www.raylib.com/).

## Building and running

### On Windows

Download and install Raylib from the [website](https://www.raylib.com) or run the following batch script to download the dependencies:

```console
> ./getdeps.bat
```

After that, you should be able to compile the project with just:

```console
> make
```

Or:

```console
> mingw32-make
```

Whichever you've got to use (although this assumes that `mingw32-make` will be fine with a project without a `WinMain` entry point). The executable will be saved in a folder called `output`.<br>

To run this emulator, you may use the command line, like this

```console
> ./output/Chip8Win.exe <gamepath>
```

Or just drag and drop a CHIP-8 rom on the executable.

### On Linux

You might want to follow the instructions listed [here](https://github.com/raysan5/raylib/wiki/Working-on-GNU-Linux). If you don't see your distribution listed in the
[Install on GNU Linux](https://github.com/raysan5/raylib/wiki/Working-on-GNU-Linux#install-on-gnu-linux), you're gonna have to manually compile and install the `SHARED` version of Raylib.
After that, you should be able to compile the project with just:

```console
$ make
```

To run this emulator, you may use the command line like this

```console
$ ./output/Chip8Linux <gamepath>
```

## Some ROMS

You can find a lot of roms for the CHIP-8 in [this](https://github.com/AlexEne/rust-chip8) repository, which consists of yet another CHIP-8 implementation made by someone else, but in Rust!
