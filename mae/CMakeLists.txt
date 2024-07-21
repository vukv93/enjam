cmake_minimum_required(VERSION 3.0)
add_library(mae 
  mae.cpp
  mae-contlib.cpp
  mae-modlib.cpp)
target_link_libraries(mae garage m jack sndfile samplerate)
target_include_directories(mae PUBLIC "../local/include")
target_link_directories(mae PUBLIC "../local/lib")
