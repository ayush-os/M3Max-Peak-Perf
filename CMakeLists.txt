# M3Max-Peak-Perf/CMakeLists.txt

cmake_minimum_required(VERSION 3.20)
project(M3Max-Peak-Perf LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -march=native -Wall -Wextra -Wpedantic")

add_library(M3Max_Utils INTERFACE)
target_include_directories(M3Max_Utils INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# --- CPU Kernel Target (Phase A) ---

set(CPU_SOURCES
    src/cpu/matmul_baseline.cpp
    src/cpu/neon_microkernel.cpp
)

add_library(CPU_Kernels STATIC ${CPU_SOURCES})
# Make the NEON headers available only to the implementation files
target_compile_features(CPU_Kernels PRIVATE cxx_std_17)
target_link_libraries(CPU_Kernels PUBLIC M3Max_Utils)

# --- GPU/Metal Kernel Target (Phase B) ---

set(GPU_HOST_SOURCES
    src/gpu/metal_host.mm
)
set(METAL_SHADERS
    src/gpu/attention_kernel.metal
)

add_library(GPU_Kernels STATIC ${GPU_HOST_SOURCES})

target_link_libraries(GPU_Kernels PUBLIC "-framework Metal" "-framework Foundation" M3Max_Utils)
target_compile_options(GPU_Kernels PRIVATE
    -fmodules
    -fcxx-modules
)

# --- Main Executable (Phase C) ---

add_executable(${PROJECT_NAME}
    src/main.cpp
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
    CPU_Kernels
    GPU_Kernels
    M3Max_Utils
)

target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bin"
)