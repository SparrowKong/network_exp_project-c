# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

这是一个计算机网络课程的教学实验项目（`network_exp_project-c`），包含多个网络协议的实现和演示。项目专为教学设计，具有详细的中文注释和简洁的代码结构。

## Repository Structure

项目采用以下目录结构：
```
network_exp_project-c/
├── src/                              # 源代码目录
│   └── sliding_window_protocol/      # 滑动窗口协议实验
│       ├── core/                     # 核心实现代码
│       │   ├── sliding_window.h      # 协议头文件
│       │   └── sliding_window.c      # 协议实现
│       ├── frontend/                 # 用户交互界面
│       │   └── interface.c           # 交互界面实现
│       └── test/                     # 测试用例
│           └── test_sliding_window.c # 测试程序
├── build/                            # 编译输出目录（自动创建）
├── bin/                              # 可执行文件目录（自动创建）
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

## Commands

### 构建项目
```bash
make                    # 构建所有程序
make all                # 同上
make clean              # 清理编译文件
make help               # 显示帮助信息
```

### 运行程序
```bash
make demo               # 运行滑动窗口协议演示程序
make test               # 运行所有测试用例
./bin/sliding_window_demo        # 直接运行演示程序
./bin/test_sliding_window        # 直接运行测试程序
```

### 开发工具
```bash
make debug              # 构建调试版本
make release            # 构建发布版本
make lint               # 代码风格检查（需要clang-format）
make format             # 代码格式化
make memcheck           # 内存检查（需要valgrind）
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

### 扩展建议
- 可以基于现有框架添加更多协议实验
- 支持更复杂的网络协议（如选择重传、流量控制）
- 添加图形化界面或Web界面
- 集成网络协议分析工具

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

## Notes for Future Development
- 保持代码的教学特性，注重可读性和注释质量
- 每个新实验都应该有完整的文档和测试
- 考虑添加更多的可视化功能帮助理解协议原理
- 保持项目结构的一致性和模块化设计
- 严格遵守用户设定的开发指导原则和代码结构要求