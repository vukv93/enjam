cmake_minimum_required(VERSION 3.0)
add_library(garage 
  jack.cpp
  main.cpp
  memory.cpp
  sndfile.cpp)
target_link_libraries(garage m jack sndfile samplerate)
