English | [简体中文](https://github.com/ZingLix/LixTalk-server/blob/master/Readme.zh.md)

# LixTalk-server

![](https://travis-ci.com/ZingLix/LixTalk-server.svg?branch=master)  [![CodeFactor](https://www.codefactor.io/repository/github/zinglix/lixtalk-server/badge)](https://www.codefactor.io/repository/github/zinglix/lixtalk-server)

LixTalk is a high-performance secure IM system.

This repo is the source code of server which runs on Linux.

[Client ->](https://github.com/ZingLix/LixTalk-client)

## Installation

LixTalk works on Ubuntu 16.04 (have not been tested on other version).

LixTalk needs several dependencies, so you should first

```
sudo apt-get update
sudo apt-get install libhiredis-dev libmysqlcppconn-dev cmake
```

Then build it. Both g++ above 5 and clang above 6.0 work well. Cmake will detect which compiler you're using. C++14 support is required.

```
cmake .
make
```

If no errors occur, then everything is ready. 

```
cd build
./LixTalk
```

Bingo! The server is running and the client can connect with correct IP and port. Enjoy chatting!

