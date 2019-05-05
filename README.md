# SmartNodeServer

[![Build Status](https://travis-ci.com/PhantomGrazzler/SmartNodeServer.svg?branch=master)](https://travis-ci.com/PhantomGrazzler/SmartNodeServer)

C++ websocket server for SmartNodes written using boost::beast.

## Building SmartNodeServer
SmartNodeServer uses Conan and CMake. The instructions below contain example commands for building a Debug executable using a multi-configuration generator such as Visual Studio.

1. Clone this repository: ```git clone https://github.com/PhantomGrazzler/SmartNodeServer.git```
2. Create a build directory, _e.g._ ```mkdir build && cd build```
3. Install dependencies: ```conan install .. -s build_type=Debug```
4. Run the CMake configure step: ```cmake ..```
5. Build the server using CMake: ```cmake --build . --config Debug```

*Note:* There is a known issue in boost::beast that code will not compile in Release mode when using Visual Studio (see https://github.com/boostorg/beast/issues/1582).

## Project layout
[The Pitchfork Layout](https://api.csswg.org/bikeshed/?force=1&url=https://raw.githubusercontent.com/vector-of-bool/pitchfork/develop/data/spec.bs) is used for this project.
