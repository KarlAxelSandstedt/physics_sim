#/bin/bash
cmake -S . -B build
cd build
cmake --build .
./basic_physics_simulation > tmp.txt & 
$(sleep(1))
ProcID=$(pidof basic_physics_simulation)
perf top -p ${ProcID}
cd ..
