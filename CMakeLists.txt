cmake_minimum_required(VERSION 3.26)
project(EcoSim C)

find_package(SDL3 REQUIRED)

add_executable(EcoSim)

target_sources(EcoSim
PRIVATE
    src/Main.c    
    src/Population.c
    src/Util.c
    src/Display.c
    src/cJson.c
)

target_include_directories(EcoSim
PRIVATE    
    include
)

target_link_libraries(EcoSim SDL3::SDL3)