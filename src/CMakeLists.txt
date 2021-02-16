cmake_minimum_required(VERSION 3.8)

project(TsnDesigner)

option(DEBUG "Enable debug" OFF)

set(CXX_FLAGS_COMMON "-Wall -std=c+2a")
set(CXX_FLAGS_DEBUG "${CXX_FLAGS_COMMON} -g")
set(CXX_FLAGS_RELEASE "${CXX_FLAGS_COMMON} -O3 -s")

if (DEBUG) 
    set(CMAKE_CXX_FLAGS "${CXX_FLAGS_DEBUG}")
else()
    set(CMAKE_CXX_FLAGS "${CXX_FLAGS_RELEASE}")
endif()

#add_library(SolutionScheduling Solution/SolutionScheduling.h Solution/SolutionScheduling.cpp)
#add_library(MutationScheduling Mutation/MutationScheduling.h Mutation/MutationScheduling.cpp)
#add_library(XmlLibrary XmlLibrary/tinyxml2.cpp XmlLibrary/tinyxml2.h)

#include_directories(Solution)
#include_directories(Mutation)
#include_directories(Temperature)
#include_directories(XmlLibrary)
#include_directories(SaSolver)

add_executable(${PROJECT_NAME} main.cpp)
#target_link_libraries(${PROJECT_NAME} SolutionScheduling)
#target_link_libraries(${PROJECT_NAME} MutationScheduling)
#target_link_libraries(${PROJECT_NAME} XmlLibrary
