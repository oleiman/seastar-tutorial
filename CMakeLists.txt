cmake_minimum_required (VERSION 3.5)

project (SeastarExample)

# set_property(TARGET SeastarExample PROPERTY CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD 20)

find_package (Seastar REQUIRED)

add_executable(example
  main.cc
  src/promises.cc
)

target_link_libraries (example
  PRIVATE Seastar::seastar
)
