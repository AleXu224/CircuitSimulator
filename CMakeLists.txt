cmake_minimum_required(VERSION 3.21)
set(VCPKG_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/glt/vcpkg")
set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
set(X_VCPKG_APPLOCAL_DEPS_INSTALL ON)
project(CircuitSimulator VERSION 0.1.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 23)
if (MSVC)
    #add_compile_options(/W4 /WX)
    # add_compile_options(/fsanitize=address)
    add_compile_definitions(_DISABLE_STRING_ANNOTATION _DISABLE_VECTOR_ANNOTATION)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2")
else()
    add_compile_options(-Wall -Wextra -Wno-unused-parameter -Wno-c++11-narrowing -Werror)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -ferror-limit=0")
endif()

file(GLOB_RECURSE asset_files "${PROJECT_SOURCE_DIR}/assets/*.png")
add_custom_target(Assets)
# Create the assets folder
add_custom_command(TARGET Assets PRE_BUILD
COMMAND ${CMAKE_COMMAND} -E make_directory "${PROJECT_BINARY_DIR}/assets")
foreach(asset_file ${asset_files})
    get_filename_component(fileName ${asset_file} NAME)
    configure_file("${asset_file}" "${PROJECT_BINARY_DIR}/assets/${fileName}" COPYONLY)
endforeach()

include(CTest)
enable_testing()

file(GLOB_RECURSE GLT_SHADERS CONFIGURE_DEPENDS "src/shaders/*.frag" "src/shaders/*.vert")
add_subdirectory(glt)

file(GLOB_RECURSE all_files CONFIGURE_DEPENDS "src/*.cpp" "include/*.h")
add_executable(CircuitSimulator main.cpp ${all_files})
add_dependencies(CircuitSimulator Assets)
target_include_directories(CircuitSimulator PRIVATE include)

target_link_libraries(CircuitSimulator PRIVATE glt)
install(TARGETS CircuitSimulator DESTINATION .)
install(DIRECTORY "${PROJECT_BINARY_DIR}/assets" DESTINATION .)

set(CPACK_GENERATOR "ZIP;NSIS")
include(CPack)
