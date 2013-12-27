Build Instructions
------------------

### Applying patches manually

You may have to install some packages. I had to install these on my Ubuntu:

    sudo apt-get install wget unzip git patch cmake make

When you're done installng dependencies run the following commands:

    wget http://www.compuphase.com/pawn/pawn-3.2.3664.zip
    unzip pawn-3.2.3664.zip
    cd SOURCE
    git clone -b patches git://github.com/Zeex/pawn.git patches
    cd patches
    ./apply_compatible.sh ../ # replace with apply_all.sh for all patches
    mv ../AMX ../amx
    mv ../LINUX ../linux
    cmake ../COMPILER
    make

This will download Pawn source code, apply all (compatible) patches and build
the compiler. If everything goes well you'll find `pawncc` and `libpawnc.so`
in `SOURCE/COMPILER/patches`.

### Building from pre-patched source

The patched source code is located at the `master` branch of this repository.
You can use the following commands for building it:

    git clone git://github.com/Zeex/pawn.git pawn
    cd pawn
    mkdir source/compiler/build
    cd source/compiler/build
    cmake ../
    make