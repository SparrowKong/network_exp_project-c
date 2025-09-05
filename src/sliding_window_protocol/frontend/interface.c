#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../core/sliding_window.h"

/* 界面显示常量 */
#define LINE_LENGTH 60
#define TITLE_WIDTH 50

/**
 * 打印分隔线
 */
void print_separator(void) {
    for (int i = 0; i < LINE_LENGTH; i++) {
        printf("=");
    }
    printf("\n");
}

/**
 * 打印标题
 * @param title 标题内容
 */
void print_title(const char* title) {
    print_separator();
    int padding = (LINE_LENGTH - strlen(title)) / 2;
    for (int i = 0; i < padding; i++) printf(" ");
    printf("%s\n", title);
    print_separator();
}

/**
 * 清除输入缓冲区
 */
void clear_input_buffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

/**
 * 安全的整数输入函数
 * @param prompt 提示信息
 * @param min_val 最小值
 * @param max_val 最大值
 * @return 输入的整数
 */
int safe_int_input(const char* prompt, int min_val, int max_val) {
    int value;
    int result;
    
    while (1) {
        printf("%s", prompt);
        result = scanf("%d", &value);
        
        if (result == 1 && value >= min_val && value <= max_val) {
            clear_input_buffer();
            return value;
        } else {
            printf("输入无效！请输入 %d 到 %d 之间的整数。\n", min_val, max_val);
            clear_input_buffer();
        }
    }
}

/**
 * 安全的浮点数输入函数
 * @param prompt 提示信息
 * @param min_val 最小值
 * @param max_val 最大值
 * @return 输入的浮点数
 */
double safe_double_input(const char* prompt, double min_val, double max_val) {
    double value;
    int result;
    
    while (1) {
        printf("%s", prompt);
        result = scanf("%lf", &value);
        
        if (result == 1 && value >= min_val && value <= max_val) {
            clear_input_buffer();
            return value;
        } else {
            printf("输入无效！请输入 %.2f 到 %.2f 之间的数值。\n", min_val, max_val);
            clear_input_buffer();
        }
    }
}

/**
 * 安全的字符串输入函数
 * @param prompt 提示信息
 * @param buffer 输入缓冲区
 * @param max_length 最大长度
 */
void safe_string_input(const char* prompt, char* buffer, int max_length) {
    printf("%s", prompt);
    
    if (fgets(buffer, max_length, stdin) != NULL) {
        // 移除换行符
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
        }
    } else {
        buffer[0] = '\0';  // 输入失败时设为空字符串
    }
}

/**
 * 显示主菜单
 */
void show_main_menu(void) {
    print_title("滑动窗口协议（停等协议）模拟器");
    printf("\n请选择操作：\n");
    printf("1. 开始消息传输实验\n");
    printf("2. 自定义网络环境设置\n");
    printf("3. 运行预设测试场景\n");
    printf("4. 查看协议说明\n");
    printf("5. 退出程序\n");
    printf("\n");
}

/**
 * 显示网络配置菜单
 * @param config 当前网络配置
 */
void show_network_config_menu(network_config_t* config) {
    print_title("网络环境设置");
    
    printf("\n当前网络环境配置：\n");
    printf("丢包概率：     %.1f%%\n", config->loss_probability * 100);
    printf("最小延迟：     %d 毫秒\n", config->min_delay_ms);
    printf("最大延迟：     %d 毫秒\n", config->max_delay_ms);
    printf("\n请选择要修改的参数：\n");
    printf("1. 修改丢包概率\n");
    printf("2. 修改网络延迟范围\n");
    printf("3. 恢复默认设置\n");
    printf("4. 返回主菜单\n");
    printf("\n");
}

/**
 * 修改网络配置
 * @param config 网络配置
 */
void modify_network_config(network_config_t* config) {
    int choice;
    
    while (1) {
        show_network_config_menu(config);
        choice = safe_int_input("请输入选项 (1-4): ", 1, 4);
        
        switch (choice) {
            case 1: {
                double loss_rate = safe_double_input(
                    "请输入丢包概率 (0.0-1.0): ", 0.0, 1.0);
                config->loss_probability = loss_rate;
                printf("丢包概率已设置为 %.1f%%\n", loss_rate * 100);
                break;
            }
            case 2: {
                int min_delay = safe_int_input(
                    "请输入最小延迟 (毫秒, 1-1000): ", 1, 1000);
                int max_delay = safe_int_input(
                    "请输入最大延迟 (毫秒, 必须 >= 最小延迟): ", min_delay, 2000);
                config->min_delay_ms = min_delay;
                config->max_delay_ms = max_delay;
                printf("延迟范围已设置为 %d-%d 毫秒\n", min_delay, max_delay);
                break;
            }
            case 3: {
                init_network_config(config);
                printf("已恢复默认网络设置\n");
                break;
            }
            case 4:
                return;
        }
        
        printf("\n按 Enter 键继续...");
        clear_input_buffer();
        getchar();
    }
}

/**
 * 执行消息传输实验
 * @param config 网络配置
 */
