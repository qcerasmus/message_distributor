cmake_minimum_required(VERSION 3.9)

include(ExternalProject)

#include asio for easy tcp.
ExternalProject_Add(asio
    GIT_REPOSITORY      https://github.com/chriskohlhoff/asio.git
    GIT_TAG             asio-1-18-0
    SOURCE_DIR          "${CMAKE_SOURCE_DIR}/third_party/asio"
    CONFIGURE_COMMAND   ""
    BUILD_COMMAND       ""
    INSTALL_COMMAND     ""
    TEST_COMMAND        ""
)

#include cereal for serialization.
ExternalProject_Add(cereal
    GIT_REPOSITORY      https://github.com/USCiLab/cereal.git
    GIT_TAG             v1.3.0
    SOURCE_DIR          "${CMAKE_SOURCE_DIR}/third_party/cereal"
    CONFIGURE_COMMAND   ""
    BUILD_COMMAND       ""
    INSTALL_COMMAND     ""
    TEST_COMMAND        ""
)

#include spdlog
ExternalProject_Add(spdlog
    GIT_REPOSITORY      https://github.com/gabime/spdlog.git
    GIT_TAG             v1.8.1
    SOURCE_DIR          "${CMAKE_SOURCE_DIR}/third_party/spdlog"
    CONFIGURE_COMMAND   ""
    BUILD_COMMAND       ""
    INSTALL_COMMAND     ""
    TEST_COMMAND        ""
)

ExternalProject_Add(reader_writer_queue
    GIT_REPOSITORY      https://github.com/cameron314/concurrentqueue.git
    GIT_TAG             1.0.2
    SOURCE_DIR          "${CMAKE_SOURCE_DIR}/third_party/reader_writer_queue"
    CONFIGURE_COMMAND   ""
    BUILD_COMMAND       ""
    INSTALL_COMMAND     ""
    TEST_COMMAND        ""
)
