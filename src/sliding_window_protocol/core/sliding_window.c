#include "sliding_window.h"

/* 全局变量：用于模拟网络传输的缓冲区 */
static data_frame_t network_data_buffer;
static ack_frame_t network_ack_buffer;
static bool data_in_transit = false;
static bool ack_in_transit = false;

/* ========== 初始化函数 ========== */

/**
 * 初始化发送方状态
 * @param sender 发送方状态结构体指针
 */
void init_sender(sender_state_t* sender) {
    if (!sender) return;
    
    sender->state = WAITING_FOR_CALL;    // 初始状态：等待上层调用
    sender->seq_num = 0;                 // 初始序列号为0
    sender->retry_count = 0;             // 重传计数清零
    memset(&sender->current_frame, 0, sizeof(data_frame_t));
    
    printf("[发送方] 初始化完成 - 状态: 等待调用, 序列号: %d\n", sender->seq_num);
}

/**
 * 初始化接收方状态
 * @param receiver 接收方状态结构体指针
 */
void init_receiver(receiver_state_t* receiver) {
    if (!receiver) return;
    
    receiver->state = WAITING_FOR_DATA;  // 初始状态：等待数据帧
    receiver->expected_seq = 0;          // 期望的序列号为0
    
    printf("[接收方] 初始化完成 - 状态: 等待数据, 期望序列号: %d\n", receiver->expected_seq);
}

/**
 * 初始化统计信息
 * @param stats 统计信息结构体指针
 */
void init_statistics(statistics_t* stats) {
    if (!stats) return;
    
    memset(stats, 0, sizeof(statistics_t));
    stats->start_time = clock();
    
    printf("[统计] 统计信息初始化完成\n");
}

/**
 * 初始化网络配置（默认配置）
 * @param config 网络配置结构体指针
 */
void init_network_config(network_config_t* config) {
    if (!config) return;
    
    config->loss_probability = 0.1;      // 10%丢包率
    config->min_delay_ms = 50;           // 最小延迟50ms
    config->max_delay_ms = 200;          // 最大延迟200ms
    
    printf("[网络] 网络配置初始化 - 丢包率: %.1f%%, 延迟: %d-%d ms\n", 
           config->loss_probability * 100, config->min_delay_ms, config->max_delay_ms);
}

/* ========== 帧处理函数 ========== */

/**
 * 计算简单的校验和（用于错误检测）
 * @param data 数据指针
 * @param length 数据长度
 * @return 校验和
 */
unsigned int calculate_checksum(const void* data, size_t length) {
    const unsigned char* bytes = (const unsigned char*)data;
    unsigned int sum = 0;
    
    for (size_t i = 0; i < length; i++) {
        sum += bytes[i];
    }
    
    return sum;
}

/**
 * 验证校验和
 * @param data 数据指针
 * @param length 数据长度
 * @param expected_checksum 期望的校验和
 * @return 校验是否通过
 */
bool verify_checksum(const void* data, size_t length, unsigned int expected_checksum) {
    return calculate_checksum(data, length) == expected_checksum;
}

/**
 * 创建数据帧
 * @param frame 数据帧结构体指针
 * @param seq_num 序列号
 * @param data 数据内容
 * @param length 数据长度
 */
void create_data_frame(data_frame_t* frame, int seq_num, const char* data, int length) {
    if (!frame || !data) return;
    
    frame->type = DATA_FRAME;
    frame->seq_num = seq_num;
    frame->data_length = length;
    
    // 复制数据内容
    memcpy(frame->data, data, length);
    frame->data[length] = '\0';  // 确保字符串结束
    
    // 计算校验和（不包括校验和字段本身）
    frame->checksum = calculate_checksum(frame, sizeof(data_frame_t) - sizeof(unsigned int));
    
    printf("[帧创建] 数据帧 - 序列号: %d, 长度: %d, 内容: \"%.20s%s\"\n", 
           seq_num, length, data, length > 20 ? "..." : "");
}

/**
 * 创建确认帧
 * @param frame 确认帧结构体指针
 * @param ack_num 确认号
 */
void create_ack_frame(ack_frame_t* frame, int ack_num) {
    if (!frame) return;
    
    frame->type = ACK_FRAME;
    frame->ack_num = ack_num;
    
    // 计算校验和
    frame->checksum = calculate_checksum(frame, sizeof(ack_frame_t) - sizeof(unsigned int));
    
    printf("[帧创建] 确认帧 - 确认号: %d\n", ack_num);
}

/* ========== 网络模拟函数 ========== */

/**
 * 模拟帧丢失
 * @param config 网络配置
 * @return 是否丢失帧
 */
