cmake_minimum_required (VERSION 3.5)

project (SeastarExample)

find_package (Seastar REQUIRED)

add_executable(example
  main.cc
  src/getting-started.cc
)

target_link_libraries (example
  PRIVATE Seastar::seastar
)
