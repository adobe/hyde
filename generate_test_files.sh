#!/bin/bash

#very simple for now, look in common locations
find_hyde() {
    BUILD_DIR=$1
    if [ -f $BUILD_DIR/hyde ]
    then
        echo $BUILD_DIR/hyde
        return 0
    fi
    if [ -f $BUILD_DIR/Debug/hyde ]
    then
        echo $BUILD_DIR/Debug/hyde
        return 0
    fi
    if [ -f $BUILD_DIR/Release/hyde ]
    then
        echo $BUILD_DIR/Release/hyde
        return 0
    fi
    return 1
}   

EXE_DIR=$(dirname "$0")

pushd ${EXE_DIR} > /dev/null

CUR_DIR=$(pwd)
HYDE_PATH=`find_hyde "${CUR_DIR}/build"`

for CUR_FILE in ${CUR_DIR}/test_files/*; do
    # echo "Processing $CUR_FILE"
    CUR_COMMAND="${HYDE_PATH} -hyde-update -auto-toolchain-includes -use-system-clang "$CUR_FILE" --"
    echo $CUR_COMMAND
    eval $CUR_COMMAND
done

popd > /dev/null
