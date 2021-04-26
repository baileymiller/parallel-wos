cmake_minimum_required(VERSION 3.10)

project(ParallelWoS)
set(CMAKE_CXX_STANDARD 17)

# External Libraries
find_package(Eigen3 3.3 REQUIRED NO_MODULE)
find_package(OpenMP)

# Source Code
include_directories(
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/ext/pcg32
    ${PROJECT_SOURCE_DIR}/ext/stb_image
)

set(pwos_headers
    include/pwos/bbox.h
    include/pwos/circle.h
    include/pwos/common.h
    include/pwos/fwd.h
    include/pwos/image.h
    include/pwos/scene.h
)

set(pwos_srcs
    src/argparse.cpp
    src/circle.cpp
    src/image.cpp
)

# library with all of the source + header files above
add_library(
    pwos_lib
    STATIC
    ${pwos_headers}
    ${pwos_srcs}
)

# add executables
add_executable(wos src/main.cpp)
target_link_libraries(
    wos
    pwos_lib
    Eigen3::Eigen
    OpenMP::OpenMP_CXX
)
