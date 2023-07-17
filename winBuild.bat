windres -i primwalk/src/windows/resources.rc -o resources.o --use-temp-file

if not exist build mkdir build
cd build
cmake -S ../ -B . -DCMAKE_TOOLCHAIN_FILE="%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake" -G "MinGW Makefiles"
mingw32-make
mingw32-make Shaders
mingw32-make install
cd bin
fzcoach.exe
cd ../..