bool simulate_frame_loss(network_config_t* config) {
    if (!config) return false;
    
    double random_value = (double)rand() / RAND_MAX;
    bool lost = random_value < config->loss_probability;
    
    if (lost) {
        printf("[网络模拟] 帧丢失! (概率: %.1f%%, 随机值: %.3f)\n", 
               config->loss_probability * 100, random_value);
    }
    
    return lost;
}

/**
 * 模拟网络延迟
 * @param config 网络配置
 */
void simulate_network_delay(network_config_t* config) {
    if (!config) return;
    
    int delay = config->min_delay_ms + 
                (rand() % (config->max_delay_ms - config->min_delay_ms + 1));
    
    printf("[网络模拟] 延迟 %d ms\n", delay);
    
    // 简单的延迟模拟（在实际应用中可能需要更复杂的实现）
    usleep(delay * 1000);  // 转换为微秒
}

/* ========== 协议核心函数 ========== */

/**
 * 发送数据帧
 * @param sender 发送方状态
 * @param frame 要发送的数据帧
 * @param config 网络配置
 * @param stats 统计信息
 * @return 发送是否成功
 */
bool send_data_frame(sender_state_t* sender, const data_frame_t* frame, 
                     network_config_t* config, statistics_t* stats) {
    if (!sender || !frame || !config || !stats) return false;
    
    printf("\n[发送方] 准备发送数据帧 (序列号: %d)\n", frame->seq_num);
    stats->frames_sent++;
    
    // 模拟网络延迟
    simulate_network_delay(config);
    
    // 模拟帧丢失
    if (simulate_frame_loss(config)) {
        stats->frames_lost++;
        printf("[发送方] 数据帧丢失，未到达接收方\n");
        return false;
    }
    
    // 将帧放入网络缓冲区
    memcpy(&network_data_buffer, frame, sizeof(data_frame_t));
    data_in_transit = true;
    
    print_frame_info(frame, "发送");
    return true;
}

/**
 * 接收数据帧
 * @param receiver 接收方状态
 * @param frame 接收到的数据帧
 * @param config 网络配置
 * @param stats 统计信息
 * @return 接收是否成功
 */
bool receive_data_frame(receiver_state_t* receiver, data_frame_t* frame, 
                        network_config_t* config, statistics_t* stats) {
    if (!receiver || !frame || !config || !stats) return false;
    
    // 检查是否有数据帧在传输中
    if (!data_in_transit) {
        return false;  // 没有数据帧可接收
    }
    
    // 从网络缓冲区取出帧
    memcpy(frame, &network_data_buffer, sizeof(data_frame_t));
    data_in_transit = false;
    
    stats->frames_received++;
    print_frame_info(frame, "接收");
    
    // 验证校验和
    unsigned int expected_checksum = frame->checksum;
    frame->checksum = 0;  // 临时清零以计算校验和
    unsigned int calculated_checksum = calculate_checksum(frame, sizeof(data_frame_t) - sizeof(unsigned int));
    frame->checksum = expected_checksum;  // 恢复
    
    if (calculated_checksum != expected_checksum) {
        printf("[接收方] 校验和错误! 期望: %u, 计算: %u\n", 
               expected_checksum, calculated_checksum);
        return false;
    }
    
    printf("[接收方] 数据帧校验通过\n");
    return true;
}

/**
 * 发送确认帧
 * @param receiver 接收方状态
 * @param ack 确认帧
 * @param config 网络配置
 * @param stats 统计信息
 * @return 发送是否成功
 */
bool send_ack_frame(receiver_state_t* receiver, const ack_frame_t* ack, 
                    network_config_t* config, statistics_t* stats) {
    if (!receiver || !ack || !config || !stats) return false;
    
    printf("\n[接收方] 准备发送确认帧 (确认号: %d)\n", ack->ack_num);
    stats->acks_sent++;
    
    // 模拟网络延迟
    simulate_network_delay(config);
    
    // 模拟帧丢失
    if (simulate_frame_loss(config)) {
        stats->frames_lost++;
        printf("[接收方] 确认帧丢失，未到达发送方\n");
        return false;
    }
    
    // 将确认帧放入网络缓冲区
    memcpy(&network_ack_buffer, ack, sizeof(ack_frame_t));
    ack_in_transit = true;
    
    print_ack_info(ack, "发送");
    return true;
}

/**
 * 接收确认帧
 * @param sender 发送方状态
 * @param ack 接收到的确认帧
 * @param stats 统计信息
 * @return 接收是否成功
 */
