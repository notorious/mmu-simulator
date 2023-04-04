cmake_minimum_required(VERSION 3.22)
project(mmuwithtlb)

set(CMAKE_CXX_STANDARD 11)

add_executable(mmuwithtlb main.cpp PageTable.cpp print_helpers.cpp vaddr_tracereader.cpp Map.cpp Level.cpp TLB.cpp TLB.h)
