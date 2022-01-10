cmake_minimum_required(VERSION 3.6)

set(CMAKE_CXX_STANDARD 11)
set(THREADS_PREFER_PTHREAD_FLAG ON)


project(proj2)

find_package(Threads REQUIRED)

add_executable(proj2 main.cpp md5.cpp)

install(TARGETS proj2 RUNTIME DESTINATION bin)
target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)
