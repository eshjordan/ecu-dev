#!/bin/bash
root_dir=$(dirname $(readlink -f $0))

mkdir -p ${root_dir}/build/raspi
cd ${root_dir}/build/raspi

# rm -rf ${root_dir}/build/raspi/*
# rm -rf ${root_dir}/lib/raspi/*
# rm -rf ${root_dir}/bin/raspi/*
cmake -D CMAKE_TOOLCHAIN_FILE=${root_dir}/pi.cmake ${root_dir}
make -j$(nproc)

mkdir -p ${root_dir}/build/amd64
cd ${root_dir}/build/amd64

# rm -rf ${root_dir}/build/amd64/*
# rm -rf ${root_dir}/lib/amd64/*
# rm -rf ${root_dir}/bin/amd64/*
cmake ${root_dir}
make -j$(nproc)

scp -i ~/.ssh/id_rsa ${root_dir}/lib/raspi/* pi@192.168.10.112:/home/pi/ecu/lib
scp -i ~/.ssh/id_rsa ${root_dir}/bin/raspi/* pi@192.168.10.112:/home/pi/ecu/bin
