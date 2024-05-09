# It is based on [zynaddsubfx](https://github.com/zynaddsubfx/zynaddsubfx) version 2.4.1 and modified to build x64 for Windows

## Know issues

Plugin not work

# Setup Build System

Install msys2 [https://www.msys2.org/]

Run MSYS shell:

```
pacman -S mingw-w64-ucrt-x86_64-toolchain
pacman -S mingw-w64-ucrt-x86_64-mxml
pacman -S mingw-w64-ucrt-x86_64-cmake
pacman -S mingw-w64-ucrt-x86_64-cmake-gui
pacman -S mingw-w64-ucrt-x86_64-extra-cmake-modules
pacman -S mingw-w64-ucrt-x86_64-fltk
```

Run UCRT64 shell:

```
cd /c/
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.bat

export VCPKG_ROOT="C:\vcpkg"
export PATH=$VCPKG_ROOT:$PATH
export VCPKG_DEFAULT_TRIPLET=x64-mingw-static
export VCPKG_DEFAULT_HOST_TRIPLET=x64-mingw-static

./vcpkg install vcpkg-pkgconfig-get-modules:x64-mingw-static
./vcpkg install pkgconf:x64-mingw-static
./vcpkg install fftw3:x64-mingw-static
./vcpkg install zlib:x64-mingw-static
./vcpkg install fltk:x64-mingw-static
./vcpkg install portaudio:x64-mingw-static
```

## Build

```
mkdir /c/dev
cd /c/dev
git clone --recursive https://github.com/rty65tt/zynaddsubfx
cd zynaddsubfx
mkdir build
cd build
cmake-gui ..

Configure -> MinGW Makefiles -> Generate

cmake --build . --config=Release
cd src
```
