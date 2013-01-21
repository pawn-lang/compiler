Download
--------

Pre-compiled binaries for Linux and Windows can be found on the
[**`downloads`**](https://github.com/Zeex/pawn-3.2.3664_patches/tree/downloads) branch.


Build Instructions
------------------

First of all, you have to install some packages. If you're using Ubuntu this is as simple as:

    sudo apt-get install wget unzip git patch cmake make

Then execute these commands, in order:

    wget http://www.compuphase.com/pawn/pawn-3.2.3664.zip
    unzip pawn-3.2.3664.zip
    cd SOURCE
    mv AMX amx
    mv LINUX linux
    cd COMPILER
    git clone git://github.com/Zeex/pawn-3.2.3664_patches.git patches
    cd patches
    ./apply_compatible.sh ../../
    cmake ../
    make
    cp libpawnc.so /usr/lib/
    cp pawncc /usr/bin/

If everything goes well you'll find `pawncc` and `libpawnc.so` in `SOURCE/COMPILER/patches` folder.
