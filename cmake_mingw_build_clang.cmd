mkdir build
cd build
cmake .. -G "MinGW Makefiles" -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_CXX_FLAGS="-target x86_64-w64-windows-gnu"