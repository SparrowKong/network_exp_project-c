# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

这是一个计算机网络课程的教学实验项目（`network_exp_project-c`），包含多个网络协议的实现和演示。项目专为教学设计，具有详细的中文注释和简洁的代码结构。

## Repository Structure

项目采用以下目录结构：
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
<<<<<<< HEAD
│   ├── crc_algorithm/                # CRC校验算法实验
│   │   ├── core/                     # 核心实现代码
│   │   │   ├── crc_algorithm.h       # CRC算法头文件
│   │   │   └── crc_algorithm.c       # CRC算法实现
│   │   ├── frontend/                 # 用户交互界面
│   │   │   └── interface.c           # 交互界面实现
│   │   └── test/                     # 测试用例
│   │       └── test_crc_algorithm.c  # 测试程序
│   ├── tcp_chat_system/              # TCP聊天系统实验
│   │   ├── core/                     # 核心实现代码
│   │   │   ├── tcp_chat.h            # TCP聊天协议头文件
│   │   │   └── tcp_chat.c            # TCP通信核心实现
│   │   ├── frontend/                 # 用户交互界面
│   │   │   └── interface.c           # 聊天界面实现
│   │   └── test/                     # 测试用例
│   │       └── test_tcp_chat.c       # 完整测试套件
=======
>>>>>>> feature_p5
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
<<<<<<< HEAD
│   ├── crc_algorithm/                # CRC算法编译文件
│   ├── tcp_chat_system/              # TCP聊天系统编译文件
│   └── udp_chat_system/              # UDP聊天系统编译文件
├── bin/                              # 可执行文件目录（按实验分离）
│   ├── sliding_window_protocol/      # 滑动窗口协议可执行文件
│   ├── crc_algorithm/                # CRC算法可执行文件
│   ├── tcp_chat_system/              # TCP聊天系统可执行文件
=======
│   └── udp_chat_system/              # UDP聊天系统编译文件
├── bin/                              # 可执行文件目录（按实验分离）
│   ├── sliding_window_protocol/      # 滑动窗口协议可执行文件
>>>>>>> feature_p5
│   └── udp_chat_system/              # UDP聊天系统可执行文件
├── Makefile                          # 构建配置文件
├── CLAUDE.md                         # AI开发助手配置
├── README.md                         # 项目说明文档
└── .gitignore                        # Git忽略配置
```

## Current Experiments

### 1. 滑动窗口协议（停等协议）- sliding_window_protocol

**位置**: `src/sliding_window_protocol/`

**功能**: 实现停等协议（窗口大小为1的滑动窗口协议），包含：
- 完整的发送方和接收方状态机
- 网络环境模拟（丢包、延迟）
- 错误检测和校验和机制
- 超时重传功能
- 性能统计和分析

**核心文件**:
- `core/sliding_window.h` - 协议数据结构和函数声明
- `core/sliding_window.c` - 协议核心实现逻辑
- `frontend/interface.c` - 用户交互界面
- `test/test_sliding_window.c` - 完整测试套件

<<<<<<< HEAD
### 2. CRC校验算法 - crc_algorithm

**位置**: `src/crc_algorithm/`

**功能**: 实现多种标准CRC校验算法，包含：
- 多种CRC标准（CRC-8、CRC-16、CRC-16-CCITT、CRC-32）
- 位级算法和查表算法两种实现方式
- 详细的教学演示和步骤展示
- 错误检测能力验证和模拟
- 算法性能对比分析
- 批量测试和标准测试向量验证

**核心文件**:
- `core/crc_algorithm.h` - CRC算法数据结构和函数声明
- `core/crc_algorithm.c` - CRC算法核心实现（位级+查表）
- `frontend/interface.c` - 完整交互菜单系统
- `test/test_crc_algorithm.c` - 14个测试用例的完整测试套件

### 3. TCP聊天系统 - tcp_chat_system

**位置**: `src/tcp_chat_system/`

**功能**: 基于TCP Socket原语实现C/S架构的聊天系统，包含：
- 完整的TCP服务端和客户端实现
- 多客户端并发连接支持（使用select多路复用）
- 用户登录、聊天、退出功能
- 实时消息广播和用户列表管理
- 网络异常处理和连接状态监控
- 详细的统计信息和性能分析
- 完整的错误处理和边界条件测试

**核心文件**:
- `core/tcp_chat.h` - TCP通信协议数据结构和函数声明
- `core/tcp_chat.c` - TCP Socket通信核心实现
- `frontend/interface.c` - 客户端和服务端交互界面
- `test/test_tcp_chat.c` - 完整的测试套件（包含10+项测试）

### 4. UDP聊天系统 - udp_chat_system
=======
### 2. UDP聊天系统 - udp_chat_system
>>>>>>> feature_p5

**位置**: `src/udp_chat_system/`

**功能**: 基于UDP协议的C/S架构聊天系统，包含：
- 完整的UDP socket通信实现
- 支持多客户端同时连接（最多10个）
- 消息广播和群聊功能
- 消息完整性校验（校验和机制）
- 用户管理和在线状态跟踪
- 详细的统计信息和日志记录
- 交互式服务器和客户端界面

**核心文件**:
- `core/udp_chat.h` - 系统数据结构和函数声明
- `core/udp_chat.c` - UDP通信和聊天功能实现
- `frontend/interface.c` - 统一用户交互界面
- `test/test_udp_chat.c` - 完整测试套件（42个测试用例）

## Commands

### 构建项目
```bash
make                    # 构建所有实验
make list               # 列出所有可用实验
make experiments        # 构建所有实验（同make）
make clean              # 清理所有编译文件
make help               # 显示详细帮助信息
```

### 运行程序
```bash
# 通用命令
make demo               # 交互式选择并运行演示
make test               # 运行所有实验的测试

