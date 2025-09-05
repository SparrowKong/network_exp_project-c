#include "udp_chat.h"
#include <stdarg.h>

/* 调试开关 */
static bool debug_enabled = true;

/* ========== 服务器端核心函数实现 ========== */

/**
 * 初始化UDP聊天服务器
 * @param server 服务器状态结构指针
 * @param port 监听端口
 * @return 成功返回0，失败返回-1
 */
int server_init(server_state_t* server, int port) {
    if (!server) {
        log_message("ERROR", "服务器状态结构为空");
        return -1;
    }
    
    // 清空服务器状态
    memset(server, 0, sizeof(server_state_t));
    
    // 创建UDP socket
    server->socket_fd = create_udp_socket();
    if (server->socket_fd == -1) {
        log_message("ERROR", "创建UDP socket失败: %s", strerror(errno));
        return -1;
    }
    
    // 绑定端口
    if (bind_socket(server->socket_fd, port) == -1) {
        log_message("ERROR", "绑定端口 %d 失败: %s", port, strerror(errno));
        close(server->socket_fd);
        return -1;
    }
    
    // 设置为非阻塞模式
    if (set_socket_nonblocking(server->socket_fd) == -1) {
        log_message("ERROR", "设置socket为非阻塞模式失败: %s", strerror(errno));
        close(server->socket_fd);
        return -1;
    }
    
    // 初始化服务器地址信息
    server->server_addr.sin_family = AF_INET;
    server->server_addr.sin_addr.s_addr = INADDR_ANY;
    server->server_addr.sin_port = htons(port);
    
    server->is_running = false;
    server->client_count = 0;
    server->start_time = time(NULL);
    
    log_message("INFO", "UDP聊天服务器初始化完成，监听端口: %d", port);
    return 0;
}

/**
 * 运行UDP聊天服务器主循环
 * @param server 服务器状态结构指针
 * @return 成功返回0，失败返回-1
 */
int server_run(server_state_t* server) {
    if (!server || server->socket_fd <= 0) {
        log_message("ERROR", "服务器未正确初始化");
        return -1;
    }
    
    server->is_running = true;
    log_message("INFO", "UDP聊天服务器开始运行...");
    
    char buffer[BUFFER_SIZE];
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    fd_set read_fds;
    struct timeval timeout;
    
    while (server->is_running) {
        // 设置select参数
        FD_ZERO(&read_fds);
        FD_SET(server->socket_fd, &read_fds);
        
        timeout.tv_sec = SELECT_TIMEOUT_SEC;
        timeout.tv_usec = 0;
        
        // 等待数据到达
        int result = select(server->socket_fd + 1, &read_fds, NULL, NULL, &timeout);
        
        if (result == -1) {
            if (errno != EINTR) {
                log_message("ERROR", "select错误: %s", strerror(errno));
                break;
            }
            continue;
        }
        
        if (result == 0) {
            // 超时，检查客户端状态
            continue;
        }
        
        if (FD_ISSET(server->socket_fd, &read_fds)) {
            // 接收客户端消息
            ssize_t bytes_received = safe_recvfrom(server->socket_fd, buffer, 
                                                  BUFFER_SIZE - 1, &client_addr, &addr_len);
            
            if (bytes_received > 0) {
                buffer[bytes_received] = '\0';
                debug_print("收到来自 %s:%d 的消息，长度: %zd", 
                           inet_ntoa(client_addr.sin_addr), 
                           ntohs(client_addr.sin_port), bytes_received);
                
                // 处理接收到的消息
                server_handle_message(server, buffer, &client_addr, addr_len);
            }
        }
    }
    
    log_message("INFO", "UDP聊天服务器停止运行");
    return 0;
}

/**
 * 处理客户端消息
 * @param server 服务器状态结构指针
 * @param buffer 消息缓冲区
 * @param client_addr 客户端地址
 * @param addr_len 地址长度
 * @return 成功返回0，失败返回-1
 */
