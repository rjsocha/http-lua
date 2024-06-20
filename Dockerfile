# syntax=docker/dockerfile:1.6
FROM alpine:3
RUN apk --no-cache add gcc musl-dev make lua5.4-dev
COPY src/http-lua.c src/httpserver.h /build/
WORKDIR /build
RUN gcc -o http-lua -s -O2 -static http-lua.c /usr/lib/lua5.4/liblua.a -rdynamic -lm -ldl -I/usr/include/lua5.4 && strip -x http-lua
WORKDIR /result
RUN cp /build/http-lua /result/
