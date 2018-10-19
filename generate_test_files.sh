#!/bin/bash

EXE_DIR=$(dirname "$0")
HYDE_DST_ROOT="test_files_out"

pushd ${EXE_DIR} > /dev/null

for CUR_FILE in ${EXE_DIR}/test_files/*; do
    echo "Processing $CUR_FILE"
    ${EXE_DIR}/build/Debug/hyde -hyde-src-root=${EXE_DIR} -hyde-yaml-dir=${HYDE_DST_ROOT} -hyde-update "$CUR_FILE" --
done

popd > /dev/null
