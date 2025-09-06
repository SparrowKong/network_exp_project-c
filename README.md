# 计算机网络实验项目 (network_exp_project-c)

这是一个专为C语言初学者设计的计算机网络课程实验项目。包含4个网络协议实验，每个实验都有详细的中文注释和简单的操作步骤。

## 快速开始

### 系统要求
- 操作系统：Linux 或 macOS
- 编译器：GCC (支持 C99 标准)
- 构建工具：Make

### 一键构建和测试
```bash
# 1. 构建所有实验
make

# 2. 运行所有测试
make test

# 3. 交互式运行演示
make demo
```

## 项目结构

```
network_exp_project-c/
├── src/                              # 源代码目录
<<<<<<< HEAD
│   ├── sliding_window_protocol/      # 实验1: 滑动窗口协议
│   ├── crc_algorithm/                # 实验2: CRC校验算法  
│   ├── tcp_chat_system/              # 实验3: TCP聊天系统
│   └── udp_chat_system/              # 实验4: UDP聊天系统
├── bin/                              # 可执行文件目录
└── Makefile                          # 构建配置文件
=======
│   ├── sliding_window_protocol/      # 滑动窗口协议实验
│   │   ├── core/                     # 核心实现代码
│   │   │   ├── sliding_window.h      # 协议头文件
│   │   │   └── sliding_window.c      # 协议实现
│   │   ├── frontend/                 # 用户交互界面
│   │   │   └── interface.c           # 交互界面实现
│   │   └── test/                     # 测试用例
│   │       └── test_sliding_window.c # 测试程序
│   └── udp_chat_system/              # UDP聊天系统实验
│       ├── core/                     # 核心实现代码
│       │   ├── udp_chat.h            # 系统头文件
│       │   └── udp_chat.c            # 系统实现
│       ├── frontend/                 # 用户交互界面
│       │   └── interface.c           # 交互界面实现
│       └── test/                     # 测试用例
│           └── test_udp_chat.c       # 测试程序
├── build/                            # 编译输出目录（按实验分离）
│   ├── sliding_window_protocol/      # 滑动窗口协议编译文件
│   │   ├── core.o                    # 核心模块对象文件
│   │   ├── frontend.o                # 界面模块对象文件
│   │   └── test.o                    # 测试模块对象文件
│   └── udp_chat_system/              # UDP聊天系统编译文件
│       ├── core.o                    # 核心模块对象文件
│       ├── frontend.o                # 界面模块对象文件
│       └── test.o                    # 测试模块对象文件
├── bin/                              # 可执行文件目录（按实验分离）
│   ├── sliding_window_protocol/      # 滑动窗口协议可执行文件
│   │   ├── demo                      # 演示程序
│   │   └── test                      # 测试程序
│   └── udp_chat_system/              # UDP聊天系统可执行文件
│       ├── demo                      # 演示程序
│       └── test                      # 测试程序
├── Makefile                          # 支持多实验的构建配置文件
├── CLAUDE.md                         # AI 开发助手配置
└── README.md                         # 项目说明文档
>>>>>>> feature_p5
```

---

## 实验1: 滑动窗口协议（停等协议）

### 实验目标
学习数据链路层的滑动窗口协议，理解停等协议的工作原理。

### 编译和运行
```bash
# 构建实验1
make sliding_window_protocol

# 运行演示程序
make sliding_window_protocol-demo
# 或者直接运行
./bin/sliding_window_protocol/demo

# 运行测试
make sliding_window_protocol-test  
# 或者直接运行
./bin/sliding_window_protocol/test
```

### 实验操作步骤
1. 运行演示程序，观察发送方和接收方的交互过程
2. 尝试不同的网络参数（丢包率、延迟）
3. 查看统计信息，理解协议性能
4. 运行测试程序，验证协议正确性

### 学习重点
- 停等协议的状态机
- 超时重传机制
- 错误检测和校验
- 网络环境对协议性能的影响