int server_handle_message(server_state_t* server, char* buffer, 
                         struct sockaddr_in* client_addr, socklen_t addr_len) {
    chat_message_t message;
    
    // 反序列化消息
    if (message_deserialize(buffer, strlen(buffer), &message) == -1) {
        log_message("ERROR", "消息反序列化失败");
        return -1;
    }
    
    // 验证消息完整性
    if (!verify_message_integrity(&message)) {
        log_message("ERROR", "消息校验失败，丢弃消息");
        return -1;
    }
    
    debug_print("处理消息类型: %s, 来自用户: %s", 
               message_type_to_string(message.type), message.username);
    
    switch (message.type) {
        case MSG_JOIN:
            // 用户加入聊天室
            if (server_add_client(server, client_addr, message.username) == 0) {
                // 广播用户加入消息
                snprintf(message.content, sizeof(message.content), 
                        "用户 %s 加入了聊天室", message.username);
                message.type = MSG_SERVER_INFO;
                server_broadcast_message(server, &message, NULL);
            }
            break;
            
        case MSG_LEAVE:
            // 用户离开聊天室
            if (server_remove_client(server, client_addr) == 0) {
                // 广播用户离开消息
                snprintf(message.content, sizeof(message.content), 
                        "用户 %s 离开了聊天室", message.username);
                message.type = MSG_SERVER_INFO;
                server_broadcast_message(server, &message, client_addr);
            }
            break;
            
        case MSG_CHAT:
            // 聊天消息，转发给所有其他客户端
            print_message(&message);
            server_broadcast_message(server, &message, client_addr);
            break;
            
        case MSG_USER_LIST:
            // 发送在线用户列表（简化实现）
            message.type = MSG_SERVER_INFO;
            snprintf(message.content, sizeof(message.content), 
                    "当前在线用户数: %d", server->client_count);
            
            char response_buffer[BUFFER_SIZE];
            if (message_serialize(&message, response_buffer, sizeof(response_buffer)) > 0) {
                safe_sendto(server->socket_fd, response_buffer, strlen(response_buffer), client_addr);
            }
            break;
            
        default:
            log_message("WARN", "未知消息类型: %d", message.type);
            break;
    }
    
    return 0;
}

/**
 * 广播消息给所有客户端
 * @param server 服务器状态结构指针
 * @param msg 要广播的消息
 * @param sender_addr 发送方地址（如果不为NULL，则不发送给发送方）
 * @return 成功返回0，失败返回-1
 */
int server_broadcast_message(server_state_t* server, const chat_message_t* msg, 
                            const struct sockaddr_in* sender_addr) {
    char buffer[BUFFER_SIZE];
    
    // 序列化消息
    int message_len = message_serialize(msg, buffer, sizeof(buffer));
    if (message_len <= 0) {
        log_message("ERROR", "消息序列化失败");
        return -1;
    }
    
    int sent_count = 0;
    
    // 发送给所有活跃的客户端
    for (int i = 0; i < server->client_count; i++) {
        if (server->clients[i].is_active) {
            // 如果指定了发送方地址，则不发送给发送方自己
            if (sender_addr && compare_addresses(&server->clients[i].address, sender_addr)) {
                continue;
            }
            
            ssize_t sent_bytes = safe_sendto(server->socket_fd, buffer, message_len, 
                                           &server->clients[i].address);
            if (sent_bytes > 0) {
                sent_count++;
                debug_print("消息发送到 %s:%d", 
                           inet_ntoa(server->clients[i].address.sin_addr),
                           ntohs(server->clients[i].address.sin_port));
            }
        }
    }
    
    debug_print("消息广播完成，发送给 %d 个客户端", sent_count);
    return 0;
}

/**
 * 添加新客户端到服务器
 * @param server 服务器状态结构指针
 * @param client_addr 客户端地址
 * @param username 用户名
 * @return 成功返回0，失败返回-1
 */
