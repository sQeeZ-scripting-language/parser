rmdir /s /q build
mkdir build
cd build
cmake -G "MinGW Makefiles" -DCMAKE_C_COMPILER=C:/ProgramData/mingw64/mingw64/bin/gcc.exe -DCMAKE_CXX_COMPILER=C:/ProgramData/mingw64/mingw64/bin/g++.exe ..
cmake --build .