bool receive_ack_frame(sender_state_t* sender, const ack_frame_t* ack, 
                       statistics_t* stats) {
    if (!sender || !ack || !stats) return false;
    
    // 检查是否有确认帧在传输中
    if (!ack_in_transit) {
        return false;  // 没有确认帧可接收
    }
    
    // 从网络缓冲区取出确认帧
    ack_frame_t received_ack;
    memcpy(&received_ack, &network_ack_buffer, sizeof(ack_frame_t));
    ack_in_transit = false;
    
    stats->acks_received++;
    print_ack_info(&received_ack, "接收");
    
    // 验证校验和
    unsigned int expected_checksum = received_ack.checksum;
    received_ack.checksum = 0;
    unsigned int calculated_checksum = calculate_checksum(&received_ack, sizeof(ack_frame_t) - sizeof(unsigned int));
    
    if (calculated_checksum != expected_checksum) {
        printf("[发送方] 确认帧校验和错误!\n");
        return false;
    }
    
    // 检查确认号是否正确
    if (received_ack.ack_num == sender->seq_num) {
        printf("[发送方] 接收到正确的确认帧 (确认号: %d)\n", received_ack.ack_num);
        return true;
    } else {
        printf("[发送方] 接收到错误的确认号: %d (期望: %d)\n", 
               received_ack.ack_num, sender->seq_num);
        return false;
    }
}

/* ========== 超时处理函数 ========== */

/**
 * 检查是否超时
 * @param sender 发送方状态
 * @return 是否超时
 */
bool is_timeout(sender_state_t* sender) {
    if (!sender || sender->state != WAITING_FOR_ACK) return false;
    
    clock_t current_time = clock();
    double elapsed_ms = ((double)(current_time - sender->timer_start)) / CLOCKS_PER_SEC * 1000;
    
    return elapsed_ms > TIMEOUT_MS;
}

/**
 * 处理超时事件
 * @param sender 发送方状态
 * @param config 网络配置
 * @param stats 统计信息
 */
void handle_timeout(sender_state_t* sender, network_config_t* config, 
                    statistics_t* stats) {
    if (!sender || !config || !stats) return;
    
    printf("\n[超时处理] 发生超时! 准备重传...\n");
    
    sender->retry_count++;
    stats->retransmissions++;
    
    if (sender->retry_count >= MAX_RETRIES) {
        printf("[超时处理] 达到最大重传次数 (%d)，传输失败\n", MAX_RETRIES);
        sender->state = WAITING_FOR_CALL;
        return;
    }
    
    printf("[超时处理] 第 %d 次重传 (最大: %d)\n", sender->retry_count, MAX_RETRIES);
    
    // 重新发送当前帧
    send_data_frame(sender, &sender->current_frame, config, stats);
    reset_timer(sender);
}

/**
 * 重置计时器
 * @param sender 发送方状态
 */
void reset_timer(sender_state_t* sender) {
    if (!sender) return;
    
    sender->timer_start = clock();
    printf("[计时器] 重置计时器\n");
}

/* ========== 主要传输函数 ========== */

/**
 * 传输消息的主函数（停等协议实现）
 * @param message 要传输的消息
 * @param config 网络配置
 * @param stats 统计信息
 * @return 传输是否成功
 */
bool transmit_message(const char* message, network_config_t* config, 
                      statistics_t* stats) {
    if (!message || !config || !stats) return false;
    
    printf("\n========== 开始传输消息 ==========\n");
    printf("消息内容: \"%s\"\n", message);
    printf("消息长度: %lu 字节\n", strlen(message));
    
    // 初始化发送方和接收方
    sender_state_t sender;
    receiver_state_t receiver;
    init_sender(&sender);
    init_receiver(&receiver);
    
    // 为简化演示，这里传输整个消息作为一帧
    // 在实际实现中，可能需要分片处理大消息
    int message_len = strlen(message);
    if (message_len == 0) {
        printf("[错误] 消息不能为空\n");
        return false;
    }
    if (message_len > MAX_DATA_SIZE - 1) {
        printf("[错误] 消息过长，超过最大帧大小\n");
        return false;
    }
    
    // 创建数据帧
    create_data_frame(&sender.current_frame, sender.seq_num, message, message_len);
    
    // 停等协议主循环
    while (sender.state != WAITING_FOR_CALL || sender.retry_count == 0) {
        switch (sender.state) {
            case WAITING_FOR_CALL:
                // 开始传输
                printf("\n[协议状态] 发送方开始传输\n");
                sender.state = WAITING_FOR_ACK;
                
                // 发送数据帧
                send_data_frame(&sender, &sender.current_frame, config, stats);
                reset_timer(&sender);
                break;
                
            case WAITING_FOR_ACK:
                // 等待确认帧
                printf("\n[协议状态] 等待确认帧...\n");
                
                // 模拟一些延迟，让接收方有时间处理
                usleep(100000); // 100ms
                
                // 检查是否有数据帧到达接收方
                data_frame_t received_frame;
                if (receive_data_frame(&receiver, &received_frame, config, stats)) {
                    printf("[接收方] 成功接收数据帧\n");
                    
                    // 检查序列号
                    if (received_frame.seq_num == receiver.expected_seq) {
                        printf("[接收方] 序列号正确 (期望: %d, 接收: %d)\n", 
                               receiver.expected_seq, received_frame.seq_num);
                        
                        // 发送确认帧
                        ack_frame_t ack;
                        create_ack_frame(&ack, received_frame.seq_num);
                        send_ack_frame(&receiver, &ack, config, stats);
                        
                        // 更新接收方期望的序列号
                        receiver.expected_seq = (receiver.expected_seq + 1) % MAX_SEQ_NUM;
                    } else {
                        printf("[接收方] 序列号错误，丢弃帧\n");
                        // 可以选择发送上一个确认帧
                    }
                }
                
                // 检查发送方是否收到确认
                ack_frame_t received_ack;
                if (receive_ack_frame(&sender, &received_ack, stats)) {
                    printf("[发送方] 收到正确确认，传输成功!\n");
                    sender.state = WAITING_FOR_CALL;
                    sender.seq_num = (sender.seq_num + 1) % MAX_SEQ_NUM;
                    sender.retry_count = 0;
                    
                    // 传输完成
                    stats->end_time = clock();
                    printf("\n========== 传输完成 ==========\n");
                    return true;
                }
                
                // 检查超时
                if (is_timeout(&sender)) {
                    handle_timeout(&sender, config, stats);
                    if (sender.retry_count >= MAX_RETRIES) {
                        printf("\n========== 传输失败 ==========\n");
                        return false;
                    }
                }
                break;
                
            default:
                printf("[错误] 未知的协议状态\n");
                return false;
        }
        
        print_protocol_state(&sender, &receiver);
    }
    
    return false;
}

