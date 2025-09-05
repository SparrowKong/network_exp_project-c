#ifndef UDP_CHAT_H
#define UDP_CHAT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/select.h>
#include <stdbool.h>

/* 常量定义 */
#define MAX_MESSAGE_SIZE 1024       // 最大消息长度
#define MAX_USERNAME_SIZE 64        // 最大用户名长度
#define MAX_CLIENTS 10              // 服务器最大客户端数量
#define DEFAULT_PORT 8888           // 默认服务端口
#define BUFFER_SIZE 2048            // 缓冲区大小
#define SELECT_TIMEOUT_SEC 1        // select超时时间（秒）

/* 消息类型定义 */
typedef enum {
    MSG_JOIN,           // 用户加入聊天室
    MSG_LEAVE,          // 用户离开聊天室
    MSG_CHAT,           // 聊天消息
    MSG_USER_LIST,      // 用户列表请求
    MSG_SERVER_INFO,    // 服务器信息
    MSG_ERROR           // 错误消息
} message_type_t;

/* 消息结构定义 */
typedef struct {
    message_type_t type;                    // 消息类型
    char username[MAX_USERNAME_SIZE];       // 发送用户名
    char content[MAX_MESSAGE_SIZE];         // 消息内容
    time_t timestamp;                       // 时间戳
    unsigned int checksum;                  // 校验和
} chat_message_t;

/* 客户端信息结构 */
typedef struct {
    struct sockaddr_in address;             // 客户端地址
    char username[MAX_USERNAME_SIZE];       // 用户名
    time_t last_activity;                   // 最后活动时间
    bool is_active;                         // 是否在线
} client_info_t;

/* 服务器状态结构 */
typedef struct {
    int socket_fd;                          // 服务器socket文件描述符
    struct sockaddr_in server_addr;         // 服务器地址
    client_info_t clients[MAX_CLIENTS];     // 客户端列表
    int client_count;                       // 当前客户端数量
    bool is_running;                        // 服务器运行状态
    time_t start_time;                      // 服务器启动时间
} server_state_t;

/* 客户端状态结构 */
typedef struct {
    int socket_fd;                          // 客户端socket文件描述符
    struct sockaddr_in server_addr;         // 服务器地址
    char username[MAX_USERNAME_SIZE];       // 用户名
    bool is_connected;                      // 连接状态
    time_t connect_time;                    // 连接时间
} client_state_t;

/* 统计信息结构 */
typedef struct {
    int messages_sent;                      // 发送消息数
    int messages_received;                  // 接收消息数
    int bytes_sent;                         // 发送字节数
    int bytes_received;                     // 接收字节数
    time_t session_start;                   // 会话开始时间
} chat_statistics_t;

/* 核心函数声明 */

/* 服务器端函数 */
int server_init(server_state_t* server, int port);
int server_run(server_state_t* server);
void server_cleanup(server_state_t* server);
int server_handle_message(server_state_t* server, char* buffer, 
                         struct sockaddr_in* client_addr, socklen_t addr_len);
int server_broadcast_message(server_state_t* server, const chat_message_t* msg, 
                            const struct sockaddr_in* sender_addr);
int server_add_client(server_state_t* server, const struct sockaddr_in* client_addr, 
                     const char* username);
int server_remove_client(server_state_t* server, const struct sockaddr_in* client_addr);
client_info_t* server_find_client(server_state_t* server, 
                                 const struct sockaddr_in* client_addr);

/* 客户端函数 */
int client_init(client_state_t* client, const char* server_ip, int port);
int client_connect(client_state_t* client, const char* username);
int client_send_message(client_state_t* client, const char* message);
int client_receive_messages(client_state_t* client);
void client_cleanup(client_state_t* client);

/* 消息处理函数 */
int message_serialize(const chat_message_t* msg, char* buffer, size_t buffer_size);
int message_deserialize(const char* buffer, size_t buffer_size, chat_message_t* msg);
unsigned int calculate_checksum(const chat_message_t* msg);
bool verify_message_integrity(const chat_message_t* msg);
void print_message(const chat_message_t* msg);

/* 工具函数 */
const char* message_type_to_string(message_type_t type);
const char* format_timestamp(time_t timestamp);
bool is_valid_username(const char* username);
bool compare_addresses(const struct sockaddr_in* addr1, const struct sockaddr_in* addr2);
void print_server_stats(const server_state_t* server, const chat_statistics_t* stats);
void print_client_stats(const client_state_t* client, const chat_statistics_t* stats);

/* 网络工具函数 */
int set_socket_nonblocking(int sockfd);
int create_udp_socket(void);
int bind_socket(int sockfd, int port);
ssize_t safe_sendto(int sockfd, const void* buf, size_t len, 
                   const struct sockaddr_in* dest_addr);
ssize_t safe_recvfrom(int sockfd, void* buf, size_t len, 
                     struct sockaddr_in* src_addr, socklen_t* addr_len);

/* 调试和日志函数 */
void debug_print(const char* format, ...);
void log_message(const char* level, const char* format, ...);

#endif /* UDP_CHAT_H */