#!/bin/bash
root_dir=$(dirname $(readlink -f $0))

mkdir -p ${root_dir}/build/amd64
cd ${root_dir}/build/amd64

cmake -D BUILD_TYP='amd64' ${root_dir}
make -j$(nproc)

mkdir -p ${root_dir}/build/raspi
cd ${root_dir}/build/raspi

cmake -D BUILD_TYP='raspi' -D CMAKE_TOOLCHAIN_FILE=${root_dir}/pi.cmake ${root_dir}
make -j$(nproc)

mkdir -p ${root_dir}/build/esp32
cd ${root_dir}/build/esp32

source ${HOME}/esp/esp-idf/export.sh > /dev/null
idf.py -G "Unix Makefiles" -C "${root_dir}/src/esp32" -B "${root_dir}/build/esp32" --ccache build

echo ""

# eval `ssh-agent -s` > /dev/null
# ssh-add

ssh pi@raspi "/bin/bash -c '\
rm -r /home/pi/ecu/; \
mkdir -p /home/pi/ecu/{bin,lib} \
'"

rsync -rvcaz --exclude "*.a" ${root_dir}/lib/raspi/* pi@raspi:/home/pi/ecu/lib
rsync -rvcaz ${root_dir}/bin/raspi/* pi@raspi:/home/pi/ecu/bin
rsync -rvcazC ${root_dir} pi@raspi:/home/pi/

# ssh pi@raspi "sudo bash -c 'echo /home/pi/ecu/lib > /etc/ld.so.conf.d/ecu.conf; ldconfig'"
