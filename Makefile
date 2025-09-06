# Makefile for Network Experiments Project
# 计算机网络实验项目构建文件 - 支持多个实验

# 编译器设置
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g -O2
LDFLAGS = -lm

# 目录设置
SRC_DIR = src
BUILD_DIR = build
BIN_DIR = bin

# 自动发现所有实验
EXPERIMENTS = $(notdir $(wildcard $(SRC_DIR)/*))

# 默认目标
.PHONY: all clean help list experiments test demo install

all: directories $(EXPERIMENTS)
	@echo "构建完成！"
	@echo "可用实验: $(EXPERIMENTS)"
	@echo "运行 'make list' 查看所有实验"
	@echo "运行 'make help' 查看使用帮助"

# 创建必要的目录
directories:
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(BIN_DIR)

# 列出所有可用的实验
list:
	@echo "可用的实验项目:"
	@for exp in $(EXPERIMENTS); do \
		if [ -d "$(SRC_DIR)/$$exp" ]; then \
			echo "  - $$exp"; \
		fi \
	done

# 通用实验构建规则
define EXPERIMENT_TEMPLATE
$(1): directories
	@echo "构建实验: $(1)"
	@mkdir -p $(BUILD_DIR)/$(1)
	@mkdir -p $(BIN_DIR)/$(1)
	
	# 编译核心模块
	@if ls $(SRC_DIR)/$(1)/core/*.c >/dev/null 2>&1; then \
		echo "  编译 $(1) 核心模块..."; \
		for file in $(SRC_DIR)/$(1)/core/*.c; do \
			$(CC) $(CFLAGS) -c "$$$$file" -o $(BUILD_DIR)/$(1)/core_$$$$(basename "$$$$file" .c).o; \
		done; \
	fi
	
	# 编译前端模块
	@if ls $(SRC_DIR)/$(1)/frontend/*.c >/dev/null 2>&1; then \
		echo "  编译 $(1) 前端模块..."; \
		for file in $(SRC_DIR)/$(1)/frontend/*.c; do \
			$(CC) $(CFLAGS) -c "$$$$file" -o $(BUILD_DIR)/$(1)/frontend_$$$$(basename "$$$$file" .c).o; \
		done; \
	fi
	
	# 编译测试模块
	@if ls $(SRC_DIR)/$(1)/test/*.c >/dev/null 2>&1; then \
		echo "  编译 $(1) 测试模块..."; \
		for file in $(SRC_DIR)/$(1)/test/*.c; do \
			$(CC) $(CFLAGS) -c "$$$$file" -o $(BUILD_DIR)/$(1)/test_$$$$(basename "$$$$file" .c).o; \
		done; \
	fi
	
	# 链接演示程序
	@if ls $(BUILD_DIR)/$(1)/core_*.o $(BUILD_DIR)/$(1)/frontend_*.o >/dev/null 2>&1; then \
		echo "  链接 $(1) 演示程序..."; \
		$(CC) $(BUILD_DIR)/$(1)/core_*.o $(BUILD_DIR)/$(1)/frontend_*.o $(LDFLAGS) -lpthread -o $(BIN_DIR)/$(1)/demo; \
	fi
	
	# 链接测试程序
	@if ls $(BUILD_DIR)/$(1)/core_*.o $(BUILD_DIR)/$(1)/test_*.o >/dev/null 2>&1; then \
		echo "  链接 $(1) 测试程序..."; \
		$(CC) $(BUILD_DIR)/$(1)/core_*.o $(BUILD_DIR)/$(1)/test_*.o $(LDFLAGS) -o $(BIN_DIR)/$(1)/test; \
	fi
	
	@echo "  $(1) 构建完成"

# 运行特定实验的演示
$(1)-demo: $(1)
	@echo "运行 $(1) 演示程序..."
	@echo "========================================"
	@./$(BIN_DIR)/$(1)/demo

# 运行特定实验的测试
$(1)-test: $(1)
	@echo "运行 $(1) 测试程序..."
	@echo "========================================"
	@./$(BIN_DIR)/$(1)/test
	@if [ $$$$? -eq 0 ]; then \
		echo "✓ $(1) 测试通过！"; \
	else \
		echo "✗ $(1) 测试失败！"; \
	fi

# 清理特定实验
$(1)-clean:
	@echo "清理 $(1) 编译文件..."
	@rm -rf $(BUILD_DIR)/$(1)
	@rm -rf $(BIN_DIR)/$(1)

endef

# 为每个实验生成构建规则
$(foreach exp,$(EXPERIMENTS),$(eval $(call EXPERIMENT_TEMPLATE,$(exp))))

# 通用目标
experiments: $(EXPERIMENTS)

# 运行所有实验的演示（交互选择）
demo:
	@echo "可用的演示程序:"
	@i=1; for exp in $(EXPERIMENTS); do \
		echo "  $$i. $$exp"; \
		i=$$((i+1)); \
	done
	@echo "  0. 退出"
	@echo -n "请选择要运行的演示 (0-$(words $(EXPERIMENTS))): "; \
	read choice; \
	if [ "$$choice" = "0" ]; then \
		echo "退出"; \
	else \
		exp_name=$$(echo "$(EXPERIMENTS)" | cut -d' ' -f$$choice); \
		if [ -n "$$exp_name" ] && [ -f "$(BIN_DIR)/$$exp_name/demo" ]; then \
			echo "启动 $$exp_name 演示程序..."; \
			./$(BIN_DIR)/$$exp_name/demo; \
		else \
			echo "无效选择或程序不存在"; \
		fi \
	fi

# 运行所有测试
test: $(EXPERIMENTS)
	@echo "运行所有实验测试..."
	@echo "========================================"
	@failed=0; total=0; \
	for exp in $(EXPERIMENTS); do \
		if [ -f "$(BIN_DIR)/$$exp/test" ]; then \
			echo "测试 $$exp..."; \
			total=$$((total+1)); \
			if ./$(BIN_DIR)/$$exp/test > /dev/null 2>&1; then \
				echo "✓ $$exp 测试通过"; \
			else \
				echo "✗ $$exp 测试失败"; \
				failed=$$((failed+1)); \
			fi \
		fi \
	done; \
	echo "========================================"; \
	echo "测试结果: $$((total-failed))/$$total 通过"; \
	if [ $$failed -eq 0 ]; then \
		echo "🎉 所有测试通过！"; \
	else \
		echo "⚠️  $$failed 个测试失败"; \
	fi

# 清理所有编译文件
clean:
	@echo "清理所有编译文件..."
	@rm -rf $(BUILD_DIR) $(BIN_DIR)
	@echo "清理完成"

# 安装到系统目录（可选）
install: all
	@echo "安装程序到 /usr/local/bin/..."
	@for exp in $(EXPERIMENTS); do \
		if [ -f "$(BIN_DIR)/$$exp/demo" ]; then \
			sudo cp "$(BIN_DIR)/$$exp/demo" "/usr/local/bin/$${exp}_demo"; \
		fi; \
		if [ -f "$(BIN_DIR)/$$exp/test" ]; then \
			sudo cp "$(BIN_DIR)/$$exp/test" "/usr/local/bin/$${exp}_test"; \
		fi; \
	done
	@echo "安装完成"

# 卸载
uninstall:
	@echo "卸载程序..."
	@for exp in $(EXPERIMENTS); do \
		sudo rm -f "/usr/local/bin/$${exp}_demo"; \
		sudo rm -f "/usr/local/bin/$${exp}_test"; \
	done
	@echo "卸载完成"

# 显示详细的帮助信息
help:
	@echo "计算机网络实验项目构建系统"
	@echo "============================="
	@echo ""
	@echo "通用命令:"
	@echo "  make              - 构建所有实验"
	@echo "  make list         - 列出所有可用实验"
	@echo "  make experiments  - 构建所有实验"
	@echo "  make demo         - 交互式选择并运行演示"
	@echo "  make test         - 运行所有实验的测试"
	@echo "  make clean        - 清理所有编译文件"
	@echo "  make install      - 安装到系统目录"
	@echo "  make uninstall    - 从系统目录卸载"
	@echo "  make help         - 显示此帮助信息"
	@echo ""
	@echo "特定实验命令 (以sliding_window_protocol为例):"
	@echo "  make sliding_window_protocol       - 构建该实验"
	@echo "  make sliding_window_protocol-demo  - 运行该实验演示"
	@echo "  make sliding_window_protocol-test  - 运行该实验测试"
	@echo "  make sliding_window_protocol-clean - 清理该实验编译文件"
	@echo ""
	@echo "当前可用实验:"
	@for exp in $(EXPERIMENTS); do \
		echo "  - $$exp"; \
	done
	@echo ""
	@echo "项目结构:"
	@echo "  src/[实验名]/core/     - 核心实现"
	@echo "  src/[实验名]/frontend/ - 用户界面"  
	@echo "  src/[实验名]/test/     - 测试用例"
	@echo "  build/[实验名]/        - 编译输出"
	@echo "  bin/[实验名]/          - 可执行文件"

# 显示项目信息
info:
	@echo "计算机网络实验项目信息"
	@echo "====================="
	@echo "项目名称: 计算机网络课程实验项目"
	@echo "编译器:   $(CC)"
	@echo "编译选项: $(CFLAGS)"
	@echo "链接选项: $(LDFLAGS)"
	@echo ""
	@echo "发现的实验:"
	@for exp in $(EXPERIMENTS); do \
		echo "  - $$exp"; \
		echo "    源码: $(SRC_DIR)/$$exp"; \
		echo "    构建: $(BUILD_DIR)/$$exp"; \
		echo "    程序: $(BIN_DIR)/$$exp"; \
	done

# 调试信息
debug:
	@echo "调试信息:"
	@echo "EXPERIMENTS = $(EXPERIMENTS)"
	@echo "SRC_DIR = $(SRC_DIR)"  
	@echo "BUILD_DIR = $(BUILD_DIR)"
	@echo "BIN_DIR = $(BIN_DIR)"
	@echo ""
	@echo "目录结构检查:"
	@for exp in $(EXPERIMENTS); do \
		echo "实验: $$exp"; \
		[ -d "$(SRC_DIR)/$$exp/core" ] && echo "  ✓ core目录存在" || echo "  ✗ core目录不存在"; \
		[ -d "$(SRC_DIR)/$$exp/frontend" ] && echo "  ✓ frontend目录存在" || echo "  ✗ frontend目录不存在"; \
		[ -d "$(SRC_DIR)/$$exp/test" ] && echo "  ✓ test目录存在" || echo "  ✗ test目录不存在"; \
	done