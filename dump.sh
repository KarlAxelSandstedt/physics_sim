#/bin/bash
cmake -S . -B build
cd build
cmake --build .
objdump --source --disassembler-options="intel" --disassemble=$1 basic_physics_simulation
cd ..
