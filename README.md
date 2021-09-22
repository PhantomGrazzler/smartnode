# smartnode

Windows: [![Build status](https://ci.appveyor.com/api/projects/status/7bfxo9u4r13vqx79/branch/master?svg=true)](https://ci.appveyor.com/project/PhantomGrazzler/smartnode/branch/master)

This repository contains code for two executables: a SmartNode server and a SmartNode UI, both written in C++.

## Building
The instructions below contain example commands for building Debug executables using a multi-configuration generator such as Visual Studio.

1. Clone this repository: ```git clone https://github.com/PhantomGrazzler/smartnode.git```
2. Create a build directory, _e.g._ ```mkdir build && cd build```
3. Add bintray to your conan remotes if you do not already have it: `conan remote add bintray https://api.bintray.com/conan/bincrafters/public-conan`
4. Install dependencies: ```conan install .. -s build_type=Debug --build missing```
5. Run the CMake configure step: ```cmake .. -DCMAKE_BUILD_TYPE=Debug```
6. Build using CMake: ```cmake --build . --config Debug```
