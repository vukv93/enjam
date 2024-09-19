cmake_minimum_required(VERSION 3.0)
add_library(mae SHARED 
  mae.cpp
  backend/mae-jack.cpp
  modlib/modlib.cpp
  contlib/contlib.cpp
)
target_link_libraries(mae garage m jack)
