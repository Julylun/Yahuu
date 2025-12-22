# Chat xì trum application

## Installation
1. Install dependencies
- sqlite
- cmake
- make
- gcc
- dependencies required by Raylib
2. Install library
2.1. Raylib
- Clone Raylib from github to your machine
```
git clone https://github.com/raysan5/raylib.git
```
- Build Raylib
```
cd raylib
mkdir build
cd build
cmake ..
make PLATFORM=PLATFORM_DESKTOP
sudo make install
```

## Fastly Build
```
chmod 777 ./build.sh
./build.sh
```

## Build
```
mkdir build
cd build
cmake ..
make
```
## Run
```
./server
./client
```
