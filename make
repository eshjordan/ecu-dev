#!/bin/bash
root_dir=$(dirname $(readlink -f $0))

mkdir -p ${root_dir}/build/raspi
cd ${root_dir}/build/raspi

cmake -D CMAKE_TOOLCHAIN_FILE=${root_dir}/pi.cmake ${root_dir}
make -j$(nproc)

mkdir -p ${root_dir}/build/amd64
cd ${root_dir}/build/amd64

cmake ${root_dir}
make -j$(nproc)

# eval `ssh-agent -s` > /dev/null
# ssh-add

#ssh pi@raspi "/bin/bash -c '\
#rm -r /home/pi/ecu/; \
#mkdir -p /home/pi/ecu/{bin,lib} \
#'"

#rsync -rvaz --exclude "*.a" ${root_dir}/lib/raspi/* pi@raspi:/home/pi/ecu/lib
#rsync -rvaz ${root_dir}/bin/raspi/* pi@raspi:/home/pi/ecu/bin

# ssh pi@raspi "sudo bash -c 'echo /home/pi/ecu/lib > /etc/ld.so.conf.d/ecu.conf; ldconfig'"
