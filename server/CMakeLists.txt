cmake_minimum_required(VERSION 3.1)

project(server)

set(CMAKE_CXX_STANDARD 17)

if(WIN32)
    set(CMAKE_GENERATOR_PLATFORM Win32)
elseif(APPLE)
    set(CMAKE_THREAD_LIBS_INIT "-lpthread")
    set(CMAKE_HAVE_THREADS_LIBRARY 1)
    set(CMAKE_USE_WIN32_THREADS_INIT 0)
    set(CMAKE_USE_PTHREADS_INIT 1)
    set(THREADS_PREFER_PTHREAD_FLAG ON)
endif()

find_package(Protobuf REQUIRED)
find_package(gRPC CONFIG REQUIRED)

add_executable(server main.cpp demo.grpc.pb.cc demo.pb.cc)
target_link_libraries(server PRIVATE gRPC::grpc++)
