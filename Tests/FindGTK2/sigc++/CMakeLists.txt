cmake_minimum_required(VERSION 3.10)

project(sigc++ CXX)

find_package(GTK2 COMPONENTS gtk gtkmm REQUIRED)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_executable(sigc++ WIN32 main.cpp)
target_link_libraries(sigc++ GTK2::sigc++)
