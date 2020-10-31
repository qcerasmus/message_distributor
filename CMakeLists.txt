cmake_minimum_required(VERSION 3.9)

add_definitions(-DASIO_STANDALONE)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (WIN32)
    add_definitions(-D_WIN32_WINNT=0x0A00)
endif(WIN32)

project(message_distributor)

include_directories(include)
include_directories(third_party/asio/asio/include)
include_directories(third_party/cereal/include)
include_directories(third_party/spdlog/include)
include_directories(third_party/reader_writer_queue)

add_subdirectory(third_party)
add_subdirectory(client)

file (GLOB sources
    src/*.cpp)
    
add_executable(message_distributor ${sources})