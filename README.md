# Yet Another Chip 8 Emulator, written in C++

The Emulator uses SFML, a cross-platform library that allows access to different components of the device.


### To Build
In the source directory run:
```
cmake -B build
cmake -b build
```
Note that CMakeLists.txt uses `FetchContent` to download SFML, which requires internet connection.
If this option is not available, you can also use `find_package` to use SFML install on your system

### To Run
```
./chip8Emulator [path-to-.ch8 file]
```

Enjoy!