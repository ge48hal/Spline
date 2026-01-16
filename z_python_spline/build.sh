#!/usr/bin/env bash
set -e

# Remove all .so files in the current directory (z_python_spline)
rm -f *.so

# Move to parent directory
cd ..

# Remove build directory if it exists
if [ -d build ]; then
    rm -rf build
fi

# Create build directory and enter it
mkdir build
cd build

# Configure and build
cmake ..
make -j8

# Copy generated shared libraries back to z_python_spline
cp *.so ../z_python_spline/
