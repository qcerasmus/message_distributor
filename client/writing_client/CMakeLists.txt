cmake_minimum_required(VERSION 3.17)

add_executable(writing_client client.cpp)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(writing_client Threads::Threads)