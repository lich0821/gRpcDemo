# A gRPC Demo
工程结构：
```txt
gRpcDemo
├── LICENSE
├── README.MD
├── cpp
│   ├── CMakeLists.txt
│   ├── build
│   └── client.cpp
├── proto
│   └── demo.proto
└── server
    ├── CMakeLists.txt
    ├── build
    ├── main.cpp
    └── server.hpp
```

## 1 MacOS
服务端使用 C++ 实现。
### 1.1 安装相关包
```sh
brew install grpc cmake openssl@1.1
```

### 1.2 服务端
#### 1.2.1 生成相关文件
##### 1.2.1.1 生成编解码文件
```sh
cd gRpcDemo/server/
protoc --cpp_out=. -I=../proto demo.proto
```

执行 `tree proto`，可见生成了两个 `*.pb.*` 文件：
```txt
proto
├── demo.pb.cc
├── demo.pb.h
└── demo.proto
```

##### 1.2.1.2 生成服务接口文件
```sh
cd gRpcDemo/server/
protoc --grpc_out=. --plugin=protoc-gen-grpc="/usr/local/bin/grpc_cpp_plugin" -I=../proto demo.proto
# 这里需要填写 grpc_cpp_plugin 的绝对路径。安装的时候有提示；也可以通过命令获取：
# which grpc_cpp_plugin
```

执行 `tree proto`，可见生成了两个 `*.grpc.pb.*` 文件：
```txt
proto
├── demo.grpc.pb.cc
├── demo.grpc.pb.h
├── demo.pb.cc
├── demo.pb.h
└── demo.proto
```

#### 1.2.2 编译
新建 build 目录，进入该目录进行编译：
```sh
cd gRpcDemo/server/
mkdir build
cd build
export PKG_CONFIG_PATH="/usr/local/opt/openssl@1.1/lib/pkgconfig"

# 生成 Makefile
cmake .. # 注意这里有两个点，表示 CMakeLists.txt 在上层目录

# 编译
make
```

#### 1.2.3 运行
打开终端：
```sh
./server
```

### 1.3 cpp 客户端
#### 1.3.1 生成相关文件
```sh
cd gRpcDemo/cpp/
protoc --cpp_out=. -I=../proto demo.proto
protoc --grpc_out=. --plugin=protoc-gen-grpc="/usr/local/bin/grpc_cpp_plugin" -I=../proto demo.proto
```

#### 1.3.2 编译
新建 build 目录，进入该目录进行编译：
```sh
cd gRpcDemo/cpp/
mkdir build
cd build
export PKG_CONFIG_PATH="/usr/local/opt/openssl@1.1/lib/pkgconfig"

# 生成 Makefile
cmake .. # 注意这里有两个点，表示 CMakeLists.txt 在上层目录

# 编译
make
```

#### 1.3.3 运行
打开终端：
```sh
./client
```

**注意**：因生产消息的线程随 `server` 启动，如果一直没有 `client` 连上来消费，会导致队列一直增长。

## 2 Windows
Windows 下连接静态库，动态连接更简单，网上参考资料很多。

### 2.1 安装相关包
#### 2.1.1 安装 vcpkg
* 安装，参考[Vcpkg: 总览](https://github.com/microsoft/vcpkg/blob/master/README_zh_CN.md)。
```sh
cd C:\Tools
git clone https://github.com/microsoft/vcpkg
.\vcpkg\bootstrap-vcpkg.bat
```

* 添加全局配置：
环境变量增加 `vcpkg` 所在路径（本文为：`C:\Tools\vcpkg`）。

#### 2.1.2 安装 gRPC 相关组件
```sh
vcpkg install grpc:x86-windows-static
vcpkg install protobuf[zlib]:x86-windows-static
vcpkg integrate install
```

#### 2.1.3 安装 VS2019

### 2.2 生成相关文件
#### 2.2.1 生成编解码文件
```sh
cd gRpcDemo/proto/
protoc --cpp_out=. -I=../proto demo.proto
```

执行 `tree .`，可见生成了两个 `*.pb.*` 文件：
```txt
.
├── demo.pb.cc
├── demo.pb.h
└── demo.proto
```

#### 2.2.2 生成服务接口文件
```sh
protoc --grpc_out=. --plugin=protoc-gen-grpc="C:\Tools\vcpkg\packages\grpc_x64-windows\tools\grpc\grpc_cpp_plugin.exe" -I=../proto demo.proto
# 这里需要填写 grpc_cpp_plugin 的绝对路径，去 vcpkg 的安装目录下找。
```

执行 `tree .`，可见生成了两个 `*.grpc.pb.*` 文件：
```txt
.
├── demo.grpc.pb.cc
├── demo.grpc.pb.h
├── demo.pb.cc
├── demo.pb.h
└── demo.proto
```

### 2.3 编译
新建 build 目录，打开 `Developer Command Prompt for VS 2019`，切换到 `build` 目录：
```sh
# 生成 Makefile（32位）
cmake -A Win32 -S .. -DVCPKG_TARGET_TRIPLET=x86-windows-static -DCMAKE_TOOLCHAIN_FILE=C:/Tools/vcpkg/scripts/buildsystems/vcpkg.cmake
# 生成 Makefile（32位）
# cmake -A x64 -S .. -DVCPKG_TARGET_TRIPLET=x64-windows-static -DCMAKE_TOOLCHAIN_FILE=C:/Tools/vcpkg/scripts/buildsystems/vcpkg.cmake

# 编译
cmake --build . --config Release
```

### 2.4 运行
打开两个终端：
```sh
# 一个终端启动服务端
./server
```

```sh
# 另一个终端启动客户端
./client
```

**注意**：因生产消息的线程随 `server` 启动，如果一直没有 `client` 连上来消费，会导致队列一直增长。
