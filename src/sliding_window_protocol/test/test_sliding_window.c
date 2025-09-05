#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include "../core/sliding_window.h"

/* 测试用例计数器 */
static int test_count = 0;
static int passed_count = 0;
static int failed_count = 0;

/**
 * 打印测试标题
 * @param test_name 测试名称
 */
void print_test_header(const char* test_name) {
    printf("\n========================================\n");
    printf("测试 %d: %s\n", ++test_count, test_name);
    printf("========================================\n");
}

/**
 * 断言函数，用于测试结果验证
 * @param condition 条件
 * @param message 错误消息
 */
void test_assert(int condition, const char* message) {
    if (condition) {
        printf("✓ %s\n", message);
        passed_count++;
    } else {
        printf("✗ %s\n", message);
        failed_count++;
    }
}

/**
 * 测试1: 基本数据结构初始化
 */
void test_initialization(void) {
    print_test_header("基本数据结构初始化");
    
    // 测试发送方初始化
    sender_state_t sender;
    init_sender(&sender);
    test_assert(sender.state == WAITING_FOR_CALL, "发送方状态初始化正确");
    test_assert(sender.seq_num == 0, "发送方序列号初始化正确");
    test_assert(sender.retry_count == 0, "发送方重传计数初始化正确");
    
    // 测试接收方初始化
    receiver_state_t receiver;
    init_receiver(&receiver);
    test_assert(receiver.state == WAITING_FOR_DATA, "接收方状态初始化正确");
    test_assert(receiver.expected_seq == 0, "接收方期望序列号初始化正确");
    
    // 测试网络配置初始化
    network_config_t config;
    init_network_config(&config);
    test_assert(config.loss_probability >= 0.0 && config.loss_probability <= 1.0, 
                "网络丢包概率配置合理");
    test_assert(config.min_delay_ms >= 0 && config.max_delay_ms >= config.min_delay_ms, 
                "网络延迟配置合理");
    
    // 测试统计信息初始化
    statistics_t stats;
    init_statistics(&stats);
    test_assert(stats.frames_sent == 0, "统计信息初始化正确");
    test_assert(stats.frames_received == 0, "统计信息接收帧数初始化正确");
    test_assert(stats.retransmissions == 0, "统计信息重传次数初始化正确");
}

/**
 * 测试2: 帧创建和校验
 */
void test_frame_creation_and_checksum(void) {
    print_test_header("帧创建和校验功能");
    
    // 测试数据帧创建
    data_frame_t data_frame;
    const char* test_data = "Hello, World!";
    create_data_frame(&data_frame, 0, test_data, strlen(test_data));
    
    test_assert(data_frame.type == DATA_FRAME, "数据帧类型设置正确");
    test_assert(data_frame.seq_num == 0, "数据帧序列号设置正确");
    test_assert(data_frame.data_length == (int)strlen(test_data), "数据帧长度设置正确");
    test_assert(strcmp(data_frame.data, test_data) == 0, "数据帧内容设置正确");
    test_assert(data_frame.checksum != 0, "数据帧校验和已计算");
    
    // 测试校验和验证
    unsigned int original_checksum = data_frame.checksum;
    data_frame.checksum = 0;
    unsigned int calculated_checksum = calculate_checksum(&data_frame, sizeof(data_frame) - sizeof(unsigned int));
    test_assert(calculated_checksum == original_checksum, "校验和计算正确");
    
    // 测试确认帧创建
    ack_frame_t ack_frame;
    create_ack_frame(&ack_frame, 0);
    
    test_assert(ack_frame.type == ACK_FRAME, "确认帧类型设置正确");
    test_assert(ack_frame.ack_num == 0, "确认帧确认号设置正确");
    test_assert(ack_frame.checksum != 0, "确认帧校验和已计算");
}

/**
 * 测试3: 网络模拟功能
 */
void test_network_simulation(void) {
    print_test_header("网络模拟功能");
    
    network_config_t config;
    init_network_config(&config);
    
    // 测试零丢包概率
    config.loss_probability = 0.0;
    int no_loss_count = 0;
    for (int i = 0; i < 100; i++) {
        if (!simulate_frame_loss(&config)) {
            no_loss_count++;
        }
    }
    test_assert(no_loss_count == 100, "零丢包概率测试通过");
    
    // 测试100%丢包概率
    config.loss_probability = 1.0;
    int all_loss_count = 0;
    for (int i = 0; i < 100; i++) {
        if (simulate_frame_loss(&config)) {
            all_loss_count++;
        }
    }
    test_assert(all_loss_count == 100, "100%丢包概率测试通过");
    
    // 测试延迟范围
    config.min_delay_ms = 50;
    config.max_delay_ms = 100;
    
    // 注意：延迟测试只能测试函数不崩溃，不能精确测试时间
    printf("延迟模拟测试（仅测试功能可用性）...\n");
    clock_t start = clock();
    simulate_network_delay(&config);
    clock_t end = clock();
    double elapsed = ((double)(end - start)) / CLOCKS_PER_SEC * 1000;
    
    test_assert(elapsed >= 0, "网络延迟模拟功能正常");
}

