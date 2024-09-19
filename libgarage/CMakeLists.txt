cmake_minimum_required(VERSION 3.0)
add_library(garage SHARED
  jack/jack.cpp
  memory/memory.cpp
)
target_link_libraries(garage jack)