int server_add_client(server_state_t* server, const struct sockaddr_in* client_addr, 
                     const char* username) {
    if (server->client_count >= MAX_CLIENTS) {
        log_message("WARN", "服务器客户端数量已达上限");
        return -1;
    }
    
    if (!is_valid_username(username)) {
        log_message("WARN", "无效的用户名: %s", username);
        return -1;
    }
    
    // 检查客户端是否已存在
    client_info_t* existing_client = server_find_client(server, client_addr);
    if (existing_client) {
        // 更新现有客户端信息
        strncpy(existing_client->username, username, sizeof(existing_client->username) - 1);
        existing_client->username[sizeof(existing_client->username) - 1] = '\0';
        existing_client->last_activity = time(NULL);
        existing_client->is_active = true;
        log_message("INFO", "更新客户端信息: %s", username);
        return 0;
    }
    
    // 添加新客户端
    client_info_t* new_client = &server->clients[server->client_count];
    new_client->address = *client_addr;
    strncpy(new_client->username, username, sizeof(new_client->username) - 1);
    new_client->username[sizeof(new_client->username) - 1] = '\0';
    new_client->last_activity = time(NULL);
    new_client->is_active = true;
    
    server->client_count++;
    
    log_message("INFO", "新客户端加入: %s (%s:%d), 当前客户端数: %d",
               username, inet_ntoa(client_addr->sin_addr), 
               ntohs(client_addr->sin_port), server->client_count);
    
    return 0;
}

/**
 * 从服务器移除客户端
 * @param server 服务器状态结构指针
 * @param client_addr 客户端地址
 * @return 成功返回0，失败返回-1
 */
int server_remove_client(server_state_t* server, const struct sockaddr_in* client_addr) {
    for (int i = 0; i < server->client_count; i++) {
        if (compare_addresses(&server->clients[i].address, client_addr)) {
            log_message("INFO", "客户端离开: %s", server->clients[i].username);
            
            // 将后面的客户端前移
            for (int j = i; j < server->client_count - 1; j++) {
                server->clients[j] = server->clients[j + 1];
            }
            
            server->client_count--;
            return 0;
        }
    }
    
    return -1;  // 未找到客户端
}

/**
 * 查找客户端
 * @param server 服务器状态结构指针
 * @param client_addr 客户端地址
 * @return 找到返回客户端指针，未找到返回NULL
 */
client_info_t* server_find_client(server_state_t* server, 
                                 const struct sockaddr_in* client_addr) {
    for (int i = 0; i < server->client_count; i++) {
        if (compare_addresses(&server->clients[i].address, client_addr)) {
            return &server->clients[i];
        }
    }
    return NULL;
}

/**
 * 清理服务器资源
 * @param server 服务器状态结构指针
 */
void server_cleanup(server_state_t* server) {
    if (server && server->socket_fd > 0) {
        close(server->socket_fd);
        server->socket_fd = -1;
        server->is_running = false;
        log_message("INFO", "服务器资源清理完成");
    }
}

/* ========== 客户端核心函数实现 ========== */

/**
 * 初始化UDP聊天客户端
 * @param client 客户端状态结构指针
 * @param server_ip 服务器IP地址
 * @param port 服务器端口
 * @return 成功返回0，失败返回-1
 */
int client_init(client_state_t* client, const char* server_ip, int port) {
    if (!client || !server_ip) {
        log_message("ERROR", "参数错误");
        return -1;
    }
    
    // 清空客户端状态
    memset(client, 0, sizeof(client_state_t));
    
    // 创建UDP socket
    client->socket_fd = create_udp_socket();
    if (client->socket_fd == -1) {
        log_message("ERROR", "创建UDP socket失败: %s", strerror(errno));
        return -1;
    }
    
    // 设置服务器地址
    client->server_addr.sin_family = AF_INET;
    client->server_addr.sin_port = htons(port);
    if (inet_aton(server_ip, &client->server_addr.sin_addr) == 0) {
        log_message("ERROR", "无效的IP地址: %s", server_ip);
        close(client->socket_fd);
        return -1;
    }
    
    client->is_connected = false;
    
    log_message("INFO", "客户端初始化完成，服务器: %s:%d", server_ip, port);
    return 0;
}

/**
 * 客户端连接服务器
 * @param client 客户端状态结构指针
 * @param username 用户名
 * @return 成功返回0，失败返回-1
 */