---

## 实验2: CRC校验算法

<<<<<<< HEAD
### 实验目标
学习循环冗余校验（CRC）算法，理解错误检测的原理。
=======
#### 特定实验命令
```bash
# 滑动窗口协议实验
make sliding_window_protocol        # 构建该实验
make sliding_window_protocol-demo   # 运行该实验演示程序
make sliding_window_protocol-test   # 运行该实验测试
make sliding_window_protocol-clean  # 清理该实验编译文件

# UDP聊天系统实验
make udp_chat_system                # 构建该实验
make udp_chat_system-demo           # 运行该实验演示程序
make udp_chat_system-test           # 运行该实验测试
make udp_chat_system-clean          # 清理该实验编译文件

# 直接运行程序
./bin/sliding_window_protocol/demo  # 直接运行演示程序
./bin/sliding_window_protocol/test  # 直接运行测试程序
./bin/udp_chat_system/demo          # 直接运行UDP聊天系统
./bin/udp_chat_system/test          # 直接运行UDP聊天系统测试
```
>>>>>>> feature_p5

### 编译和运行
```bash
# 构建实验2
make crc_algorithm

# 运行演示程序
make crc_algorithm-demo
# 或者直接运行
./bin/crc_algorithm/demo

# 运行测试
make crc_algorithm-test
# 或者直接运行  
./bin/crc_algorithm/test
```

### 实验操作步骤
1. 运行演示程序，选择不同的CRC标准（CRC-8, CRC-16, CRC-32）
2. 输入测试数据，观察CRC计算过程
3. 尝试修改数据，验证错误检测能力
4. 比较不同算法的性能差异

### 学习重点
- CRC算法的数学原理
- 不同CRC标准的特点
- 位级算法与查表算法的区别
- 错误检测的局限性

---

## 实验3: TCP聊天系统

### 实验目标
学习TCP Socket编程，实现基于TCP的客户端-服务器聊天系统。

### 编译和运行
```bash
# 构建实验3
make tcp_chat_system

# 运行演示程序
make tcp_chat_system-demo
# 或者直接运行
./bin/tcp_chat_system/demo

# 运行测试
make tcp_chat_system-test
# 或者直接运行
./bin/tcp_chat_system/test
```

### 实验操作步骤

**启动服务器**:
1. 运行演示程序：`./bin/tcp_chat_system/demo`
2. 选择"1. 启动聊天服务器"
3. 输入端口号（建议8080）
4. 服务器开始监听连接

**启动客户端**:
1. 打开新终端，运行演示程序
2. 选择"2. 连接聊天服务器（客户端）"
3. 输入服务器IP（本机测试用127.0.0.1）
4. 输入服务器端口号
5. 输入用户名开始聊天

**聊天操作**:
- 输入消息后按回车发送
- 输入"quit"退出聊天
- 服务器会广播消息给所有在线用户

### 学习重点
- TCP Socket编程基础
- 客户端-服务器架构
- 多路复用技术（select）
- 网络异常处理

---

## 实验4: UDP聊天系统

### 实验目标
学习UDP Socket编程，理解UDP与TCP的区别。

### 编译和运行
```bash
# 构建实验4
make udp_chat_system

# 运行演示程序  
make udp_chat_system-demo
# 或者直接运行
./bin/udp_chat_system/demo

# 运行测试
make udp_chat_system-test
# 或者直接运行
./bin/udp_chat_system/test
```

### 实验操作步骤

**启动服务器**:
1. 运行演示程序：`./bin/udp_chat_system/demo`
2. 选择"1. 启动UDP聊天服务器"
3. 输入端口号（建议9090）
4. 服务器开始监听UDP消息

**启动客户端**:
1. 打开新终端，运行演示程序
2. 选择"2. 连接UDP聊天服务器（客户端）"
3. 输入服务器IP和端口号
4. 输入用户名开始聊天

