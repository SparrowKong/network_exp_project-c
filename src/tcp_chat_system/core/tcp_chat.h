#ifndef TCP_CHAT_H
#define TCP_CHAT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <time.h>
#include <stdbool.h>
#include <signal.h>
#include <fcntl.h>

/* 常量定义 */
#define MAX_MESSAGE_SIZE 1024       // 最大消息长度
#define MAX_USERNAME_SIZE 32        // 最大用户名长度  
#define MAX_CLIENTS 10              // 最大同时连接客户端数量
#define DEFAULT_PORT 8080           // 默认端口号
#define BUFFER_SIZE 2048            // 缓冲区大小
#define BACKLOG 5                   // 监听队列长度

/* 消息类型定义 */
typedef enum {
    MSG_LOGIN,          // 用户登录消息
    MSG_LOGOUT,         // 用户退出消息
    MSG_CHAT,           // 聊天消息
    MSG_USER_LIST,      // 用户列表消息
    MSG_SYSTEM,         // 系统消息
    MSG_HEARTBEAT       // 心跳消息
} message_type_t;

/* 消息结构体 */
typedef struct {
    message_type_t type;                    // 消息类型
    char username[MAX_USERNAME_SIZE];       // 发送者用户名
    char content[MAX_MESSAGE_SIZE];         // 消息内容
    time_t timestamp;                       // 时间戳
    int client_id;                          // 客户端ID
} chat_message_t;

/* 客户端连接信息 */
typedef struct {
    int socket_fd;                          // Socket文件描述符
    struct sockaddr_in address;             // 客户端地址信息
    char username[MAX_USERNAME_SIZE];       // 用户名
    time_t connect_time;                    // 连接时间
    time_t last_heartbeat;                  // 最后心跳时间
    bool is_active;                         // 是否活跃
    int client_id;                          // 客户端唯一ID
} client_info_t;

/* 服务端状态结构体 */
typedef struct {
    int server_fd;                          // 服务端Socket
    struct sockaddr_in server_addr;         // 服务端地址
    client_info_t clients[MAX_CLIENTS];     // 客户端列表
    int client_count;                       // 当前客户端数量
    fd_set read_fds;                        // 读文件描述符集合
    int max_fd;                             // 最大文件描述符
    bool is_running;                        // 服务器运行状态
    int port;                               // 监听端口
} server_state_t;

/* 客户端状态结构体 */
typedef struct {
    int socket_fd;                          // Socket文件描述符
    struct sockaddr_in server_addr;         // 服务端地址
    char username[MAX_USERNAME_SIZE];       // 用户名
    bool is_connected;                      // 连接状态
    time_t connect_time;                    // 连接时间
    char server_ip[INET_ADDRSTRLEN];        // 服务器IP地址
    int server_port;                        // 服务器端口
} client_state_t;

/* 聊天统计信息 */
typedef struct {
    int total_messages_sent;                // 总发送消息数
    int total_messages_received;            // 总接收消息数
    int total_clients_connected;            // 总连接客户端数
    int current_online_clients;             // 当前在线客户端数
    time_t server_start_time;               // 服务器启动时间
    time_t total_running_time;              // 总运行时间
    int connection_errors;                  // 连接错误次数
    int message_errors;                     // 消息错误次数
} chat_statistics_t;

/* 服务端函数声明 */

/* 服务端初始化和管理 */
bool init_server(server_state_t* server, int port);
void cleanup_server(server_state_t* server);
void start_server(server_state_t* server);
void stop_server(server_state_t* server);
void server_main_loop(server_state_t* server, chat_statistics_t* stats);

/* 客户端连接管理 */
bool accept_new_client(server_state_t* server, chat_statistics_t* stats);
void disconnect_client(server_state_t* server, int client_index, chat_statistics_t* stats);
void cleanup_disconnected_clients(server_state_t* server);
int find_client_by_fd(server_state_t* server, int fd);
int get_available_client_slot(server_state_t* server);

/* 消息处理函数 */
bool receive_message(int socket_fd, chat_message_t* message);
bool send_message(int socket_fd, const chat_message_t* message);
void broadcast_message(server_state_t* server, const chat_message_t* message, int sender_fd);
void handle_client_message(server_state_t* server, int client_index, chat_statistics_t* stats);
void process_login_message(server_state_t* server, int client_index, const chat_message_t* message);
void process_chat_message(server_state_t* server, int client_index, const chat_message_t* message);
void send_user_list(server_state_t* server, int client_fd);
void send_system_message(int client_fd, const char* content);

/* 客户端函数声明 */

/* 客户端初始化和管理 */
bool init_client(client_state_t* client, const char* server_ip, int port, const char* username);
void cleanup_client(client_state_t* client);
bool connect_to_server(client_state_t* client);
void disconnect_from_server(client_state_t* client);
void client_main_loop(client_state_t* client, chat_statistics_t* stats);

/* 客户端消息处理 */
bool send_login_message(client_state_t* client);
bool send_chat_message(client_state_t* client, const char* content);
bool send_logout_message(client_state_t* client);
void handle_server_message(client_state_t* client, chat_statistics_t* stats);
void process_received_message(const chat_message_t* message);

/* 工具函数 */

/* 消息创建和验证 */
void create_message(chat_message_t* message, message_type_t type, 
                   const char* username, const char* content, int client_id);
bool validate_message(const chat_message_t* message);
void format_timestamp(time_t timestamp, char* buffer, size_t buffer_size);

/* 网络工具函数 */
bool set_socket_nonblocking(int socket_fd);
bool set_socket_reuseaddr(int socket_fd);
void print_socket_error(const char* operation);
bool is_valid_ip(const char* ip);
bool is_valid_port(int port);

/* 用户界面辅助函数 */
void print_welcome_message(void);
void print_server_info(const server_state_t* server);
void print_client_info(const client_state_t* client);
void print_chat_statistics(const chat_statistics_t* stats);
void print_online_users(const server_state_t* server);
void print_message_formatted(const chat_message_t* message);

/* 统计信息管理 */
void init_statistics(chat_statistics_t* stats);
void update_statistics_message_sent(chat_statistics_t* stats);
void update_statistics_message_received(chat_statistics_t* stats);
void update_statistics_client_connected(chat_statistics_t* stats);
void update_statistics_client_disconnected(chat_statistics_t* stats);
void update_statistics_error(chat_statistics_t* stats, bool is_connection_error);

/* 调试和测试函数 */
void print_debug_info(const char* function, const char* message);
void log_server_event(const server_state_t* server, const char* event);
void log_client_event(const client_state_t* client, const char* event);
bool test_socket_creation(void);
bool test_address_binding(int port);
bool test_message_serialization(void);

#endif // TCP_CHAT_H