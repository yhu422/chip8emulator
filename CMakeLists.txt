cmake_minimum_required(VERSION 3.29)
project(chip8emulator)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

include(FetchContent)
FetchContent_Declare(SFML
        GIT_REPOSITORY https://github.com/SFML/SFML.git
        GIT_TAG 2.6.x
        GIT_SHALLOW ON
        EXCLUDE_FROM_ALL
        SYSTEM)
FetchContent_MakeAvailable(SFML)
add_library(chip8 STATIC chip8.cpp)
target_link_libraries(chip8 PRIVATE sfml-graphics)
add_executable(chip8emulator main.cpp)
target_link_libraries(chip8emulator PRIVATE sfml-graphics)
target_link_libraries(chip8emulator PRIVATE chip8 sfml-graphics)
target_compile_features(chip8emulator PRIVATE cxx_std_17)

if(WIN32)
    add_custom_command(
            TARGET main
            COMMENT "Copy OpenAL DLL"
            PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${SFML_SOURCE_DIR}/extlibs/bin/$<IF:$<EQUAL:${CMAKE_SIZEOF_VOID_P},8>,x64,x86>/openal32.dll $<TARGET_FILE_DIR:main>
            VERBATIM)
endif()