int client_connect(client_state_t* client, const char* username) {
    if (!client || !username || !is_valid_username(username)) {
        log_message("ERROR", "参数错误或用户名无效");
        return -1;
    }
    
    // 保存用户名
    strncpy(client->username, username, sizeof(client->username) - 1);
    client->username[sizeof(client->username) - 1] = '\0';
    
    // 发送加入消息
    chat_message_t join_msg = {0};
    join_msg.type = MSG_JOIN;
    strncpy(join_msg.username, username, sizeof(join_msg.username) - 1);
    join_msg.username[sizeof(join_msg.username) - 1] = '\0';
    snprintf(join_msg.content, sizeof(join_msg.content), "用户 %s 请求加入聊天室", username);
    join_msg.timestamp = time(NULL);
    join_msg.checksum = calculate_checksum(&join_msg);
    
    char buffer[BUFFER_SIZE];
    int msg_len = message_serialize(&join_msg, buffer, sizeof(buffer));
    if (msg_len <= 0) {
        log_message("ERROR", "消息序列化失败");
        return -1;
    }
    
    ssize_t sent_bytes = safe_sendto(client->socket_fd, buffer, msg_len, &client->server_addr);
    if (sent_bytes <= 0) {
        log_message("ERROR", "发送加入消息失败");
        return -1;
    }
    
    client->is_connected = true;
    client->connect_time = time(NULL);
    
    log_message("INFO", "客户端 %s 连接成功", username);
    return 0;
}

/**
 * 客户端发送消息
 * @param client 客户端状态结构指针
 * @param message 要发送的消息内容
 * @return 成功返回0，失败返回-1
 */
int client_send_message(client_state_t* client, const char* message) {
    if (!client || !client->is_connected || !message) {
        return -1;
    }
    
    chat_message_t chat_msg = {0};
    chat_msg.type = MSG_CHAT;
    strncpy(chat_msg.username, client->username, sizeof(chat_msg.username) - 1);
    chat_msg.username[sizeof(chat_msg.username) - 1] = '\0';
    strncpy(chat_msg.content, message, sizeof(chat_msg.content) - 1);
    chat_msg.content[sizeof(chat_msg.content) - 1] = '\0';
    chat_msg.timestamp = time(NULL);
    chat_msg.checksum = calculate_checksum(&chat_msg);
    
    char buffer[BUFFER_SIZE];
    int msg_len = message_serialize(&chat_msg, buffer, sizeof(buffer));
    if (msg_len <= 0) {
        return -1;
    }
    
    ssize_t sent_bytes = safe_sendto(client->socket_fd, buffer, msg_len, &client->server_addr);
    return (sent_bytes > 0) ? 0 : -1;
}

/**
 * 客户端接收消息
 * @param client 客户端状态结构指针
 * @return 成功返回0，失败返回-1
 */
int client_receive_messages(client_state_t* client) {
    if (!client || !client->is_connected) {
        return -1;
    }
    
    char buffer[BUFFER_SIZE];
    struct sockaddr_in server_addr;
    socklen_t addr_len = sizeof(server_addr);
    
    ssize_t bytes_received = safe_recvfrom(client->socket_fd, buffer, 
                                          BUFFER_SIZE - 1, &server_addr, &addr_len);
    if (bytes_received <= 0) {
        return -1;
    }
    
    buffer[bytes_received] = '\0';
    
    chat_message_t message;
    if (message_deserialize(buffer, strlen(buffer), &message) == 0) {
        if (verify_message_integrity(&message)) {
            print_message(&message);
        }
    }
    
    return 0;
}

/**
 * 清理客户端资源
 * @param client 客户端状态结构指针
 */
void client_cleanup(client_state_t* client) {
    if (client && client->socket_fd > 0) {
        // 发送离开消息
        if (client->is_connected) {
            chat_message_t leave_msg = {0};
            leave_msg.type = MSG_LEAVE;
            strncpy(leave_msg.username, client->username, sizeof(leave_msg.username) - 1);
            leave_msg.username[sizeof(leave_msg.username) - 1] = '\0';
            snprintf(leave_msg.content, sizeof(leave_msg.content), "用户 %s 离开聊天室", client->username);
            leave_msg.timestamp = time(NULL);
            leave_msg.checksum = calculate_checksum(&leave_msg);
            
            char buffer[BUFFER_SIZE];
            int msg_len = message_serialize(&leave_msg, buffer, sizeof(buffer));
            if (msg_len > 0) {
                safe_sendto(client->socket_fd, buffer, msg_len, &client->server_addr);
            }
        }
        
        close(client->socket_fd);
        client->socket_fd = -1;
        client->is_connected = false;
        log_message("INFO", "客户端资源清理完成");
    }
}

