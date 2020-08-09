# smartnode

[![Build Status](https://travis-ci.com/PhantomGrazzler/smartnode.svg?branch=master)](https://travis-ci.com/PhantomGrazzler/smartnode)
[![Build status](https://ci.appveyor.com/api/projects/status/nk0elpdmaoxfgebv/branch/master?svg=true)](https://ci.appveyor.com/project/PhantomGrazzler/smartnode/branch/master)

This repository contains code for two executables: a SmartNode server and a SmartNode UI, both written in C++.

## Building
The instructions below contain example commands for building Debug executables using a multi-configuration generator such as Visual Studio.

1. Clone this repository: ```git clone https://github.com/PhantomGrazzler/smartnode.git```
2. Create a build directory, _e.g._ ```mkdir build && cd build```
3. Install dependencies: ```conan install .. -s build_type=Debug --build missing```
4. Run the CMake configure step: ```cmake .. -DCMAKE_BUILD_TYPE=Debug```
5. Build the server using CMake: ```cmake --build . --config Debug```

*Note:* There is a known issue in boost::beast that code will not compile in Release mode when using Visual Studio (see https://github.com/boostorg/beast/issues/1582).