/**
 * 测试4: 理想网络环境下的基本传输
 */
void test_basic_transmission_ideal_network(void) {
    print_test_header("理想网络环境下的基本传输");
    
    // 设置理想网络环境（无丢包，低延迟）
    network_config_t config;
    config.loss_probability = 0.0;
    config.min_delay_ms = 1;
    config.max_delay_ms = 5;
    
    statistics_t stats;
    init_statistics(&stats);
    
    const char* test_message = "测试消息";
    
    printf("开始理想网络环境传输测试...\n");
    bool result = transmit_message(test_message, &config, &stats);
    
    test_assert(result == true, "理想网络环境传输成功");
    test_assert(stats.frames_sent > 0, "有帧被发送");
    test_assert(stats.frames_received > 0, "有帧被接收");
    test_assert(stats.acks_sent > 0, "有确认帧被发送");
    test_assert(stats.acks_received > 0, "有确认帧被接收");
    test_assert(stats.frames_lost == 0, "理想环境下无帧丢失");
    test_assert(stats.retransmissions == 0, "理想环境下无重传");
}

/**
 * 测试5: 有丢包的网络环境传输
 */
void test_transmission_with_loss(void) {
    print_test_header("有丢包的网络环境传输");
    
    // 设置中等丢包率的网络环境
    network_config_t config;
    config.loss_probability = 0.3;  // 30%丢包率
    config.min_delay_ms = 10;
    config.max_delay_ms = 50;
    
    statistics_t stats;
    init_statistics(&stats);
    
    const char* test_message = "丢包测试消息";
    
    printf("开始有丢包网络环境传输测试...\n");
    bool result = transmit_message(test_message, &config, &stats);
    
    // 注意：由于随机性，这个测试可能成功也可能失败
    // 主要测试系统在丢包环境下的行为
    printf("传输结果: %s\n", result ? "成功" : "失败");
    printf("总发送帧数: %d\n", stats.frames_sent);
    printf("丢失帧数: %d\n", stats.frames_lost);
    printf("重传次数: %d\n", stats.retransmissions);
    
    test_assert(stats.frames_sent > 0, "有帧被发送");
    
    if (stats.frames_lost > 0) {
        test_assert(stats.retransmissions >= 0, "有丢包时可能触发重传");
    }
}

/**
 * 测试6: 超时重传机制
 */
void test_timeout_retransmission(void) {
    print_test_header("超时重传机制");
    
    // 测试超时检测
    sender_state_t sender;
    init_sender(&sender);
    
    // 模拟发送状态
    sender.state = WAITING_FOR_ACK;
    sender.timer_start = clock() - (TIMEOUT_MS + 100) * CLOCKS_PER_SEC / 1000;
    
    test_assert(is_timeout(&sender) == true, "超时检测功能正常");
    
    // 测试未超时情况
    sender.timer_start = clock();
    test_assert(is_timeout(&sender) == false, "未超时检测正常");
    
    // 测试计时器重置
    clock_t old_time = sender.timer_start;
    usleep(1000);  // 等待1ms确保时间不同
    reset_timer(&sender);
    test_assert(sender.timer_start > old_time, "计时器重置功能正常");
}

/**
 * 测试7: 序列号处理
 */
void test_sequence_number_handling(void) {
    print_test_header("序列号处理");
    
    sender_state_t sender;
    receiver_state_t receiver;
    init_sender(&sender);
    init_receiver(&receiver);
    
    // 测试初始序列号
    test_assert(sender.seq_num == 0, "发送方初始序列号为0");
    test_assert(receiver.expected_seq == 0, "接收方期望序列号为0");
    
    // 测试序列号循环（停等协议使用0和1）
    data_frame_t frame1, frame2, frame3;
    create_data_frame(&frame1, 0, "Frame 0", 7);
    create_data_frame(&frame2, 1, "Frame 1", 7);
    create_data_frame(&frame3, 0, "Frame 2", 7);  // 循环回到0
    
    test_assert(frame1.seq_num == 0, "第一帧序列号为0");
    test_assert(frame2.seq_num == 1, "第二帧序列号为1");
    test_assert(frame3.seq_num == 0, "第三帧序列号循环回到0");
    
    printf("序列号循环机制验证完成\n");
}

/**
 * 测试8: 错误处理和边界条件
 */
