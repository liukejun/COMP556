cmake_minimum_required(VERSION 2.8)
project(network)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wall -std=c++11" )
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -glldb")

add_library(baselib STATIC
        "ReceiverWindow.cc"
        "SenderWindow.cc"
        "Slot.cc"
        "Window.cc")
add_executable(sender
        "sendFile.cpp"
        )
target_link_libraries(sender baselib)

add_executable(receiver
        "recvFile.cpp"
        )
target_link_libraries(receiver baselib)