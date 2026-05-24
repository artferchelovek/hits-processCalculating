cmake_minimum_required(VERSION 3.10)
project(parallel_calculate)

set(CMAKE_CXX_STANDARD 17)

# 1. OpenMP
if(APPLE)
    find_library(LIBOMP_LIBRARY NAMES omp PATHS /opt/homebrew/opt/libomp/lib /usr/local/opt/libomp/lib)
    find_path(LIBOMP_INCLUDE_DIR NAMES omp.h PATHS /opt/homebrew/opt/libomp/include /usr/local/opt/libomp/include)
    if(LIBOMP_LIBRARY AND LIBOMP_INCLUDE_DIR)
        set(OpenMP_CXX_FOUND TRUE)
        set(OMP_FLAGS "-Xpreprocessor" "-fopenmp")
        set(OpenMP_CXX_LIBRARIES "${LIBOMP_LIBRARY}")
        set(OpenMP_CXX_INCLUDE_DIRS "${LIBOMP_INCLUDE_DIR}")
    endif()
else()
    find_package(OpenMP REQUIRED)
    set(OMP_FLAGS ${OpenMP_CXX_FLAGS})
endif()

# 2. OpenCL
if(APPLE)
    find_library(OPENCL_LIBRARIES OpenCL)
    set(OpenCL_FOUND TRUE)
    set(OpenCL_INCLUDE_DIRS "/System/Library/Frameworks/OpenCL.framework/Headers")
else()
    find_package(OpenCL REQUIRED)
endif()

set(SOURCE_FILES
    main.cpp 
    filters/sequential.cpp 
    filters/openmp.cpp
    filters/simd.cpp
    filters/opencl.cpp
)

add_executable(parallel_calculate ${SOURCE_FILES})

if(OpenMP_CXX_FOUND)
    target_compile_options(parallel_calculate PRIVATE ${OMP_FLAGS})
    target_link_libraries(parallel_calculate PRIVATE ${OpenMP_CXX_LIBRARIES})
    target_include_directories(parallel_calculate PRIVATE ${OpenMP_CXX_INCLUDE_DIRS})
endif()

if(OpenCL_FOUND)
    target_link_libraries(parallel_calculate PRIVATE ${OPENCL_LIBRARIES})
    target_include_directories(parallel_calculate PRIVATE ${OpenCL_INCLUDE_DIRS})
endif()