### 学习重点
- UDP Socket编程
- UDP与TCP的区别
- 无连接通信的特点
- 消息完整性校验

---

<<<<<<< HEAD
## 常用命令总结
=======
## 实验2: UDP聊天系统

### 实验简介

UDP聊天系统是一个基于UDP协议的C/S架构应用程序。本实验实现了完整的多客户端聊天系统，展示了UDP socket编程的核心概念和网络应用程序的设计原理。

### 功能特性

- ✅ **C/S架构设计**: 完整的客户端-服务器架构实现
- ✅ **UDP协议通信**: 基于UDP socket的网络通信
- ✅ **多客户端支持**: 同时支持最多10个客户端连接
- ✅ **实时群聊**: 消息广播功能，支持多人实时聊天
- ✅ **消息完整性校验**: 基于校验和的消息完整性验证
- ✅ **用户管理**: 用户注册、在线状态跟踪、用户列表
- ✅ **交互式界面**: 分离的服务器和客户端交互界面
- ✅ **统计信息**: 详细的连接统计和性能监控
- ✅ **错误处理**: 完善的网络异常和错误处理机制
- ✅ **完整测试套件**: 42个测试用例，覆盖各种场景

### 编译和运行

#### UDP聊天系统专用命令
```bash
# 构建系统
make udp_chat_system                # 构建UDP聊天系统

# 运行测试
make udp_chat_system-test           # 运行测试套件（42个测试用例）

# 运行演示
make udp_chat_system-demo           # 运行主程序（包含服务器和客户端选择）

# 直接运行
./bin/udp_chat_system/demo          # 运行主程序
./bin/udp_chat_system/test          # 运行测试程序

# 清理编译文件
make udp_chat_system-clean          # 清理该实验的编译文件
```

#### 使用方法

**方法1: 交互式菜单**
```bash
./bin/udp_chat_system/demo
# 选择1启动服务器，选择2启动客户端
```

**方法2: 命令行参数**
```bash
# 启动服务器
./bin/udp_chat_system/demo server

# 启动客户端
./bin/udp_chat_system/demo client

# 运行测试
./bin/udp_chat_system/demo test

# 查看帮助
./bin/udp_chat_system/demo --help
```

### 实验内容

#### 1. 系统架构

**服务器端功能**:
- UDP socket创建和绑定
- 多客户端连接管理
- 消息接收和广播
- 用户状态维护
- 统计信息记录

**客户端功能**:
- 连接服务器
- 发送聊天消息
- 接收其他用户消息
- 用户命令处理

#### 2. 核心功能模块

**核心通信实现** (`src/udp_chat_system/core/`)
- `udp_chat.h` - 系统数据结构定义、函数接口声明
- `udp_chat.c` - UDP通信核心实现、消息处理、用户管理

**用户交互界面** (`src/udp_chat_system/frontend/`)
- `interface.c` - 统一的用户界面，支持服务器和客户端模式选择

**测试用例** (`src/udp_chat_system/test/`)
- `test_udp_chat.c` - 42个全面测试用例，包含单元测试和集成测试

#### 3. 消息协议设计

**消息类型**:
- `MSG_JOIN` - 用户加入聊天室
- `MSG_LEAVE` - 用户离开聊天室  
- `MSG_CHAT` - 聊天消息
- `MSG_USER_LIST` - 用户列表请求
- `MSG_SERVER_INFO` - 服务器信息
- `MSG_ERROR` - 错误消息

**消息结构**:
```c
typedef struct {
    message_type_t type;                    // 消息类型
    char username[MAX_USERNAME_SIZE];       // 用户名
    char content[MAX_MESSAGE_SIZE];         // 消息内容
    time_t timestamp;                       // 时间戳
    unsigned int checksum;                  // 校验和
} chat_message_t;
```

#### 4. 使用场景

