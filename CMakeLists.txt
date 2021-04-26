cmake_minimum_required(VERSION 3.10)

project(ParallelWoS)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE DEBUG)

# External Libraries
find_package(Eigen3 3.3 REQUIRED NO_MODULE)

find_package(OpenMP)
if (OPENMP_FOUND)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()

# Source Code
include_directories(
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/include/integrators
    ${PROJECT_SOURCE_DIR}/ext/pcg32
    ${PROJECT_SOURCE_DIR}/ext/stb_image
)

set(pwos_headers
    include/pwos/argparse.h
    include/pwos/bbox.h
    include/pwos/circle.h
    include/pwos/closestPointGrid.h
    include/pwos/common.h
    include/pwos/fwd.h
    include/pwos/image.h
    include/pwos/integrator.h
    include/pwos/integrators/wos.h
    include/pwos/integrators/distance.h
    include/pwos/progressBar.h
    include/pwos/scene.h
    include/pwos/stats.h
)

set(pwos_srcs
    src/argparse.cpp
    src/bbox.cpp
    src/circle.cpp
    src/closestPointGrid.cpp
    src/image.cpp
    src/integrator.cpp
    src/scene.cpp
    src/stats.cpp
)

# library with all of the source + header files above
add_library(
    pwos_lib
    STATIC
    ${pwos_headers}
    ${pwos_srcs}
)

# add executables
add_executable(pwos src/main.cpp)
target_link_libraries(
    pwos
    pwos_lib
    Eigen3::Eigen
    OpenMP::OpenMP_CXX
)
