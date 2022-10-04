## Mac
### 安装相关包
```sh
brew install grpc cmake openssl@1.1
```

### 生成相关文件
#### 生成编解码文件
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

#### 生成服务接口文件
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

### 编译
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

### 运行
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
