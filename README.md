# CHIP-8 implementation (in C)

I want to one day be able to create a NES emulator, even if not a good
one. Since that is pretty much not possible right now, I decided to implement something simpler. I followed [these](https://multigesture.net/articles/how-to-write-an-emulator-chip-8-interpreter/) [two](https://tobiasvl.github.io/blog/write-a-chip-8-emulator/) tutorials while building this implementation of the CHIP-8 plus [this](http://devernay.free.fr/hacks/chip8/C8TECH10.HTM) nice technical reference.

This implementation was made using a nice little library called [Raylib](https://www.raylib.com/).

Until the project is made somewhat compiler agnostic, you're gonna need GCC to compile it. A version of Mingw GCC comes with the Raylib installer for Windows.

## Building and running

### On Windows

Download and install Raylib from the [website](https://www.raylib.com) or run the following batch script (preferably using the Command Prompt) to download the dependencies:

```console
> .\getdeps.bat
```

In case that doesn't work, try running the PowerShell script (inside PowerShell):

```console
> .\getdeps.ps1                                                     # This might work just fine
or
> powershell -ExecutionPolicy Bypass -File .\getdeps.ps1            # Use this in case PowerShell says "execution of scripts is disabled on this system" to the previous command
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
> .\output\Chip8Win.exe <gamepath>
```

Or just drag and drop a CHIP-8 rom on the executable.

### On Linux

You might want to follow the instructions listed [here](https://github.com/raysan5/raylib/wiki/Working-on-GNU-Linux). If you don't see your distribution listed in the
[Install on GNU Linux](https://github.com/raysan5/raylib/wiki/Working-on-GNU-Linux#install-on-gnu-linux) section, you're either gonna have to manually compile and install Raylib (either
`STATIC` or `SHARED` versions should work just fine) or, alternatively, you might be able to just run the following shell script:

```console
$ chmod +x ./getdeps.sh              # give it permission to execute
$ ./getdeps.sh                       # execute the script
```

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
