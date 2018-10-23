#!/bin/bash

EXE_DIR=$(dirname "$0")

pushd ${EXE_DIR} > /dev/null

CUR_DUR=$(pwd)

HYDE_PATH=${CUR_DUR}/build/Debug/hyde # This assumes a binary built with Xcode.
HYDE_SRC_ROOT=${CUR_DUR}
HYDE_DST_ROOT=${CUR_DUR}/test_files_out

for CUR_FILE in ${CUR_DUR}/test_files/*; do
    echo "Processing $CUR_FILE"
    CUR_COMMAND="${HYDE_PATH} -hyde-src-root=${HYDE_SRC_ROOT} -hyde-yaml-dir=${HYDE_DST_ROOT} -hyde-update "$CUR_FILE" --"
    echo $CUR_COMMAND
    eval $CUR_COMMAND
done

popd > /dev/null
