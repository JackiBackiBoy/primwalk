windres -i fzui/src/windows/resources.rc -o resources.o --use-temp-file
cmake -S . -B . -DCMAKE_TOOLCHAIN_FILE="%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake" -A x64