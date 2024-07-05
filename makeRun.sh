#!/bin/bash

clear

# Run make clean
echo "Running make clean..."
make clean

# Check if make clean was successful
if [ $? -ne 0 ]; then
    echo "make clean failed!"
    exit 1
fi

# Run make with DEBUG=1
echo "Running make DEBUG=1..."
make DEBUG=1

# Check if make was successful
if [ $? -ne 0 ]; then
    echo "make DEBUG=1 failed!"
    exit 1
fi

echo "Build completed successfully."
