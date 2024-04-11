cmake_minimum_required(VERSION 3.21)
set(VCPKG_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/glt/vcpkg")
set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
set(X_VCPKG_APPLOCAL_DEPS_INSTALL ON)
project(CircuitSimulator VERSION 0.3.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

include(CTest)
enable_testing()

file(GLOB_RECURSE GLT_SHADERS CONFIGURE_DEPENDS "src/shaders/*.frag" "src/shaders/*.vert")
add_subdirectory(glt)

file(GLOB_RECURSE all_files CONFIGURE_DEPENDS "src/*.cpp" "include/*.h")
add_executable(CircuitSimulator main.cpp ${all_files})
target_include_directories(CircuitSimulator PRIVATE include)

file(GLOB_RECURSE asset_files "${PROJECT_SOURCE_DIR}/assets/*.png")
add_custom_target(Assets)
# Create the assets folder
add_custom_command(TARGET Assets PRE_BUILD
COMMAND ${CMAKE_COMMAND} -E make_directory "${PROJECT_BINARY_DIR}/assets")
foreach(asset_file ${asset_files})
get_filename_component(fileName ${asset_file} NAME)
# file(READ ${asset_file} content)
# file(GENERATE OUTPUT "$<TARGET_FILE_DIR:CircuitSimulator>/assets/${fileName}" CONTENT ${content})
add_custom_command(TARGET Assets PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${asset_file} $<TARGET_FILE_DIR:CircuitSimulator>/assets/${fileName})
endforeach()
add_dependencies(CircuitSimulator Assets)

add_dependencies(CircuitSimulator glt)
target_compile_definitions(CircuitSimulator PUBLIC NOMINMAX=1 WIN32_LEAN_AND_MEAN=1)
if (MSVC)
    #add_compile_options(/W4 /WX)
    # add_compile_options(/fsanitize=address)
    target_compile_definitions(CircuitSimulator PRIVATE _DISABLE_STRING_ANNOTATION _DISABLE_VECTOR_ANNOTATION)
else()
    target_compile_options(CircuitSimulator PRIVATE -Wall -Wextra -Wno-unused-parameter -Wno-c++11-narrowing -Werror -ferror-limit=0)
endif()
target_link_libraries(CircuitSimulator PRIVATE glt)
install(TARGETS CircuitSimulator RUNTIME DESTINATION .)
install(DIRECTORY "$<TARGET_FILE_DIR:CircuitSimulator>/assets" DESTINATION .)

find_package(Eigen3 CONFIG REQUIRED)
target_link_libraries(CircuitSimulator PRIVATE Eigen3::Eigen)

add_subdirectory(extern/portable-file-dialogs)
target_link_libraries(CircuitSimulator PRIVATE portable_file_dialogs)

set(CPACK_GENERATOR "ZIP;NSIS")
include(CPack)
