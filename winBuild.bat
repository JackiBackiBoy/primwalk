windres -i src\fhcoach\resources.rc -o resources.o --use-temp-file

if not exist build mkdir build
cd build
cmake -S ../ -B . -G "MinGW Makefiles"
make
fhcoach.exe
cd ..
