# Makefile for Sliding Window Protocol Experiment
# 滑动窗口协议实验构建文件

# 编译器设置
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g -O2
LDFLAGS = -lm

# 目录设置
SRC_DIR = src/sliding_window_protocol
CORE_DIR = $(SRC_DIR)/core
FRONTEND_DIR = $(SRC_DIR)/frontend
TEST_DIR = $(SRC_DIR)/test
BUILD_DIR = build
BIN_DIR = bin

# 源文件
CORE_SOURCES = $(CORE_DIR)/sliding_window.c
FRONTEND_SOURCES = $(FRONTEND_DIR)/interface.c
TEST_SOURCES = $(TEST_DIR)/test_sliding_window.c

# 目标文件
CORE_OBJECTS = $(BUILD_DIR)/sliding_window.o
FRONTEND_OBJECTS = $(BUILD_DIR)/interface.o
TEST_OBJECTS = $(BUILD_DIR)/test_sliding_window.o

# 可执行文件
MAIN_EXECUTABLE = $(BIN_DIR)/sliding_window_demo
TEST_EXECUTABLE = $(BIN_DIR)/test_sliding_window

# 默认目标
.PHONY: all clean test demo help install directories

all: directories $(MAIN_EXECUTABLE) $(TEST_EXECUTABLE)
	@echo "构建完成！"
	@echo "运行演示程序: make demo"
	@echo "运行测试程序: make test"

# 创建必要的目录
directories:
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(BIN_DIR)

# 编译核心模块
$(CORE_OBJECTS): $(CORE_SOURCES) $(CORE_DIR)/sliding_window.h
	@echo "编译核心模块..."
	$(CC) $(CFLAGS) -c $(CORE_SOURCES) -o $@

# 编译前端界面模块
$(FRONTEND_OBJECTS): $(FRONTEND_SOURCES) $(CORE_DIR)/sliding_window.h
	@echo "编译前端界面模块..."
	$(CC) $(CFLAGS) -c $(FRONTEND_SOURCES) -o $@

# 编译测试模块
$(TEST_OBJECTS): $(TEST_SOURCES) $(CORE_DIR)/sliding_window.h
	@echo "编译测试模块..."
	$(CC) $(CFLAGS) -c $(TEST_SOURCES) -o $@

# 构建主程序（演示程序）
$(MAIN_EXECUTABLE): $(CORE_OBJECTS) $(FRONTEND_OBJECTS)
	@echo "链接主程序..."
	$(CC) $(CORE_OBJECTS) $(FRONTEND_OBJECTS) $(LDFLAGS) -o $@

# 构建测试程序
$(TEST_EXECUTABLE): $(CORE_OBJECTS) $(TEST_OBJECTS)
	@echo "链接测试程序..."
	$(CC) $(CORE_OBJECTS) $(TEST_OBJECTS) $(LDFLAGS) -o $@

# 运行演示程序
demo: $(MAIN_EXECUTABLE)
	@echo "启动滑动窗口协议演示程序..."
	@echo "=========================================="
	./$(MAIN_EXECUTABLE)

# 运行测试
test: $(TEST_EXECUTABLE)
	@echo "运行滑动窗口协议测试套件..."
	@echo "=========================================="
	./$(TEST_EXECUTABLE)
	@echo "=========================================="
	@if [ $$? -eq 0 ]; then \
		echo "✓ 所有测试通过！"; \
	else \
		echo "✗ 测试失败，请检查输出"; \
	fi

# 清理编译文件
clean:
	@echo "清理编译文件..."
	rm -rf $(BUILD_DIR) $(BIN_DIR)
	@echo "清理完成"

# 安装（复制到系统目录，可选）
install: all
	@echo "安装程序到 /usr/local/bin/..."
	@sudo cp $(MAIN_EXECUTABLE) /usr/local/bin/sliding_window_demo
	@sudo cp $(TEST_EXECUTABLE) /usr/local/bin/test_sliding_window
	@echo "安装完成"

# 卸载
uninstall:
	@echo "卸载程序..."
	@sudo rm -f /usr/local/bin/sliding_window_demo
	@sudo rm -f /usr/local/bin/test_sliding_window
	@echo "卸载完成"

# 检查代码风格（可选，需要安装相应工具）
lint:
	@echo "检查代码风格..."
	@which clang-format > /dev/null 2>&1 && \
		find $(SRC_DIR) -name "*.c" -o -name "*.h" | xargs clang-format --dry-run --Werror || \
		echo "clang-format 未安装，跳过代码风格检查"

# 格式化代码（可选，需要安装相应工具）
format:
	@echo "格式化代码..."
	@which clang-format > /dev/null 2>&1 && \
		find $(SRC_DIR) -name "*.c" -o -name "*.h" | xargs clang-format -i || \
		echo "clang-format 未安装，跳过代码格式化"

# 生成文档（可选，需要安装doxygen）
docs:
	@echo "生成文档..."
	@which doxygen > /dev/null 2>&1 && \
		doxygen Doxyfile || \
		echo "doxygen 未安装，跳过文档生成"

# 调试版本
debug: CFLAGS += -DDEBUG -g3
debug: all
	@echo "调试版本构建完成"

# 发布版本
release: CFLAGS += -DNDEBUG -O3
release: all
	@echo "发布版本构建完成"

# 内存检查（需要安装valgrind）
memcheck: $(TEST_EXECUTABLE)
	@echo "运行内存检查..."
	@which valgrind > /dev/null 2>&1 && \
		valgrind --leak-check=full --show-leak-kinds=all ./$(TEST_EXECUTABLE) || \
		echo "valgrind 未安装，跳过内存检查"

# 性能分析（需要安装gprof相关工具）
profile: CFLAGS += -pg
profile: all
	@echo "性能分析版本构建完成"
	@echo "运行程序后使用 gprof 查看分析结果"

# 显示帮助信息
help:
	@echo "可用的 make 目标："
	@echo "  all      - 构建所有程序（默认）"
	@echo "  demo     - 运行演示程序"
	@echo "  test     - 运行测试程序"
	@echo "  clean    - 清理编译文件"
	@echo "  install  - 安装到系统目录"
	@echo "  uninstall- 从系统目录卸载"
	@echo "  lint     - 检查代码风格"
	@echo "  format   - 格式化代码"
	@echo "  docs     - 生成文档"
	@echo "  debug    - 构建调试版本"
	@echo "  release  - 构建发布版本"
	@echo "  memcheck - 运行内存检查"
	@echo "  profile  - 构建性能分析版本"
	@echo "  help     - 显示此帮助信息"
	@echo ""
	@echo "示例用法："
	@echo "  make         # 构建所有程序"
	@echo "  make demo    # 运行演示"
	@echo "  make test    # 运行测试"
	@echo "  make clean   # 清理文件"

# 显示项目信息
info:
	@echo "滑动窗口协议实验项目信息"
	@echo "========================"
	@echo "项目名称: 滑动窗口协议（停等协议）模拟"
	@echo "编译器:   $(CC)"
	@echo "编译选项: $(CFLAGS)"
	@echo "链接选项: $(LDFLAGS)"
	@echo "源文件:"
	@echo "  核心模块: $(CORE_SOURCES)"
	@echo "  界面模块: $(FRONTEND_SOURCES)"
	@echo "  测试模块: $(TEST_SOURCES)"
	@echo "可执行文件:"
	@echo "  演示程序: $(MAIN_EXECUTABLE)"
	@echo "  测试程序: $(TEST_EXECUTABLE)"