#/bin/bash
#cmake -S . -B build
cmake -DCMAKE_BUILD_TYPE=Debug -S . -B build
cd build
cmake --build .
gdb ./basic_physics_simulation
cd ..
