cmake_minimum_required(VERSION 3.10)
project(Base64)

set(SOURCE_FILES
  ${CMAKE_SOURCE_DIR}/source/main.cpp
  )

add_definitions(-std=c++17)
add_executable(base64 ${SOURCE_FILES})
