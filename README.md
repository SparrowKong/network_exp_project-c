# 计算机网络实验项目 (network_exp_project-c)

这是一个用于计算机网络课程的教学实验项目，包含多个网络协议的实现和演示。每个实验都具有详细的中文注释，简洁的代码结构，以及完整的测试用例。

## 项目结构

```
network_exp_project-c/
├── src/                              # 源代码目录
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
```

---

## 实验1: 滑动窗口协议（停等协议）

### 实验简介

滑动窗口协议是计算机网络中重要的数据链路层协议。本实验实现了最简单的滑动窗口协议——停等协议（Stop-and-Wait Protocol），窗口大小为1。

### 功能特性

- ✅ **完整的停等协议实现**: 包含发送方和接收方的完整状态机
- ✅ **网络环境模拟**: 支持可配置的丢包率和网络延迟
- ✅ **错误检测与校验**: 实现简单的校验和机制
- ✅ **超时重传**: 支持超时检测和自动重传
- ✅ **详细的中文日志**: 实时显示协议执行过程
- ✅ **性能统计**: 提供传输成功率、重传率等统计信息
- ✅ **交互式界面**: 用户友好的命令行界面
- ✅ **完善的测试**: 包含10个测试用例，覆盖各种场景

### 编译和运行

#### 通用构建命令
```bash
make                          # 构建所有实验
make list                     # 列出所有可用实验
make experiments              # 构建所有实验（同make）
make test                     # 运行所有实验的测试
make demo                     # 交互式选择并运行演示
make clean                    # 清理所有编译文件
make help                     # 显示详细帮助信息
```

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

#### 系统安装（可选）
```bash
make install        # 安装到系统目录 /usr/local/bin/
make uninstall      # 从系统目录卸载
```

### 实验内容

#### 1. 协议原理
停等协议的工作流程：
1. 发送方发送一个数据帧，启动计时器
2. 等待接收方的确认帧（ACK）
3. 如果收到正确的ACK，发送下一帧
4. 如果超时或收到错误的ACK，重传当前帧
5. 重复上述过程直到传输完成

#### 2. 核心功能模块

**核心协议实现** (`src/sliding_window_protocol/core/`)
- `sliding_window.h` - 协议头文件，定义数据结构和函数接口
- `sliding_window.c` - 协议核心实现，包含发送、接收、重传逻辑

**用户交互界面** (`src/sliding_window_protocol/frontend/`)
- `interface.c` - 提供友好的命令行界面，支持参数配置和结果展示

**测试用例** (`src/sliding_window_protocol/test/`)
- `test_sliding_window.c` - 包含10个全面的测试用例

#### 3. 实验场景

**理想网络环境**: 
- 丢包率: 0%
- 延迟: 10-50ms
- 预期结果: 无重传，高效传输

**一般网络环境**: 
- 丢包率: 10%
- 延迟: 50-150ms
- 预期结果: 少量重传，传输成功

**恶劣网络环境**: 
- 丢包率: 30%
- 延迟: 200-500ms
- 预期结果: 多次重传，可能传输失败

### 学习要点

1. **理解ARQ协议**: 掌握自动重传请求的基本原理
2. **序列号机制**: 理解序列号的循环使用（0和1）
3. **超时重传**: 学习超时检测和重传策略
4. **错误检测**: 了解简单校验和的计算方法
5. **网络建模**: 理解网络丢包和延迟对协议性能的影响

### 性能分析

通过实验可以观察和分析：
- 网络环境对传输效率的影响
- 超时时间设置的重要性
- 重传次数与网络质量的关系
- 停等协议的优缺点

### 实验扩展

可以基于当前实现进行以下扩展：
- 实现更大窗口的滑动窗口协议
- 添加选择重传（Selective Repeat）机制
- 实现更复杂的错误检测算法
- 添加流量控制机制

---

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

- **编程语言**: C99
- **编译器**: GCC
- **构建工具**: Make
- **测试**: 自定义测试框架
- **平台**: Linux/macOS/Windows（通过MinGW）

## 开发说明

### 代码规范
- 使用中文注释，便于学习理解
- 遵循C99标准，保证代码兼容性
- 每个函数都有详细的参数说明
- 错误处理完善，包含边界条件检查

### 项目特色
- **教学导向**: 代码结构清晰，便于教学讲解
- **中文友好**: 所有输出和注释都使用中文
- **可视化**: 详细的执行过程显示
- **可配置**: 支持多种网络环境配置
- **测试完善**: 包含全面的测试用例

## 贡献指南

欢迎贡献新的网络协议实验或改进现有实现：
1. Fork 本项目
2. 创建新的实验目录 `src/new_experiment/`
3. 按照项目结构创建 `core/`, `frontend/`, `test/` 目录
4. 实现协议并添加测试用例
5. 更新 README.md 添加新实验的说明
6. 提交 Pull Request

## 许可证

本项目采用 MIT 许可证，详见 LICENSE 文件。

## 联系方式

如有问题或建议，请通过以下方式联系：
- 提交 GitHub Issue
- 发送邮件至项目维护者

---

*本项目专为计算机网络课程教学设计，旨在帮助学生更好地理解网络协议原理。*