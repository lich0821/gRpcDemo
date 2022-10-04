cmake_minimum_required(VERSION 3.1)

project(demo)

find_package(Protobuf REQUIRED)
find_package(gRPC CONFIG REQUIRED)

add_executable(server src/server.cpp proto/demo.grpc.pb.cc proto/demo.pb.cc)
add_executable(client src/client.cpp proto/demo.grpc.pb.cc proto/demo.pb.cc)

target_link_libraries(server PRIVATE gRPC::grpc++)
target_link_libraries(client PRIVATE gRPC::grpc++)