void run_transmission_experiment(network_config_t* config) {
    print_title("消息传输实验");
    
    char message[MAX_DATA_SIZE];
    printf("当前网络环境：\n");
    printf("- 丢包概率: %.1f%%\n", config->loss_probability * 100);
    printf("- 延迟范围: %d-%d 毫秒\n", config->min_delay_ms, config->max_delay_ms);
    printf("\n");
    
    // 获取用户输入的消息
    safe_string_input("请输入要传输的消息: ", message, MAX_DATA_SIZE - 1);
    
    if (strlen(message) == 0) {
        printf("消息不能为空，使用默认消息\n");
        strcpy(message, "Hello, 这是一个停等协议的测试消息！");
    }
    
    printf("\n准备开始传输，按 Enter 键开始...");
    getchar();
    
    // 初始化统计信息
    statistics_t stats;
    init_statistics(&stats);
    
    // 执行传输
    printf("\n");
    print_title("传输过程");
    
    bool success = transmit_message(message, config, &stats);
    
    // 显示结果
    printf("\n");
    if (success) {
        print_title("传输成功！");
        printf("消息 \"%s\" 已成功传输\n", message);
    } else {
        print_title("传输失败！");
        printf("消息 \"%s\" 传输失败\n", message);
    }
    
    // 显示统计信息
    print_statistics(&stats);
    
    printf("\n按 Enter 键继续...");
    getchar();
}

/**
 * 运行预设测试场景
 */
void run_preset_scenarios(void) {
    print_title("预设测试场景");
    
    printf("请选择测试场景：\n");
    printf("1. 理想网络环境 (无丢包，低延迟)\n");
    printf("2. 一般网络环境 (轻微丢包，中等延迟)\n");
    printf("3. 恶劣网络环境 (高丢包率，高延迟)\n");
    printf("4. 返回主菜单\n");
    printf("\n");
    
    int choice = safe_int_input("请选择场景 (1-4): ", 1, 4);
    
    network_config_t config;
    const char* test_message = "停等协议测试消息 - 计算机网络实验";
    
    switch (choice) {
        case 1:
            // 理想网络
            config.loss_probability = 0.0;
            config.min_delay_ms = 10;
            config.max_delay_ms = 50;
            printf("\n=== 理想网络环境测试 ===\n");
            break;
            
        case 2:
            // 一般网络
            config.loss_probability = 0.1;
            config.min_delay_ms = 50;
            config.max_delay_ms = 150;
            printf("\n=== 一般网络环境测试 ===\n");
            break;
            
        case 3:
            // 恶劣网络
            config.loss_probability = 0.3;
            config.min_delay_ms = 200;
            config.max_delay_ms = 500;
            printf("\n=== 恶劣网络环境测试 ===\n");
            break;
            
        case 4:
            return;
    }
    
    printf("测试消息: \"%s\"\n", test_message);
    printf("按 Enter 键开始测试...");
    getchar();
    
    statistics_t stats;
    init_statistics(&stats);
    
    printf("\n");
    print_title("测试进行中");
    
    bool success = transmit_message(test_message, &config, &stats);
    
    printf("\n");
    if (success) {
        print_title("测试通过！");
    } else {
        print_title("测试失败！");
    }
    
    print_statistics(&stats);
    
    printf("\n按 Enter 键继续...");
    getchar();
}

/**
 * 显示协议说明
 */
void show_protocol_explanation(void) {
    print_title("停等协议说明");
    
    printf("\n什么是停等协议？\n");
    printf("停等协议是最简单的自动重传请求(ARQ)协议，也是窗口大小为1的\n");
    printf("滑动窗口协议。它的工作原理如下：\n\n");
    
    printf("工作流程：\n");
    printf("1. 发送方发送一个数据帧\n");
    printf("2. 启动计时器，等待接收方的确认帧(ACK)\n");
    printf("3. 如果在超时时间内收到正确的ACK，发送下一帧\n");
    printf("4. 如果超时或收到错误的ACK，重传当前帧\n");
    printf("5. 重复以上过程，直到所有数据传输完成\n\n");
    
    printf("协议特点：\n");
    printf("✓ 简单可靠：实现简单，能保证数据的可靠传输\n");
    printf("✓ 序列号：使用0和1两个序列号进行帧的标识\n");
    printf("✓ 超时重传：具有超时重传机制，应对网络丢包\n");
    printf("✓ 错误检测：使用校验和检测传输错误\n\n");
    
    printf("缺点：\n");
    printf("✗ 效率较低：每次只能发送一帧，信道利用率不高\n");
    printf("✗ 延迟敏感：网络延迟会显著影响传输效率\n\n");
    
    printf("本实验模拟的网络环境：\n");
    printf("• 随机丢包：模拟真实网络的丢包现象\n");
    printf("• 随机延迟：模拟网络传输延迟\n");
    printf("• 错误检测：模拟数据传输中的校验过程\n\n");
    
    printf("按 Enter 键返回主菜单...");
    getchar();
}

/**
 * 主程序入口
 */
int main(void) {
    // 初始化随机数种子
    srand((unsigned int)time(NULL));
    
    // 初始化网络配置
    network_config_t config;
    init_network_config(&config);
    
    int choice;
    
    printf("欢迎使用滑动窗口协议模拟器！\n");
    printf("这是一个用于学习计算机网络中停等协议的教学工具。\n\n");
    
    while (1) {
        show_main_menu();
        choice = safe_int_input("请输入选项 (1-5): ", 1, 5);
        
        switch (choice) {
            case 1:
                run_transmission_experiment(&config);
                break;
                
            case 2:
                modify_network_config(&config);
                break;
                
            case 3:
                run_preset_scenarios();
                break;
                
            case 4:
                show_protocol_explanation();
                break;
                
            case 5:
                print_title("感谢使用");
                printf("程序已退出。再见！\n");
                return 0;
                
            default:
                printf("无效选项，请重新选择。\n");
        }
        
        printf("\n");
    }
    
    return 0;
}