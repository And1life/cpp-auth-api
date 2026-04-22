cmake_minimum_required(VERSION 3.16)
project(auth_service)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost REQUIRED COMPONENTS system thread)

file(GLOB SOURCES src/*.cpp)

add_executable(auth_service ${SOURCES})

target_include_directories(auth_service
    PUBLIC ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(auth_service
    Boost::system
    Boost::thread
)