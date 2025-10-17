cmake_minimum_required(VERSION 3.26)
project(EcoSim C)
set(CMAKE_BUILD_TYPE Debug)

find_package(SDL3 REQUIRED)
find_package(SDL3_ttf REQUIRED)

add_executable(EcoSim)

target_sources(EcoSim
PRIVATE
    src/Main.c
    src/Population.c
    src/Util.c
    src/Display.c
    src/cJson.c
    src/Hashmap.c
    src/MapGen.c
    src/Globals.c
    src/HexGridMath.c
	src/GameLogic.c
)

target_include_directories(EcoSim
PRIVATE    
    include
    ${SDL3TTF_INCLUDE_DIRS}
)

target_link_libraries(EcoSim 
    SDL3::SDL3
    SDL3_ttf::SDL3_ttf
)

# Ensure pkg-config flags are applied
target_compile_options(EcoSim PRIVATE ${SDL3TTF_CFLAGS_OTHER})