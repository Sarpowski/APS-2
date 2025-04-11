cmake_minimum_required(VERSION 3.28.1)
project(courseWork)

set(CMAKE_CXX_STANDARD 20)

add_executable(courseWork main.cpp
        src/global.cpp
        src/os.cpp
        src/resource.cpp
        src/task.cpp
        src/test.cpp
        src/event.cpp
        src/semaphore.cpp
        src/semaphore.cpp
)

target_include_directories(courseWork
        PUBLIC ${CMAKE_SOURCE_DIR}/headers
)