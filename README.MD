# A gRPC Demo
工程结构：
```txt
gRpcDemo
├── CMakeLists.txt
├── LICENSE
├── README.MD
├── build
├── proto
│   └── demo.proto
└── src
    ├── client.cpp
    └── server.cpp
```

## 1 MacOS
### 1.1 安装相关包
```sh
brew install grpc cmake openssl@1.1
```

### 1.2 生成相关文件
#### 1.2.1 生成编解码文件
```sh
cd gRpcDemo/proto/
protoc -I=. --cpp_out=. demo.proto
```

执行 `tree proto`，可见生成了两个 `*.pb.*` 文件：
```txt
proto
├── demo.pb.cc
├── demo.pb.h
└── demo.proto
```

#### 1.2.2 生成服务接口文件
```sh
protoc -I=. --grpc_out=. --plugin=protoc-gen-grpc="/usr/local/bin/grpc_cpp_plugin" demo.proto
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

### 1.3 编译
新建 build 目录，进入该目录进行编译：
```sh
mkdir build
cd build
export PKG_CONFIG_PATH="/usr/local/opt/openssl@1.1/lib/pkgconfig"

# 生成 Makefile
cmake .. # 注意这里有两个点，表示 CMakeLists.txt 在上层目录

# 编译
make
```

### 1.4 运行
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

## 2 Windows
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
vcpkg install grpc
vcpkg install protobuf[zlib]
vcpkg integrate install
```

#### 2.1.3 安装 VS2019

### 2.2 生成相关文件
#### 2.2.1 生成编解码文件
```sh
cd gRpcDemo/proto/
protoc -I=. --cpp_out=. demo.proto
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
protoc -I=. --grpc_out=. --plugin=protoc-gen-grpc="C:\Tools\vcpkg\packages\grpc_x64-windows\tools\grpc\grpc_cpp_plugin.exe" demo.proto
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
# 生成 VS 工程
cmake ../ -DCMAKE_TOOLCHAIN_FILE=C:/Tools/vcpkg/scripts/buildsystems/vcpkg.cmake

# 编译
cmake --build .
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