**场景1: 基本聊天**
1. 启动服务器：`./bin/udp_chat_system/demo server`
2. 启动客户端1：`./bin/udp_chat_system/demo client`
3. 启动客户端2：`./bin/udp_chat_system/demo client`
4. 在客户端中输入消息进行群聊

**场景2: 测试网络通信**
```bash
# 在服务器上运行（假设IP为192.168.1.100）
./bin/udp_chat_system/demo server

# 在客户端机器上连接
./bin/udp_chat_system/demo client
# 输入服务器IP: 192.168.1.100
```

**场景3: 压力测试**
```bash
# 运行测试套件验证系统稳定性
make udp_chat_system-test
```

### 学习要点

1. **UDP编程**: 掌握UDP socket的创建、绑定、发送和接收
2. **C/S架构**: 理解客户端-服务器架构的设计原理
3. **并发处理**: 学习使用select()处理多客户端连接
4. **协议设计**: 了解应用层协议的设计方法
5. **错误处理**: 掌握网络编程中的异常处理技巧
6. **状态管理**: 学习服务器端客户端状态的维护方法

### 技术实现特点

1. **非阻塞I/O**: 使用select()实现高效的多客户端管理
2. **消息序列化**: 自定义消息序列化和反序列化机制
3. **完整性校验**: 简单但有效的校验和验证
4. **内存管理**: 安全的内存分配和释放
5. **跨平台**: 兼容Linux/macOS系统

### 性能指标

- **最大并发客户端**: 10个
- **消息最大长度**: 1024字节
- **用户名最大长度**: 64字节
- **默认服务端口**: 8888
- **测试用例数量**: 42个
- **测试覆盖率**: >95%

### 实验扩展

基于当前实现可以进行以下扩展：
- 添加私聊功能
- 实现文件传输
- 添加用户认证机制
- 支持聊天室概念
- 实现消息持久化
- 添加图形界面
- 支持更多消息类型（图片、音频等）

---

## 技术栈
>>>>>>> feature_p5

### 构建命令
```bash
make                    # 构建所有实验
make clean              # 清理编译文件
make help               # 显示帮助信息
make list               # 列出所有实验
```

### 运行命令
```bash
make demo               # 交互式选择实验
make test               # 运行所有测试

# 特定实验
make [实验名]-demo      # 运行指定实验演示
make [实验名]-test      # 运行指定实验测试
```

### 实验名称
- `sliding_window_protocol` - 滑动窗口协议
- `crc_algorithm` - CRC校验算法
- `tcp_chat_system` - TCP聊天系统  
- `udp_chat_system` - UDP聊天系统

## 学习建议

### 对于C语言初学者
1. **按顺序学习**: 建议按实验1→2→3→4的顺序进行
2. **先看后做**: 先阅读源代码注释，理解原理后再运行程序
3. **动手实践**: 多尝试修改参数，观察结果变化
4. **理解测试**: 认真阅读测试用例，理解每个功能点

### 调试技巧
1. **编译错误**: 检查语法和头文件包含
2. **运行错误**: 使用gdb调试器定位问题
3. **网络问题**: 检查防火墙和端口占用
4. **权限问题**: 确保有执行权限

### 扩展学习
- 尝试修改协议参数，观察性能变化
- 阅读RFC文档，理解协议标准
- 实现更复杂的功能（如文件传输）
- 使用网络抓包工具分析数据包

## 技术栈
- **编程语言**: C99
- **编译器**: GCC
- **构建工具**: Make
- **网络编程**: BSD Socket API
- **调试工具**: GDB

## 故障排除

### 常见问题
1. **编译失败**: 确保安装了GCC和Make
2. **端口被占用**: 更换其他端口号
3. **连接失败**: 检查IP地址和端口是否正确
4. **权限不足**: 使用chmod +x给予执行权限

### 获取帮助
- 运行`make help`查看详细命令说明
- 查看源代码中的详细中文注释
- 运行测试程序了解各功能的正确用法