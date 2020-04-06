#!/usr/bin/env bash

cd build && \
cmake ../pawn/source/compiler \
-DCMAKE_C_FLAGS=-m32 \
-DCMAKE_BUILD_TYPE=Release && \
make