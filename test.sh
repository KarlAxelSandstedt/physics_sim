#/bin/bash
cmake -S . -B build
cd build
cmake --build .
ctest -V CTestTestfile.cmake
cd ..
