# Visual Studio

* install Visual Studio 2022 with C++ stuff
* clone recursively
* run configure.bat
* open ../Lpg2_build/Lpg2.sln

## Formatting

* install https://github.com/llvm/llvm-project/releases/download/llvmorg-14.0.6/LLVM-14.0.6-win64.exe
* run CMake
* use the clang-format target to format the code in the solution

# Linux

* install GCC
* install ninja
* install perl
* install Firefox
* clone recursively
* run ./configure.sh
  * fix the things that don't work because C++ development is a house of cards (which is one reason why we try to develop a better language)
* the CMake build directory will be at ../build-Lpg2
* run ninja in the build directory or import it into QtCreator

## Formatting

* install clang-format-14
* run CMake
* ninja should format the C++ source files automatically when you build the project

## Code coverage

* open the build directory ../build-Lpg2
* ninja testcoverage
* if it works (which is unlikely), ninja will open Firefox with an HTML coverage report
