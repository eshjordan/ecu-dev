#!/bin/bash
root_dir=$(dirname $(readlink -f $0))

mkdir -p ${root_dir}/build
cd ${root_dir}/build

rm -rf ${root_dir}/build/*
cmake -D CMAKE_TOOLCHAIN_FILE=${root_dir}/pi.cmake ${root_dir}
make -j$(nproc)

rm -rf ${root_dir}/build/*
cmake ${root_dir}
make -j$(nproc)
