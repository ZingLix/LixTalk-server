[English](https://github.com/ZingLix/LixTalk-server/blob/master/Readme.md) | 简体中文

# LixTalk-server

[![Build Status](https://travis-ci.com/ZingLix/LixTalk-server.svg?branch=master)](https://travis-ci.com/ZingLix/LixTalk-server)  [![CodeFactor](https://www.codefactor.io/repository/github/zinglix/lixtalk-server/badge)](https://www.codefactor.io/repository/github/zinglix/lixtalk-server)

LixTalk 是一个高效、安全的即时通讯系统。

这个 repo 是运行于 Linux 上的服务端代码。

[客户端 ->](https://github.com/ZingLix/LixTalk-client)

## Installation

LixTalk 能够在 Ubuntu 16.04 下运行（其他版本和系统尚未测试）.

首先需要安装 LixTalk 需要的依赖

```
sudo apt update
sudo apt-get install libhiredis-dev libmysqlcppconn-dev cmake
```

之后就可以开始构建。高于 5.4 的 g++ 和 高于 6.0 的 clang 都可以完成构建，Cmake 会自动检测你所使用的编译器，需要支持 C++14 。

```
cmake .
make
```

如果没有错误，那么一切已经准备就绪。

```
cd build
./LixTalk
```

Bingo! 服务器已经开始运行，客户端通过正确的 IP 和端口就可以连接。享受 LixTalk 带来的聊天乐趣吧！