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
│   ├── sliding_window_protocol/      # 实验1: 滑动窗口协议
│   ├── crc_algorithm/                # 实验2: CRC校验算法  
│   ├── tcp_chat_system/              # 实验3: TCP聊天系统
│   └── udp_chat_system/              # 实验4: UDP聊天系统
├── bin/                              # 可执行文件目录
└── Makefile                          # 构建配置文件
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

### 实验目标
学习循环冗余校验（CRC）算法，理解错误检测的原理。

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

## 常用命令总结

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