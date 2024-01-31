cmake_minimum_required(VERSION 3.17)

#download third_party libraries
include(FetchContent)
FetchContent_Populate(asio
  GIT_REPOSITORY    https://github.com/chriskohlhoff/asio.git
  GIT_TAG           asio-1-18-0
  SOURCE_DIR        ${CMAKE_SOURCE_DIR}/third_party/asio
)

FetchContent_Populate(cereal
  GIT_REPOSITORY    https://github.com/USCiLab/cereal.git
  GIT_TAG           v1.3.0
  SOURCE_DIR        ${CMAKE_SOURCE_DIR}/third_party/cereal
)

FetchContent_Populate(spdlog
  GIT_REPOSITORY    https://github.com/gabime/spdlog.git
  GIT_TAG           v1.8.1
  SOURCE_DIR        ${CMAKE_SOURCE_DIR}/third_party/spdlog
)

FetchContent_Populate(reader_writer_queue
  GIT_REPOSITORY    https://github.com/cameron314/concurrentqueue.git
  GIT_TAG           1.0.2
  SOURCE_DIR        ${CMAKE_SOURCE_DIR}/third_party/reader_writer_queue
)

add_definitions(-DASIO_STANDALONE)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (WIN32)
    add_definitions(-D_WIN32_WINNT=0x0A00)
endif(WIN32)

project(message_distributor)

include_directories(include)
include_directories(third_party/asio/asio/include)
include_directories(third_party/cereal/include)
include_directories(third_party/spdlog/include)
include_directories(third_party/reader_writer_queue)

add_subdirectory(client)

file (GLOB sources
    src/*.cpp)
    
add_executable(message_distributor ${sources})
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(message_distributor Threads::Threads)
