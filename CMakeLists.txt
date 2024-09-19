# @todo[240910_015227] Find out the real required CMake version.
# Base project setup.
cmake_minimum_required(VERSION 3.0)
project(enjam VERSION 0.0.1)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE Debug)
#set(CMAKE_CXX_FLAGS "-Wall -Wextra -glldb")
#set(CMAKE_C_FLAGS "-Wall -Wextra -glldb")
set(CMAKE_CXX_FLAGS "-Wall -Wextra -gdwarf-4")
set(CMAKE_C_FLAGS "-Wall -Wextra -gdwarf-4")
include_directories(".")
# Base components.
add_subdirectory(libgarage)
add_subdirectory(mae)
# Test programs.
add_subdirectory(test)