# 特定实验命令
make sliding_window_protocol-demo   # 运行滑动窗口协议演示
make sliding_window_protocol-test   # 运行滑动窗口协议测试
<<<<<<< HEAD
make crc_algorithm-demo             # 运行CRC算法演示
make crc_algorithm-test             # 运行CRC算法测试
make tcp_chat_system-demo           # 运行TCP聊天系统演示
make tcp_chat_system-test           # 运行TCP聊天系统测试
=======
>>>>>>> feature_p5
make udp_chat_system-demo           # 运行UDP聊天系统演示
make udp_chat_system-test           # 运行UDP聊天系统测试

# 直接运行
<<<<<<< HEAD
./bin/sliding_window_protocol/demo  # 直接运行滑动窗口协议演示
./bin/sliding_window_protocol/test  # 直接运行滑动窗口协议测试
./bin/crc_algorithm/demo            # 直接运行CRC算法演示
./bin/crc_algorithm/test            # 直接运行CRC算法测试
./bin/tcp_chat_system/demo          # 直接运行TCP聊天系统演示程序
./bin/tcp_chat_system/test          # 直接运行TCP聊天系统测试程序
=======
./bin/sliding_window_protocol/demo  # 直接运行演示程序
./bin/sliding_window_protocol/test  # 直接运行测试程序
>>>>>>> feature_p5
./bin/udp_chat_system/demo          # 直接运行UDP聊天系统
./bin/udp_chat_system/test          # 直接运行UDP聊天系统测试
```

### 开发工具
```bash
make info               # 显示项目信息
make debug              # 显示调试信息和目录结构检查
make install            # 安装到系统目录
make uninstall          # 从系统目录卸载

