cmake_minimum_required(VERSION 3.27)
project(CircuitSimulator VERSION 0.1.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
if (MSVC)
    add_compile_options(/W4 /WX)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2")
else()
    add_compile_options(-Wall -Wextra -Wno-unused-parameter -Werror)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -ferror-limit=0")
endif()

include(CTest)
enable_testing()

file(GLOB_RECURSE all_files CONFIGURE_DEPENDS "src/*.cpp" "include/*.h")
add_executable(CircuitSimulator main.cpp ${all_files})
target_include_directories(CircuitSimulator PRIVATE include)

add_subdirectory(glt)

target_link_libraries(CircuitSimulator PRIVATE glt)
