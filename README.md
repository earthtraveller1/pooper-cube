# Pooper Cube

Can Pooper, but in the form of a cube. Yes, amazing, I know.

## Quick Start

### Windows

If you haven't already, you need to install the Vulkan SDK, CMake, a build system (preferably Ninja, but MSBuild is fine) and a C++20 compliant compiler (MSVC is fine).

```
cmake . -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

### Linux

If you haven't already, you need to install the Vulkan SDK, X.Org development files (The `xorg-dev` package on Ubuntu, not sure about other distros), CMake, a build system (preferably Ninja, but MSBuild is fine) and a C++20 compliant compiler (MSVC is fine).

```
cmake . -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

