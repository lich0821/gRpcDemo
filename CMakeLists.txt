cmake_minimum_required(VERSION 3.1)

project(wcf)

set(CMAKE_CXX_STANDARD 17)

find_package(Protobuf REQUIRED)
find_package(gRPC CONFIG REQUIRED)

add_executable(server src/main.cpp proto/wcf.grpc.pb.cc proto/wcf.pb.cc)
add_executable(client src/client.cpp proto/wcf.grpc.pb.cc proto/wcf.pb.cc)

target_link_libraries(server PRIVATE gRPC::grpc++)
target_link_libraries(client PRIVATE gRPC::grpc++)
