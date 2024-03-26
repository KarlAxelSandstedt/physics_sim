#/bin/bash
cmake -S . -B build
cd build
make
cd ..
valgrind --leak-check=full ./build/tests/wfc3D_test
