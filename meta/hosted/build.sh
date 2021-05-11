#!/bin/bash
g++ \
    -o test_ttf \
    -g \
    -std=c++20 \
    -Imeta/hosted/includes \
    -Iuserspace/libraries \
    -Iuserspace/apps \
    -Iuserspace/hosted/includes \
    -D__CONFIG_IS_RELEASE__=0 \
    -D__CONFIG_IS_HOSTED__=1 \
    -DDISABLE_LOGGER \
    meta/hosted/test_ttf.cpp \
    userspace/libraries/libgraphic/font/TrueTypeFontFace.cpp \
    userspace/libraries/libgraphic/png/PngReader.cpp \
    userspace/libraries/libcompression/Inflate.cpp \
    userspace/libraries/libcompression/CRC.cpp \
    userspace/libraries/libio/File.cpp \
    userspace/libraries/libio/Format.cpp \
    userspace/libraries/libio/Streams.cpp \
    meta/hosted/plugs/*.cpp \
    -fsanitize=address \
    -fsanitize=undefined

g++ \
    -o test_png \
    -g \
    -std=c++20 \
    -Imeta/hosted/includes \
    -Iuserspace/libraries \
    -Iuserspace/apps \
    -Iuserspace/hosted/includes \
    -D__CONFIG_IS_RELEASE__=0 \
    -D__CONFIG_IS_HOSTED__=1 \
    -DDISABLE_LOGGER \
    meta/hosted/test_png.cpp \
    userspace/libraries/libgraphic/font/TrueTypeFontFace.cpp \
    userspace/libraries/libgraphic/png/PngReader.cpp \
    userspace/libraries/libcompression/Inflate.cpp \
    userspace/libraries/libcompression/CRC.cpp \
    userspace/libraries/libio/File.cpp \
    userspace/libraries/libio/Format.cpp \
    userspace/libraries/libio/Streams.cpp \
    meta/hosted/plugs/*.cpp \
    -fsanitize=address \
    -fsanitize=undefined