FROM ubuntu:bionic AS compiler-build

RUN apt-get update && \
    apt-get install -y gcc \
                       gcc-multilib \
                       make \
                       cmake \
                       git \
                       python3.8

WORKDIR /home
RUN git clone https://github.com/pawn-lang/pawn.git pawn
RUN mkdir build && \
    cd build && \
    cmake ../pawn/source/compiler \
    -DCMAKE_C_FLAGS=-m32 \
    -DCMAKE_BUILD_TYPE=Release && \
    make
WORKDIR /home/build
RUN cd /home/build && \
    chmod +x pawncc libpawnc.so
ENTRYPOINT ["/home/build/pawncc"]
CMD /bin/bash

FROM compiler-build AS compiler
RUN apt-get install bash
RUN mkdir ~/compiler
WORKDIR ~/compiler
COPY --from=compiler-build /home/build/pawncc .
COPY --from=compiler-build /home/build/pawnruns .
COPY --from=compiler-build /home/build/pawndisasm .
COPY --from=compiler-build /home/build/libpawnc.so .
ENTRYPOINT ["/compiler/pawncc"]
CMD /bin/sh
