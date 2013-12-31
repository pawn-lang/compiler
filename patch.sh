#!/bin/sh

# Download the source code and extract it.
wget http://www.compuphase.com/pawn/pawn-3.2.3664.zip
unzip pawn-3.2.3664.zip -d pawn

# Apply patches.
python patch.py --source pawn/SOURCE

# Rename some directories to fix possible compile errors
# on case-sensitive file systems.
mv pawn/SOURCE/AMX   pawn/SOURCE/amx
mv pawn/SOURCE/LINUX pawn/SOURCE/linux

# Build the code.
mkdir build && cd build
cmake ../pawn/SOURCE/COMPILER -G "Unix Makefiles" \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_FLAGS=-m32
make
