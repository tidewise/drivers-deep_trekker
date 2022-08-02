cmake_minimum_required(VERSION 3.1)
project(deep_trekker
        VERSION 0.1
        DESCRIPTION "Deep trekker drivers algorithms")
find_package(Rock)

# Needed by libdatachannel
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

rock_init()
rock_standard_layout()
