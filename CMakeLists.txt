cmake_minimum_required(VERSION 3.15)
project(apc02)

find_package(Threads REQUIRED)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W -Wall -Wextra -Werror -pedantic -fstack-protector-all")

add_executable(apc02 main.cpp Arguments.cpp Arguments.h)
target_link_libraries(${PROJECT_NAME} Threads::Threads)