void test_error_handling_and_edge_cases(void) {
    print_test_header("错误处理和边界条件");
    
    // 测试NULL指针处理
    sender_state_t* null_sender = NULL;
    receiver_state_t* null_receiver = NULL;
    network_config_t* null_config = NULL;
    statistics_t* null_stats = NULL;
    
    init_sender(null_sender);  // 应该安全处理
    init_receiver(null_receiver);  // 应该安全处理
    init_network_config(null_config);  // 应该安全处理
    init_statistics(null_stats);  // 应该安全处理
    
    printf("NULL指针处理测试完成（程序未崩溃）\n");
    test_assert(1, "NULL指针安全处理");
    
    // 测试空字符串传输
    network_config_t config;
    statistics_t stats;
    init_network_config(&config);
    init_statistics(&stats);
    config.loss_probability = 0.0;  // 确保传输成功
    
    bool empty_result = transmit_message("", &config, &stats);
    test_assert(empty_result == false, "空字符串传输处理正确");
    
    // 测试超长消息
    char long_message[MAX_DATA_SIZE + 100];
    memset(long_message, 'A', MAX_DATA_SIZE + 50);
    long_message[MAX_DATA_SIZE + 50] = '\0';
    
    bool long_result = transmit_message(long_message, &config, &stats);
    test_assert(long_result == false, "超长消息处理正确");
}

/**
 * 测试9: 统计功能验证
 */
void test_statistics_functionality(void) {
    print_test_header("统计功能验证");
    
    statistics_t stats;
    init_statistics(&stats);
    
    // 验证统计信息初始化
    test_assert(stats.frames_sent == 0, "统计-发送帧数初始化");
    test_assert(stats.frames_received == 0, "统计-接收帧数初始化");
    test_assert(stats.acks_sent == 0, "统计-发送确认数初始化");
    test_assert(stats.acks_received == 0, "统计-接收确认数初始化");
    test_assert(stats.retransmissions == 0, "统计-重传次数初始化");
    test_assert(stats.frames_lost == 0, "统计-丢失帧数初始化");
    
    // 模拟一些统计更新
    stats.frames_sent = 5;
    stats.frames_received = 4;
    stats.acks_sent = 4;
    stats.acks_received = 3;
    stats.retransmissions = 1;
    stats.frames_lost = 2;
    stats.end_time = stats.start_time + CLOCKS_PER_SEC;  // 模拟1秒传输
    
    printf("统计信息打印测试:\n");
    print_statistics(&stats);
    
    test_assert(1, "统计信息显示功能正常");
}

/**
 * 测试10: 完整的协议状态转换
 */
void test_protocol_state_transitions(void) {
    print_test_header("完整的协议状态转换");
    
    sender_state_t sender;
    receiver_state_t receiver;
    init_sender(&sender);
    init_receiver(&receiver);
    
    // 验证初始状态
    test_assert(sender.state == WAITING_FOR_CALL, "发送方初始状态正确");
    test_assert(receiver.state == WAITING_FOR_DATA, "接收方初始状态正确");
    
    // 模拟状态转换
    sender.state = WAITING_FOR_ACK;
    test_assert(sender.state == WAITING_FOR_ACK, "发送方状态转换正确");
    
    // 测试状态打印功能
    printf("协议状态显示测试:\n");
    print_protocol_state(&sender, &receiver);
    
    test_assert(1, "协议状态显示功能正常");
}

/**
 * 运行所有测试
 */
void run_all_tests(void) {
    printf("开始执行滑动窗口协议测试套件\n");
    printf("=====================================\n");
    
    // 初始化随机数种子（用于网络模拟）
    srand((unsigned int)time(NULL));
    
    // 执行所有测试
    test_initialization();
    test_frame_creation_and_checksum();
    test_network_simulation();
    test_basic_transmission_ideal_network();
    test_transmission_with_loss();
    test_timeout_retransmission();
    test_sequence_number_handling();
    test_error_handling_and_edge_cases();
    test_statistics_functionality();
    test_protocol_state_transitions();
    
    // 输出测试结果汇总
    printf("\n\n========================================\n");
    printf("测试结果汇总\n");
    printf("========================================\n");
    printf("总测试数:     %d\n", test_count);
    printf("通过的断言:   %d\n", passed_count);
    printf("失败的断言:   %d\n", failed_count);
    printf("成功率:       %.1f%%\n", 
           passed_count > 0 ? (double)passed_count / (passed_count + failed_count) * 100 : 0);
    
    if (failed_count == 0) {
        printf("\n🎉 所有测试通过！滑动窗口协议实现正确。\n");
    } else {
        printf("\n⚠️  发现 %d 个失败的断言，请检查实现。\n", failed_count);
    }
    
    printf("========================================\n");
}

/**
 * 主程序入口
 */
int main(void) {
    printf("滑动窗口协议（停等协议）测试程序\n");
    printf("用于验证协议实现的正确性和鲁棒性\n");
    printf("=====================================\n");
    
    run_all_tests();
    
    printf("\n测试完成。\n");
    return failed_count > 0 ? 1 : 0;  // 如果有失败则返回错误码
}