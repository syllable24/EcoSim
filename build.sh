#!/bin/bash
clear
cmake -S . -B build
cmake --build build
BUILD_SUCCESS=$?;
if [ $BUILD_SUCCESS -eq 0 ]; then
    mv ./build/EcoSim.exe ./EcoSim.exe    
    ./EcoSim.exe
else 
    echo "Build failed"
fi