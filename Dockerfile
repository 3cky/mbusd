FROM alpine:latest AS build
RUN apk add --no-cache alpine-sdk gcc cmake
RUN git clone https://github.com/3cky/mbusd
WORKDIR /mbusd/build
RUN cmake -DCMAKE_INSTALL_PREFIX=/usr .. && make && make install

FROM alpine:latest AS scratch
ENV QEMU_EXECVE=1
COPY --from=build /usr/bin/mbusd /usr/bin/mbusd
ENTRYPOINT ["/usr/bin/mbusd", "-d", "-L", "-", "-c", "/etc/mbusd.conf"]
