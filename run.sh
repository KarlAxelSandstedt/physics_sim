#/bin/bash
cmake -S . -B build
cd build
cmake --build .
./basic_physics_simulation
cd ..
