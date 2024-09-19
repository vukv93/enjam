# The default constructor one.
add_executable(test test.cpp)
set_property(TARGET test PROPERTY VERSION ${PROJECT_VERSION})
target_link_libraries(test mae)
