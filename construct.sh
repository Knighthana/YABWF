#!/usr/bin/env bash

# Brief: the construction script for yabwf project
# Author: Knighthana (https://github.com/Knighthana)
# Version: docv1.0.0
# Date: 2024/01/07
# any problem please make an issue on https://github.com/Knighthana/YABWF

# directory
cd $(dirname "$0")
PROJECT_PATH=$(pwd)
# `HOST`
if [ -n "$HOST" ]; then
    HOST=$HOST
else
    HOST=x86_64-linux-gnu
fi
# `RUNPREFIXDIR`
if [ -n "$RUNPREFIXDIR" ]; then
    RUNPREFIXDIR=$RUNPREFIXDIR
else
    RUNPREFIXDIR=
fi
# `BUILDMACHINE`
if [ -n "$BUILDMACHINE" ]; then
    BUILDMACHINE=$BUILDMACHINE
else
    BUILDMACHINE=amd64
fi
# `TRANSFERDIR`
if [ -n "$TRANSFERDIR" ]; then
    TRANSFERDIR=$TRANSFERDIR
else
    TRANSFERDIR=$PROJECT_PATH/out
fi
# `CPUTHREAD`
if [ -n "$CPUTHREAD" ]; then
    CPUTHREAD=$CPUTHREAD
else
    CPUTHREAD=$(nproc)
fi
# clean func
clean()
{
    rm -f $TRANSFERDIR$RUNPREFIXDIR/bin/boa
    rm -f $TRANSFERDIR$RUNPREFIXDIR/lib/boa_indexer
    cd $PROJECT_PATH
    make distclean
}
# verify-cross func
verify_cross()
{
    cd $PROJECT_PATH
    make clean 2>/dev/null
    echo "verify-cross: configuring with --host=$HOST CC=$CC"
    if ! ./configure --build=$BUILDMACHINE --host=$HOST CC=$CC; then
        echo "verify-cross: configure failed for host=$HOST CC=$CC" >&2
        exit 1
    fi
    echo "verify-cross: building with -j$CPUTHREAD"
    if ! make -j$CPUTHREAD -C src; then
        echo "verify-cross: make failed for host=$HOST" >&2
        exit 1
    fi
    echo "verify-cross: SUCCESS for host=$HOST"
    for bin in src/boa src/boa_indexer; do
        if [ -f "$bin" ]; then
            size=$(stat --format="%s" "$bin" 2>/dev/null || stat -f"%z" "$bin" 2>/dev/null)
            echo "  artifact: $bin ($size bytes)"
        fi
    done
}
# build func
build()
{
    cd $PROJECT_PATH
    make clean
    ./configure --prefix=$RUNPREFIXDIR --build=$BUILDMACHINE --host=$HOST
    make -j$CPUTHREAD
    mkdir -p $TRANSFERDIR$RUNPREFIXDIR/bin
    mkdir -p $TRANSFERDIR$RUNPREFIXDIR/lib
    cp -f $PROJECT_PATH/src/boa $TRANSFERDIR$RUNPREFIXDIR/bin/boa
    cp -f $PROJECT_PATH/src/boa_indexer $TRANSFERDIR$RUNPREFIXDIR/lib/boa_indexer
}
# procedure
if [ -z "$1" ]; then
    build;
else
    if [ "$1" = "build" ]; then
        build;
    elif [ "$1" = "clean" ]; then
        clean
    elif [ "$1" = "verify-cross" ]; then
        verify_cross
    else
        echo "\"$1\" option is not supported"
        echo "supporting options:"
        echo " build -- build the project"
        echo " clean -- clean the project and output"
        echo " verify-cross -- cross-compilation verification (set HOST and CC env vars)"
    fi
fi