/* ========== 消息处理函数实现 ========== */

/**
 * 消息序列化
 * @param msg 消息结构指针
 * @param buffer 缓冲区
 * @param buffer_size 缓冲区大小
 * @return 序列化后的字节数，失败返回-1
 */
int message_serialize(const chat_message_t* msg, char* buffer, size_t buffer_size) {
    if (!msg || !buffer || buffer_size < sizeof(chat_message_t)) {
        return -1;
    }
    
    // 简单的序列化：直接复制结构体（注意：实际项目中应考虑字节序和填充）
    memcpy(buffer, msg, sizeof(chat_message_t));
    return sizeof(chat_message_t);
}

/**
 * 消息反序列化
 * @param buffer 缓冲区
 * @param buffer_size 缓冲区大小
 * @param msg 消息结构指针
 * @return 成功返回0，失败返回-1
 */
int message_deserialize(const char* buffer, size_t buffer_size, chat_message_t* msg) {
    if (!buffer || !msg || buffer_size < sizeof(chat_message_t)) {
        return -1;
    }
    
    // 简单的反序列化：直接复制结构体
    memcpy(msg, buffer, sizeof(chat_message_t));
    return 0;
}

/**
 * 计算消息校验和（简单实现）
 * @param msg 消息结构指针
 * @return 校验和值
 */
unsigned int calculate_checksum(const chat_message_t* msg) {
    if (!msg) return 0;
    
    unsigned int checksum = 0;
    const unsigned char* data = (const unsigned char*)msg;
    
    // 计算除了校验和字段外的所有字节的校验和
    size_t len = sizeof(chat_message_t) - sizeof(msg->checksum);
    for (size_t i = 0; i < len; i++) {
        checksum += data[i];
    }
    
    return checksum;
}

/**
 * 验证消息完整性
 * @param msg 消息结构指针
 * @return 校验成功返回true，失败返回false
 */
bool verify_message_integrity(const chat_message_t* msg) {
    if (!msg) return false;
    
    unsigned int calculated_checksum = calculate_checksum(msg);
    return calculated_checksum == msg->checksum;
}

/**
 * 打印消息
 * @param msg 消息结构指针
 */
void print_message(const chat_message_t* msg) {
    if (!msg) return;
    
    printf("[%s] %s: %s\n", 
           format_timestamp(msg->timestamp), 
           msg->username, 
           msg->content);
}

/* ========== 工具函数实现 ========== */

/**
 * 消息类型转字符串
 * @param type 消息类型
 * @return 类型字符串
 */
const char* message_type_to_string(message_type_t type) {
    switch (type) {
        case MSG_JOIN: return "加入";
        case MSG_LEAVE: return "离开";
        case MSG_CHAT: return "聊天";
        case MSG_USER_LIST: return "用户列表";
        case MSG_SERVER_INFO: return "服务器信息";
        case MSG_ERROR: return "错误";
        default: return "未知";
    }
}

/**
 * 格式化时间戳
 * @param timestamp 时间戳
 * @return 格式化后的时间字符串
 */
const char* format_timestamp(time_t timestamp) {
    static char time_str[64];
    struct tm* tm_info = localtime(&timestamp);
    strftime(time_str, sizeof(time_str), "%H:%M:%S", tm_info);
    return time_str;
}

/**
 * 验证用户名有效性
 * @param username 用户名
 * @return 有效返回true，无效返回false
 */
bool is_valid_username(const char* username) {
    if (!username || strlen(username) == 0 || strlen(username) >= MAX_USERNAME_SIZE) {
        return false;
    }
    
    // 检查是否包含非法字符（简化检查）
    for (const char* p = username; *p; p++) {
        if (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') {
            return false;
        }
    }
    
    return true;
}

/**
 * 比较两个网络地址
 * @param addr1 地址1
 * @param addr2 地址2
 * @return 相同返回true，不同返回false
 */
bool compare_addresses(const struct sockaddr_in* addr1, const struct sockaddr_in* addr2) {
    if (!addr1 || !addr2) return false;
    
    return (addr1->sin_addr.s_addr == addr2->sin_addr.s_addr) && 
           (addr1->sin_port == addr2->sin_port);
}

