FROM ubuntu:eoan AS compiler-build

RUN apt-get update && \
    dpkg --add-architecture i386 && \
    apt-get install -y gcc \
                       gcc-multilib \
                       make \
                       cmake \
                       git \
                       python3.8

WORKDIR /home
ADD . /home/pawn
COPY ./build.sh /home/build.sh
RUN mkdir build
RUN /home/build.sh
WORKDIR /home/build

CMD ["/bin/bash"]

FROM ubuntu:eoan AS compiler
RUN mkdir /home/compiler
RUN mkdir /home/build
WORKDIR /home/build
COPY --from=compiler-build /home/build/pawncc .
COPY --from=compiler-build /home/build/pawnruns .
COPY --from=compiler-build /home/build/pawndisasm .
COPY --from=compiler-build /home/build/libpawnc.so .
WORKDIR /home
ADD ./copy.sh /home

CMD /bin/bash