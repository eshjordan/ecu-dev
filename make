#!/bin/bash
cd build
cmake ..
make -j$(nproc)