/**
 * 打印服务器统计信息
 * @param server 服务器状态结构指针
 * @param stats 统计信息结构指针
 */
void print_server_stats(const server_state_t* server, const chat_statistics_t* stats) {
    if (!server || !stats) return;
    
    printf("\n=== 服务器统计信息 ===\n");
    printf("运行时间: %ld 秒\n", time(NULL) - server->start_time);
    printf("当前客户端数: %d\n", server->client_count);
    printf("发送消息数: %d\n", stats->messages_sent);
    printf("接收消息数: %d\n", stats->messages_received);
    printf("发送字节数: %d\n", stats->bytes_sent);
    printf("接收字节数: %d\n", stats->bytes_received);
    printf("===================\n\n");
}

/**
 * 打印客户端统计信息
 * @param client 客户端状态结构指针
 * @param stats 统计信息结构指针
 */
void print_client_stats(const client_state_t* client, const chat_statistics_t* stats) {
    if (!client || !stats) return;
    
    printf("\n=== 客户端统计信息 ===\n");
    printf("用户名: %s\n", client->username);
    printf("连接时间: %ld 秒\n", time(NULL) - client->connect_time);
    printf("发送消息数: %d\n", stats->messages_sent);
    printf("接收消息数: %d\n", stats->messages_received);
    printf("发送字节数: %d\n", stats->bytes_sent);
    printf("接收字节数: %d\n", stats->bytes_received);
    printf("===================\n\n");
}

/* ========== 网络工具函数实现 ========== */

/**
 * 设置socket为非阻塞模式
 * @param sockfd socket文件描述符
 * @return 成功返回0，失败返回-1
 */
int set_socket_nonblocking(int sockfd) {
    int flags = fcntl(sockfd, F_GETFL, 0);
    if (flags == -1) {
        return -1;
    }
    
    return fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
}

/**
 * 创建UDP socket
 * @return 成功返回socket文件描述符，失败返回-1
 */
int create_udp_socket(void) {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1) {
        return -1;
    }
    
    // 设置地址重用
    int opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    return sockfd;
}

/**
 * 绑定socket到指定端口
 * @param sockfd socket文件描述符
 * @param port 端口号
 * @return 成功返回0，失败返回-1
 */
int bind_socket(int sockfd, int port) {
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    
    return bind(sockfd, (struct sockaddr*)&addr, sizeof(addr));
}

/**
 * 安全的sendto函数
 * @param sockfd socket文件描述符
 * @param buf 数据缓冲区
 * @param len 数据长度
 * @param dest_addr 目标地址
 * @return 发送字节数，失败返回-1
 */
ssize_t safe_sendto(int sockfd, const void* buf, size_t len, 
                   const struct sockaddr_in* dest_addr) {
    return sendto(sockfd, buf, len, 0, 
                 (const struct sockaddr*)dest_addr, sizeof(*dest_addr));
}

/**
 * 安全的recvfrom函数
 * @param sockfd socket文件描述符
 * @param buf 数据缓冲区
 * @param len 缓冲区大小
 * @param src_addr 源地址
 * @param addr_len 地址长度
 * @return 接收字节数，失败返回-1
 */
ssize_t safe_recvfrom(int sockfd, void* buf, size_t len, 
                     struct sockaddr_in* src_addr, socklen_t* addr_len) {
    return recvfrom(sockfd, buf, len, 0, 
                   (struct sockaddr*)src_addr, addr_len);
}

/* ========== 调试和日志函数实现 ========== */

/**
 * 调试信息打印
 * @param format 格式字符串
 * @param ... 可变参数
 */
void debug_print(const char* format, ...) {
    if (!debug_enabled) return;
    
    va_list args;
    va_start(args, format);
    printf("[DEBUG] ");
    vprintf(format, args);
    printf("\n");
    va_end(args);
}

/**
 * 日志信息打印
 * @param level 日志级别
 * @param format 格式字符串
 * @param ... 可变参数
 */
void log_message(const char* level, const char* format, ...) {
    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    
    printf("[%02d:%02d:%02d] [%s] ", 
           tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec, level);
    
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    printf("\n");
    va_end(args);
}