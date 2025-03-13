# CHIP-8 implementation (in C)

I want to one day be able to create a NES emulator, even if not a good
one. Since that is pretty much not possible right now, I decided to implement something simpler. I followed [these](https://multigesture.net/articles/how-to-write-an-emulator-chip-8-interpreter/) [two](https://tobiasvl.github.io/blog/write-a-chip-8-emulator/) tutorials while building this implementation of the CHIP-8 plus [this](http://devernay.free.fr/hacks/chip8/C8TECH10.HTM) nice technical reference.

## Building and running

### On Windows

Run the following batch script to download the dependencies:

```console
$ ./getdeps.bat
```

**Obs.:** If you have already installed Raylib 5.5
before through the installer available on the Raylib website, you can skip this first step, as the Makefile also tries searching for the necessary files in the default Raylib installation directory.

After that, you should be able to compile the project with just:

```console
$ make
```

Or:

```console
$ mingw32-make
```

Whichever you've got to use (although this assumes that `mingw32-make` will be fine with a project without a `WinMain` entry point). The executable will be saved in a folder called `output`.<br>

To run this game, you may use the command line, like this

```console
$ ./output/Chip8Win.exe <gamepath>
```

Or just drag and drop a CHIP-8 rom on the executable.


### On Linux

Linux users are gonna have to wait until the script(s) get made to be able to just compile and run this project.
