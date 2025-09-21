FROM alpine:latest
RUN apk --no-cache add --virtual .build-dependencies \
  alpine-sdk cmake gettext gettext-dev

WORKDIR /root
COPY . ./
RUN git clean -fdx
RUN cmake -S . -B build \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_INSTALL_PREFIX=/ \
      -DCMAKE_EXE_LINKER_FLAGS=-static
RUN cmake --build build --parallel
RUN DESTDIR=/root/stage cmake --install build --strip

FROM scratch
COPY --from=0 /root/stage/bin/advent /advent

ENV WORKSPACE="/root"
ENV HOME="${WORKSPACE}"
WORKDIR "${WORKSPACE}"
ENTRYPOINT ["/advent"]
