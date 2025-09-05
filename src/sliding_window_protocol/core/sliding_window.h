#ifndef SLIDING_WINDOW_H
#define SLIDING_WINDOW_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <stdbool.h>

/* 常量定义 */
#define MAX_DATA_SIZE 1024      // 最大数据帧大小
#define MAX_SEQ_NUM 2           // 序列号范围 (0, 1) 对于停等协议
#define TIMEOUT_MS 1000         // 超时时间（毫秒）
#define MAX_RETRIES 3           // 最大重传次数

/* 帧类型定义 */
typedef enum {
    DATA_FRAME,     // 数据帧
    ACK_FRAME,      // 确认帧
    NAK_FRAME       // 否定确认帧（可选）
} frame_type_t;

/* 协议状态定义 */
typedef enum {
    WAITING_FOR_CALL,       // 等待上层调用
    WAITING_FOR_ACK,        // 等待确认帧
    WAITING_FOR_DATA        // 等待数据帧（接收方）
} protocol_state_t;

/* 数据帧结构 */
typedef struct {
    frame_type_t type;          // 帧类型
    int seq_num;                // 序列号
    int data_length;            // 数据长度
    char data[MAX_DATA_SIZE];   // 数据内容
    unsigned int checksum;      // 校验和（简单校验）
} data_frame_t;

/* 确认帧结构 */
typedef struct {
    frame_type_t type;          // 帧类型
    int ack_num;                // 确认号
    unsigned int checksum;      // 校验和
} ack_frame_t;

/* 网络环境配置 */
typedef struct {
    double loss_probability;    // 丢包概率 (0.0-1.0)
    int min_delay_ms;          // 最小延迟（毫秒）
    int max_delay_ms;          // 最大延迟（毫秒）
} network_config_t;

/* 发送方状态 */
typedef struct {
    protocol_state_t state;     // 当前状态
    int seq_num;               // 当前序列号
    data_frame_t current_frame; // 当前发送的帧
    clock_t timer_start;       // 计时器开始时间
    int retry_count;           // 重传计数
} sender_state_t;

/* 接收方状态 */
typedef struct {
    protocol_state_t state;     // 当前状态
    int expected_seq;           // 期望的序列号
} receiver_state_t;

/* 统计信息 */
typedef struct {
    int frames_sent;            // 发送的帧数
    int frames_received;        // 接收的帧数
    int acks_sent;             // 发送的确认数
    int acks_received;         // 接收的确认数
    int retransmissions;       // 重传次数
    int frames_lost;           // 丢失的帧数
    clock_t start_time;        // 开始时间
    clock_t end_time;          // 结束时间
} statistics_t;

/* 函数声明 */

/* 初始化函数 */
void init_sender(sender_state_t* sender);
void init_receiver(receiver_state_t* receiver);
void init_statistics(statistics_t* stats);
void init_network_config(network_config_t* config);

/* 帧处理函数 */
unsigned int calculate_checksum(const void* data, size_t length);
bool verify_checksum(const void* data, size_t length, unsigned int expected_checksum);
void create_data_frame(data_frame_t* frame, int seq_num, const char* data, int length);
void create_ack_frame(ack_frame_t* frame, int ack_num);

/* 网络模拟函数 */
bool simulate_frame_loss(network_config_t* config);
void simulate_network_delay(network_config_t* config);

/* 协议核心函数 */
bool send_data_frame(sender_state_t* sender, const data_frame_t* frame, 
                     network_config_t* config, statistics_t* stats);
bool receive_data_frame(receiver_state_t* receiver, data_frame_t* frame, 
                        network_config_t* config, statistics_t* stats);
bool send_ack_frame(receiver_state_t* receiver, const ack_frame_t* ack, 
                    network_config_t* config, statistics_t* stats);
bool receive_ack_frame(sender_state_t* sender, const ack_frame_t* ack, 
                       statistics_t* stats);

/* 超时处理 */
bool is_timeout(sender_state_t* sender);
void handle_timeout(sender_state_t* sender, network_config_t* config, 
                    statistics_t* stats);
void reset_timer(sender_state_t* sender);

/* 主要传输函数 */
bool transmit_message(const char* message, network_config_t* config, 
                      statistics_t* stats);

/* 工具函数 */
void print_frame_info(const data_frame_t* frame, const char* direction);
void print_ack_info(const ack_frame_t* ack, const char* direction);
void print_statistics(const statistics_t* stats);
void print_protocol_state(const sender_state_t* sender, const receiver_state_t* receiver);

#endif // SLIDING_WINDOW_H