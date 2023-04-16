if not exist build mkdir build
cd build
cmake -S ../ -B . -DCMAKE_TOOLCHAIN_FILE="%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake" -A x64
cd ../