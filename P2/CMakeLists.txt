cmake_minimum_required(VERSION 3.6)
project(network)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wall" )
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -glldb")

add_executable(bufferUnitTest
        "sendFile.cpp"
        "recvFile.cpp"
        "ReceiverWindow.cc"
        "SenderWindow.cc"
        "Slot.cc"
        "Window.cc"
        )