# 特定实验管理
make [实验名]           # 构建特定实验
make [实验名]-demo      # 运行特定实验演示
make [实验名]-test      # 运行特定实验测试  
make [实验名]-clean     # 清理特定实验编译文件
```

## Development Guidelines

### 项目开发要求
- **中文注释**: 所有代码注释和输出都使用中文，便于学习理解
- **教学导向**: 代码结构清晰，便于教学讲解
- **模块化设计**: 每个实验独立在src/下的子目录中
- **三层架构**: core/（核心实现）、frontend/（界面）、test/（测试）
- **完善测试**: 每个实验都包含完整的测试用例

### 添加新实验
1. 在`src/`下创建新的实验目录，名称要清晰反映实验内容
2. 创建三个子目录：`core/`, `frontend/`, `test/`
3. 在`core/`中实现协议核心逻辑
4. 在`frontend/`中创建用户交互界面
5. 在`test/`中编写全面的测试用例
6. 更新`Makefile`添加新实验的构建规则
7. 更新`README.md`添加新实验的说明文档

### 代码规范
- 使用C99标准
- 函数命名使用下划线分隔（snake_case）
- 结构体使用`_t`后缀
- 每个函数都有详细的中文参数说明
- 包含错误处理和边界条件检查
- 使用`printf`输出详细的执行过程信息

### 测试要求
- 每个实验至少包含10个测试用例
- 覆盖正常功能、边界条件、错误处理
- 使用断言验证结果
- 提供详细的测试报告

## Architecture Notes

### 滑动窗口协议实现要点
- 使用状态机模式实现发送方和接收方
- 通过全局缓冲区模拟网络传输
- 使用简单校验和进行错误检测
- 实现可配置的网络环境参数
- 提供详细的统计信息和性能分析

<<<<<<< HEAD
### CRC校验算法实现要点
- 支持多种标准CRC算法（CRC-8、CRC-16、CRC-16-CCITT、CRC-32）
- 实现位级算法和查表算法两种方式
- 提供详细的计算步骤演示和教学功能
- 包含错误检测能力验证和性能对比分析
- 支持批量测试和标准测试向量验证

### TCP聊天系统实现要点
- 基于标准TCP Socket API实现C/S架构
- 服务端使用select多路复用支持多客户端并发
- 实现完整的消息协议（登录、聊天、系统消息等）
- 客户端和服务端独立的状态管理
- 支持用户认证、消息广播、在线用户管理
- 网络异常处理和优雅断开连接
- 完整的测试覆盖（基础功能、边界条件、性能测试）
- 教学导向的中文注释和清晰代码结构

=======
>>>>>>> feature_p5
### UDP聊天系统实现要点
- 基于UDP socket实现C/S架构通信
- 使用select()实现非阻塞I/O和多客户端管理
- 自定义消息协议支持多种消息类型（加入、离开、聊天、服务器信息等）
- 消息完整性校验确保数据传输可靠性
- 服务器端维护客户端列表实现消息广播
- 客户端和服务器分离的交互式界面设计
- 完整的错误处理和异常情况处理机制

### 扩展建议
- 可以基于现有框架添加更多协议实验
- 支持更复杂的网络协议（如选择重传、流量控制）
- 添加图形化界面或Web界面
- 集成网络协议分析工具
- 实现更复杂的聊天功能（私聊、群组、文件传输）
- 添加数据库支持和用户管理系统

## User Requirements and Preferences

以下是用户设定的重要开发指导原则：

- **使用中文描述和回答**: 所有交流和文档都使用中文
- **教学导向的项目**: 这个项目是为计算机网络课程开发的实验项目代码，需要为学习的同学们进行讲解
- **代码简洁易懂**: 代码的结构尽量简单化，代码的注释尽量的丰富
- **完整的交互界面**: 除了实现核心功能外，为每一个实验创建一个输入输出界面，不需要很复杂，能完整展示开发的内容就可以
- **必须包含测试**: 每个实验都需要编写测试用例

### 严格的代码结构要求:
1. **实验独立性**: 每个实验单独放在src下的子目录中，子目录的名字能够清晰的反映实验的内容
2. **三层架构**: 每个实验的核心实现代码放在/core下，交互界面代码放在/frontend下，测试用例放在/test下
3. **文档更新**: 每生成一个实验，将文档内容放在README.md下，并清楚的分割每个实验
4. 注意每个实验的makefile和编译后的目标路径，也要像src一样，做好不同实验的隔离

## Notes for Future Development
- 保持代码的教学特性，注重可读性和注释质量
- 每个新实验都应该有完整的文档和测试
- 考虑添加更多的可视化功能帮助理解协议原理
- 保持项目结构的一致性和模块化设计
- 严格遵守用户设定的开发指导原则和代码结构要求