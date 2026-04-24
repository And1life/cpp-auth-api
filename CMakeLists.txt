cmake_minimum_required(VERSION 3.16)
project(auth_service)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# Boost
find_package(Boost REQUIRED COMPONENTS system thread)

# PostgreSQL
find_package(PostgreSQL REQUIRED)

#argon2
find_library(ARGON2_LIB argon2 REQUIRED)

# sources
file(GLOB SOURCES src/*.cpp)

add_executable(auth_service ${SOURCES})

# includes
target_include_directories(auth_service
    PUBLIC ${PROJECT_SOURCE_DIR}/include
)

# linking
target_link_libraries(auth_service
    Boost::system
    Boost::thread
    PostgreSQL::PostgreSQL
    ${ARGON2_LIB}
)