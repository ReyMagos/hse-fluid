cmake_minimum_required(VERSION 3.28)

project(hse_fluid LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)

# Helps CMake find clang-scan-deps which is required
# to compile C++20 modules
# set(CMAKE_CXX_COMPILER_CLANG_SCAN_DEPS /usr/bin/clang-scan-deps)

# We are using CommandLine library which is a part of LLVM/Support component
# Embedding LLVM as CMake target is done as shown here:
# https://llvm.org/docs/CMake.html#embedding-llvm-in-your-project
find_package(LLVM REQUIRED CONFIG)

add_executable(${PROJECT_NAME} main.cpp fixed.cppm fluid.cppm
        llvm_patched/Support/CommandLine.cpp llvm_patched/Support/CommandLine.h)

target_include_directories(${PROJECT_NAME} PRIVATE ${LLVM_INCLUDE_DIRS})
target_sources(${PROJECT_NAME}
        PUBLIC
        FILE_SET modules TYPE CXX_MODULES FILES fluid.cppm fixed.cppm
)

llvm_map_components_to_libnames(llvm_libs support)
target_link_libraries(${PROJECT_NAME} ${llvm_libs})