/* ========== 工具函数 ========== */

/**
 * 打印数据帧信息
 * @param frame 数据帧
 * @param direction 方向描述
 */
void print_frame_info(const data_frame_t* frame, const char* direction) {
    if (!frame || !direction) return;
    
    printf("[数据帧%s] 类型: %s, 序列号: %d, 长度: %d, 校验和: %u\n",
           direction,
           frame->type == DATA_FRAME ? "数据" : "未知",
           frame->seq_num,
           frame->data_length,
           frame->checksum);
    printf("           数据内容: \"%.50s%s\"\n", 
           frame->data, frame->data_length > 50 ? "..." : "");
}

/**
 * 打印确认帧信息
 * @param ack 确认帧
 * @param direction 方向描述
 */
void print_ack_info(const ack_frame_t* ack, const char* direction) {
    if (!ack || !direction) return;
    
    printf("[确认帧%s] 类型: %s, 确认号: %d, 校验和: %u\n",
           direction,
           ack->type == ACK_FRAME ? "确认" : "未知",
           ack->ack_num,
           ack->checksum);
}

/**
 * 打印统计信息
 * @param stats 统计信息
 */
void print_statistics(const statistics_t* stats) {
    if (!stats) return;
    
    double duration = ((double)(stats->end_time - stats->start_time)) / CLOCKS_PER_SEC;
    
    printf("\n========== 传输统计 ==========\n");
    printf("传输时间:     %.3f 秒\n", duration);
    printf("发送帧数:     %d\n", stats->frames_sent);
    printf("接收帧数:     %d\n", stats->frames_received);
    printf("发送确认数:   %d\n", stats->acks_sent);
    printf("接收确认数:   %d\n", stats->acks_received);
    printf("重传次数:     %d\n", stats->retransmissions);
    printf("丢失帧数:     %d\n", stats->frames_lost);
    
    if (stats->frames_sent > 0) {
        double success_rate = ((double)(stats->frames_sent - stats->frames_lost)) / stats->frames_sent * 100;
        printf("传输成功率:   %.1f%%\n", success_rate);
    }
    
    if (stats->retransmissions > 0 && stats->frames_received > 0) {
        double retrans_rate = ((double)stats->retransmissions) / stats->frames_sent * 100;
        printf("重传率:       %.1f%%\n", retrans_rate);
    }
    
    printf("=============================\n");
}

/**
 * 打印协议状态
 * @param sender 发送方状态
 * @param receiver 接收方状态
 */
void print_protocol_state(const sender_state_t* sender, const receiver_state_t* receiver) {
    if (!sender || !receiver) return;
    
    const char* sender_state_str[] = {"等待调用", "等待确认", "等待数据"};
    const char* receiver_state_str[] = {"等待调用", "等待确认", "等待数据"};
    
    printf("\n--- 协议状态 ---\n");
    printf("发送方: %s (序列号: %d, 重传: %d)\n", 
           sender_state_str[sender->state], sender->seq_num, sender->retry_count);
    printf("接收方: %s (期望序列号: %d)\n", 
           receiver_state_str[receiver->state], receiver->expected_seq);
    printf("---------------\n");
}