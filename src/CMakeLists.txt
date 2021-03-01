cmake_minimum_required(VERSION 3.8)

project(TsnDesigner)

option(DEBUG "Enable debug" ON)

set(CXX_FLAGS_COMMON "-Wall -std=c++2a")
set(CXX_FLAGS_DEBUG "${CXX_FLAGS_COMMON} -g")
set(CXX_FLAGS_RELEASE "${CXX_FLAGS_COMMON} -O3 -s")

if (DEBUG) 
    set(CMAKE_CXX_FLAGS "${CXX_FLAGS_DEBUG}")
else()
    set(CMAKE_CXX_FLAGS "${CXX_FLAGS_RELEASE}")
endif()

#add_library(Routing RoutAlgo/RoutingDijkstra.h RoutAlgo/RoutingDijkstra.cpp)
#add_library(Network Network/Network.h Network/Network.cpp)
#add_library(Scheduler Scheduler/Scheduler.h Scheduler/Scheduler.cpp)
#add_library(XmlLibrary XmlLibrary/tinyxml2.cpp XmlLibrary/tinyxml2.h)

include_directories(Msg)
include_directories(GCL)
include_directories(Network)
include_directories(RoutAlgo)
include_directories(Scheduler)
include_directories(XmlLibrary)

set(CPP_SOURCES
    Network/Network.cpp
    RoutAlgo/RoutingDijkstra.cpp
    Scheduler/Scheduler.cpp
    XmlLibrary/tinyxml2.cpp
    GCL/GCL.cpp
    main.cpp)

set(CPP_HEADERS
    Network/Network.h
    Scheduler/Scheduler.h
    XmlLibrary/tinyxml2.h
    Msg/Message.h
    GCL/GCL.h
    RoutAlgo/RoutingDijkstra.h
    RoutAlgo/RoutBase.h)

add_executable(${PROJECT_NAME} ${CPP_HEADERS} ${CPP_SOURCES})
#target_link_libraries(${PROJECT_NAME} Network)
#target_link_libraries(${PROJECT_NAME} Routing)
#target_link_libraries(${PROJECT_NAME} Scheduler)
#target_link_libraries(${PROJECT_NAME} XmlLibrary)
