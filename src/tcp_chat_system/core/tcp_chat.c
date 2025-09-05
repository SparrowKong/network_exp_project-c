#include "tcp_chat.h"

/* ================================
 * 工具函数实现
 * ================================ */

void print_debug_info(const char* function, const char* message) {
    printf("[调试] %s: %s\n", function, message);
}

void create_message(chat_message_t* message, message_type_t type, 
                   const char* username, const char* content, int client_id) {
    if (!message) return;
    
    message->type = type;
    message->client_id = client_id;
    message->timestamp = time(NULL);
    
    if (username) {
        strncpy(message->username, username, MAX_USERNAME_SIZE - 1);
        message->username[MAX_USERNAME_SIZE - 1] = '\0';
    } else {
        message->username[0] = '\0';
    }
    
    if (content) {
        strncpy(message->content, content, MAX_MESSAGE_SIZE - 1);
        message->content[MAX_MESSAGE_SIZE - 1] = '\0';
    } else {
        message->content[0] = '\0';
    }
}

bool validate_message(const chat_message_t* message) {
    if (!message) return false;
    
    // 检查消息类型是否有效
    if (message->type < MSG_LOGIN || message->type > MSG_HEARTBEAT) {
        return false;
    }
    
    // 检查用户名长度
    size_t username_len = strlen(message->username);
    if (username_len == 0 || username_len >= MAX_USERNAME_SIZE) {
        return false;
    }
    
    // 检查消息内容长度
    if (strlen(message->content) >= MAX_MESSAGE_SIZE) {
        return false;
    }
    
    return true;
}

void format_timestamp(time_t timestamp, char* buffer, size_t buffer_size) {
    if (!buffer) return;
    
    struct tm* tm_info = localtime(&timestamp);
    strftime(buffer, buffer_size, "%Y-%m-%d %H:%M:%S", tm_info);
}

bool set_socket_nonblocking(int socket_fd) {
    int flags = fcntl(socket_fd, F_GETFL, 0);
    if (flags == -1) return false;
    
    return fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK) == 0;
}

bool set_socket_reuseaddr(int socket_fd) {
    int opt = 1;
    return setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == 0;
}

void print_socket_error(const char* operation) {
    printf("[错误] %s失败: %s\n", operation, strerror(errno));
}

bool is_valid_ip(const char* ip) {
    if (!ip) return false;
    
    struct sockaddr_in sa;
    return inet_pton(AF_INET, ip, &(sa.sin_addr)) == 1;
}

bool is_valid_port(int port) {
    return port > 0 && port < 65536;
}

/* ================================
 * 消息发送/接收函数实现
 * ================================ */

bool receive_message(int socket_fd, chat_message_t* message) {
    if (!message) return false;
    
    ssize_t bytes_received = recv(socket_fd, message, sizeof(chat_message_t), 0);
    
    if (bytes_received <= 0) {
        if (bytes_received == 0) {
            print_debug_info(__func__, "连接已关闭");
        } else {
            print_socket_error("接收消息");
        }
        return false;
    }
    
    if (bytes_received != sizeof(chat_message_t)) {
        print_debug_info(__func__, "接收到不完整的消息");
        return false;
    }
    
    return validate_message(message);
}

bool send_message(int socket_fd, const chat_message_t* message) {
    if (!message || !validate_message(message)) {
        return false;
    }
    
    ssize_t bytes_sent = send(socket_fd, message, sizeof(chat_message_t), 0);
    
    if (bytes_sent <= 0) {
        print_socket_error("发送消息");
        return false;
    }
    
    if (bytes_sent != sizeof(chat_message_t)) {
        print_debug_info(__func__, "消息发送不完整");
        return false;
    }
    
    return true;
}

/* ================================
 * 统计信息管理函数实现
 * ================================ */

void init_statistics(chat_statistics_t* stats) {
    if (!stats) return;
    
    memset(stats, 0, sizeof(chat_statistics_t));
    stats->server_start_time = time(NULL);
}

void update_statistics_message_sent(chat_statistics_t* stats) {
    if (stats) stats->total_messages_sent++;
}

void update_statistics_message_received(chat_statistics_t* stats) {
    if (stats) stats->total_messages_received++;
}

void update_statistics_client_connected(chat_statistics_t* stats) {
    if (stats) {
        stats->total_clients_connected++;
        stats->current_online_clients++;
    }
}

void update_statistics_client_disconnected(chat_statistics_t* stats) {
    if (stats && stats->current_online_clients > 0) {
        stats->current_online_clients--;
    }
}

void update_statistics_error(chat_statistics_t* stats, bool is_connection_error) {
    if (!stats) return;
    
    if (is_connection_error) {
        stats->connection_errors++;
    } else {
        stats->message_errors++;
    }
}

/* ================================
 * 服务端函数实现
 * ================================ */

bool init_server(server_state_t* server, int port) {
    if (!server || !is_valid_port(port)) {
        return false;
    }
    
    // 初始化服务端状态
    memset(server, 0, sizeof(server_state_t));
    server->port = port;
    server->client_count = 0;
    server->is_running = false;
    server->max_fd = 0;
    
    // 初始化客户端列表
    for (int i = 0; i < MAX_CLIENTS; i++) {
        server->clients[i].socket_fd = -1;
        server->clients[i].is_active = false;
        server->clients[i].client_id = -1;
    }
    
    // 创建Socket
    server->server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server->server_fd < 0) {
        print_socket_error("创建Socket");
        return false;
    }
    
    // 设置Socket选项
    if (!set_socket_reuseaddr(server->server_fd)) {
        print_socket_error("设置Socket重用地址");
        close(server->server_fd);
        return false;
    }
    
    // 配置服务端地址
    memset(&server->server_addr, 0, sizeof(server->server_addr));
    server->server_addr.sin_family = AF_INET;
    server->server_addr.sin_addr.s_addr = INADDR_ANY;
    server->server_addr.sin_port = htons(port);
    
    // 绑定地址
    if (bind(server->server_fd, (struct sockaddr*)&server->server_addr, 
             sizeof(server->server_addr)) < 0) {
        print_socket_error("绑定地址");
        close(server->server_fd);
        return false;
    }
    
    // 开始监听
    if (listen(server->server_fd, BACKLOG) < 0) {
        print_socket_error("开始监听");
        close(server->server_fd);
        return false;
    }
    
    server->max_fd = server->server_fd;
    printf("✓ 服务端初始化成功，监听端口: %d\n", port);
    return true;
}

void cleanup_server(server_state_t* server) {
    if (!server) return;
    
    // 断开所有客户端连接
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (server->clients[i].is_active && server->clients[i].socket_fd >= 0) {
            close(server->clients[i].socket_fd);
            server->clients[i].is_active = false;
        }
    }
    
    // 关闭服务端Socket
    if (server->server_fd >= 0) {
        close(server->server_fd);
    }
    
    server->is_running = false;
    printf("✓ 服务端清理完成\n");
}

void start_server(server_state_t* server) {
    if (!server) return;
    
    server->is_running = true;
    printf("✓ 服务端启动成功\n");
}

void stop_server(server_state_t* server) {
    if (!server) return;
    
    server->is_running = false;
    printf("✓ 服务端停止\n");
}

int get_available_client_slot(server_state_t* server) {
    if (!server) return -1;
    
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (!server->clients[i].is_active) {
            return i;
        }
    }
    return -1;
}

int find_client_by_fd(server_state_t* server, int fd) {
    if (!server) return -1;
    
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (server->clients[i].is_active && server->clients[i].socket_fd == fd) {
            return i;
        }
    }
    return -1;
}

bool accept_new_client(server_state_t* server, chat_statistics_t* stats) {
    if (!server) return false;
    
    // 查找可用的客户端槽位
    int client_index = get_available_client_slot(server);
    if (client_index == -1) {
        printf("⚠️  达到最大客户端连接数限制\n");
        return false;
    }
    
    // 接受新连接
    socklen_t addr_len = sizeof(struct sockaddr_in);
    int client_fd = accept(server->server_fd, 
                          (struct sockaddr*)&server->clients[client_index].address, 
                          &addr_len);
    
    if (client_fd < 0) {
        print_socket_error("接受客户端连接");
        if (stats) update_statistics_error(stats, true);
        return false;
    }
    
    // 初始化客户端信息
    client_info_t* client = &server->clients[client_index];
    client->socket_fd = client_fd;
    client->is_active = true;
    client->client_id = client_index;
    client->connect_time = time(NULL);
    client->last_heartbeat = client->connect_time;
    strcpy(client->username, ""); // 用户名稍后在登录时设置
    
    // 更新服务端状态
    server->client_count++;
    if (client_fd > server->max_fd) {
        server->max_fd = client_fd;
    }
    
    // 更新统计信息
    if (stats) update_statistics_client_connected(stats);
    
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client->address.sin_addr, client_ip, INET_ADDRSTRLEN);
    printf("✓ 新客户端连接: %s:%d (ID: %d)\n", 
           client_ip, ntohs(client->address.sin_port), client_index);
    
    return true;
}

void disconnect_client(server_state_t* server, int client_index, chat_statistics_t* stats) {
    if (!server || client_index < 0 || client_index >= MAX_CLIENTS) return;
    
    client_info_t* client = &server->clients[client_index];
    if (!client->is_active) return;
    
    // 发送用户离开的系统消息
    if (strlen(client->username) > 0) {
        chat_message_t logout_msg;
        char logout_content[MAX_MESSAGE_SIZE];
        snprintf(logout_content, sizeof(logout_content), "%s 离开了聊天室", client->username);
        create_message(&logout_msg, MSG_SYSTEM, "系统", logout_content, -1);
        broadcast_message(server, &logout_msg, client->socket_fd);
    }
    
    // 关闭连接
    close(client->socket_fd);
    
    // 清理客户端信息
    memset(client, 0, sizeof(client_info_t));
    client->socket_fd = -1;
    client->is_active = false;
    client->client_id = -1;
    
    // 更新服务端状态
    server->client_count--;
    
    // 更新统计信息
    if (stats) update_statistics_client_disconnected(stats);
    
    printf("✓ 客户端断开连接 (ID: %d)\n", client_index);
}

void broadcast_message(server_state_t* server, const chat_message_t* message, int sender_fd) {
    if (!server || !message) return;
    
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (server->clients[i].is_active && 
            server->clients[i].socket_fd != sender_fd &&
            server->clients[i].socket_fd >= 0) {
            
            if (!send_message(server->clients[i].socket_fd, message)) {
                printf("⚠️  向客户端 %d 发送消息失败\n", i);
            }
        }
    }
}

void send_system_message(int client_fd, const char* content) {
    if (client_fd < 0 || !content) return;
    
    chat_message_t sys_msg;
    create_message(&sys_msg, MSG_SYSTEM, "系统", content, -1);
    send_message(client_fd, &sys_msg);
}

void send_user_list(server_state_t* server, int client_fd) {
    if (!server || client_fd < 0) return;
    
    char user_list[MAX_MESSAGE_SIZE] = "在线用户: ";
    bool has_users = false;
    
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (server->clients[i].is_active && strlen(server->clients[i].username) > 0) {
            if (has_users) {
                strcat(user_list, ", ");
            }
            strcat(user_list, server->clients[i].username);
            has_users = true;
        }
    }
    
    if (!has_users) {
        strcat(user_list, "无");
    }
    
    send_system_message(client_fd, user_list);
}

void process_login_message(server_state_t* server, int client_index, const chat_message_t* message) {
    if (!server || client_index < 0 || client_index >= MAX_CLIENTS || !message) return;
    
    client_info_t* client = &server->clients[client_index];
    
    // 检查用户名是否已存在
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (i != client_index && server->clients[i].is_active &&
            strcmp(server->clients[i].username, message->username) == 0) {
            send_system_message(client->socket_fd, "用户名已存在，请选择其他用户名");
            return;
        }
    }
    
    // 设置用户名
    strncpy(client->username, message->username, MAX_USERNAME_SIZE - 1);
    client->username[MAX_USERNAME_SIZE - 1] = '\0';
    
    // 发送欢迎消息
    char welcome_msg[MAX_MESSAGE_SIZE];
    snprintf(welcome_msg, sizeof(welcome_msg), "欢迎 %s 加入聊天室！", client->username);
    send_system_message(client->socket_fd, welcome_msg);
    
    // 发送用户列表
    send_user_list(server, client->socket_fd);
    
    // 广播用户加入消息
    chat_message_t join_msg;
    snprintf(welcome_msg, sizeof(welcome_msg), "%s 加入了聊天室", client->username);
    create_message(&join_msg, MSG_SYSTEM, "系统", welcome_msg, -1);
    broadcast_message(server, &join_msg, client->socket_fd);
    
    printf("✓ 用户 '%s' 登录成功 (ID: %d)\n", client->username, client_index);
}

void process_chat_message(server_state_t* server, int client_index, const chat_message_t* message) {
    if (!server || client_index < 0 || client_index >= MAX_CLIENTS || !message) return;
    
    client_info_t* client = &server->clients[client_index];
    
    // 检查用户是否已登录
    if (strlen(client->username) == 0) {
        send_system_message(client->socket_fd, "请先登录");
        return;
    }
    
    // 广播聊天消息
    broadcast_message(server, message, client->socket_fd);
    
    printf("💬 [%s]: %s\n", message->username, message->content);
}

void handle_client_message(server_state_t* server, int client_index, chat_statistics_t* stats) {
    if (!server || client_index < 0 || client_index >= MAX_CLIENTS) return;
    
    client_info_t* client = &server->clients[client_index];
    if (!client->is_active) return;
    
    chat_message_t message;
    if (!receive_message(client->socket_fd, &message)) {
        // 接收失败，断开客户端
        disconnect_client(server, client_index, stats);
        return;
    }
    
    // 更新统计信息
    if (stats) update_statistics_message_received(stats);
    
    // 更新心跳时间
    client->last_heartbeat = time(NULL);
    
    // 处理不同类型的消息
    switch (message.type) {
        case MSG_LOGIN:
            process_login_message(server, client_index, &message);
            break;
            
        case MSG_CHAT:
            process_chat_message(server, client_index, &message);
            break;
            
        case MSG_LOGOUT:
            disconnect_client(server, client_index, stats);
            break;
            
        case MSG_HEARTBEAT:
            // 心跳消息，只需要更新时间（已在上面完成）
            break;
            
        default:
            printf("⚠️  收到未知类型消息: %d\n", message.type);
            break;
    }
}

void server_main_loop(server_state_t* server, chat_statistics_t* stats) {
    if (!server) return;
    
    struct timeval timeout;
    int activity;
    
    while (server->is_running) {
        // 清零文件描述符集合
        FD_ZERO(&server->read_fds);
        
        // 添加服务端Socket到集合
        FD_SET(server->server_fd, &server->read_fds);
        server->max_fd = server->server_fd;
        
        // 添加所有活跃的客户端Socket
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (server->clients[i].is_active && server->clients[i].socket_fd >= 0) {
                FD_SET(server->clients[i].socket_fd, &server->read_fds);
                if (server->clients[i].socket_fd > server->max_fd) {
                    server->max_fd = server->clients[i].socket_fd;
                }
            }
        }
        
        // 设置超时时间
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        
        // 等待活动
        activity = select(server->max_fd + 1, &server->read_fds, NULL, NULL, &timeout);
        
        if (activity < 0) {
            print_socket_error("select");
            break;
        }
        
        if (activity == 0) {
            // 超时，继续循环
            continue;
        }
        
        // 检查服务端Socket（新连接）
        if (FD_ISSET(server->server_fd, &server->read_fds)) {
            accept_new_client(server, stats);
        }
        
        // 检查客户端Socket（消息）
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (server->clients[i].is_active && 
                server->clients[i].socket_fd >= 0 &&
                FD_ISSET(server->clients[i].socket_fd, &server->read_fds)) {
                handle_client_message(server, i, stats);
            }
        }
    }
}

/* ================================
 * 客户端函数实现
 * ================================ */

bool init_client(client_state_t* client, const char* server_ip, int port, const char* username) {
    if (!client || !server_ip || !username || !is_valid_port(port)) {
        return false;
    }
    
    // 初始化客户端状态
    memset(client, 0, sizeof(client_state_t));
    client->socket_fd = -1;
    client->is_connected = false;
    client->server_port = port;
    
    // 复制服务器IP和用户名
    strncpy(client->server_ip, server_ip, INET_ADDRSTRLEN - 1);
    client->server_ip[INET_ADDRSTRLEN - 1] = '\0';
    
    strncpy(client->username, username, MAX_USERNAME_SIZE - 1);
    client->username[MAX_USERNAME_SIZE - 1] = '\0';
    
    // 验证IP地址
    if (!is_valid_ip(server_ip)) {
        printf("❌ 无效的服务器IP地址: %s\n", server_ip);
        return false;
    }
    
    printf("✓ 客户端初始化成功\n");
    return true;
}

void cleanup_client(client_state_t* client) {
    if (!client) return;
    
    if (client->socket_fd >= 0) {
        close(client->socket_fd);
        client->socket_fd = -1;
    }
    
    client->is_connected = false;
    printf("✓ 客户端清理完成\n");
}

bool connect_to_server(client_state_t* client) {
    if (!client) return false;
    
    // 创建Socket
    client->socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client->socket_fd < 0) {
        print_socket_error("创建客户端Socket");
        return false;
    }
    
    // 配置服务器地址
    memset(&client->server_addr, 0, sizeof(client->server_addr));
    client->server_addr.sin_family = AF_INET;
    client->server_addr.sin_port = htons(client->server_port);
    
    if (inet_pton(AF_INET, client->server_ip, &client->server_addr.sin_addr) <= 0) {
        printf("❌ IP地址转换失败: %s\n", client->server_ip);
        close(client->socket_fd);
        return false;
    }
    
    // 连接服务器
    if (connect(client->socket_fd, (struct sockaddr*)&client->server_addr, 
                sizeof(client->server_addr)) < 0) {
        print_socket_error("连接服务器");
        close(client->socket_fd);
        return false;
    }
    
    client->is_connected = true;
    client->connect_time = time(NULL);
    
    printf("✓ 已连接到服务器 %s:%d\n", client->server_ip, client->server_port);
    return true;
}

void disconnect_from_server(client_state_t* client) {
    if (!client || !client->is_connected) return;
    
    // 发送退出消息
    send_logout_message(client);
    
    // 关闭连接
    if (client->socket_fd >= 0) {
        close(client->socket_fd);
        client->socket_fd = -1;
    }
    
    client->is_connected = false;
    printf("✓ 已断开与服务器的连接\n");
}

bool send_login_message(client_state_t* client) {
    if (!client || !client->is_connected) return false;
    
    chat_message_t login_msg;
    create_message(&login_msg, MSG_LOGIN, client->username, "登录", 0);
    
    return send_message(client->socket_fd, &login_msg);
}

bool send_chat_message(client_state_t* client, const char* content) {
    if (!client || !client->is_connected || !content) return false;
    
    chat_message_t chat_msg;
    create_message(&chat_msg, MSG_CHAT, client->username, content, 0);
    
    return send_message(client->socket_fd, &chat_msg);
}

bool send_logout_message(client_state_t* client) {
    if (!client || !client->is_connected) return false;
    
    chat_message_t logout_msg;
    create_message(&logout_msg, MSG_LOGOUT, client->username, "退出", 0);
    
    return send_message(client->socket_fd, &logout_msg);
}

void process_received_message(const chat_message_t* message) {
    if (!message) return;
    
    char time_str[64];
    format_timestamp(message->timestamp, time_str, sizeof(time_str));
    
    switch (message->type) {
        case MSG_CHAT:
            printf("[%s] %s: %s\n", time_str, message->username, message->content);
            break;
            
        case MSG_SYSTEM:
            printf("[系统] %s\n", message->content);
            break;
            
        case MSG_USER_LIST:
            printf("[用户列表] %s\n", message->content);
            break;
            
        default:
            printf("[未知消息类型] %s\n", message->content);
            break;
    }
}

void handle_server_message(client_state_t* client, chat_statistics_t* stats) {
    if (!client || !client->is_connected) return;
    
    chat_message_t message;
    if (!receive_message(client->socket_fd, &message)) {
        printf("❌ 接收服务器消息失败，连接可能已断开\n");
        client->is_connected = false;
        return;
    }
    
    // 更新统计信息
    if (stats) update_statistics_message_received(stats);
    
    // 处理消息
    process_received_message(&message);
}

void client_main_loop(client_state_t* client, chat_statistics_t* stats) {
    if (!client || !client->is_connected) return;
    
    fd_set read_fds;
    struct timeval timeout;
    int activity;
    
    printf("进入聊天模式，输入消息后按回车发送，输入 'quit' 退出\n");
    printf("================================================\n");
    
    while (client->is_connected) {
        FD_ZERO(&read_fds);
        FD_SET(0, &read_fds);  // 标准输入
        FD_SET(client->socket_fd, &read_fds);  // Socket
        
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        
        activity = select(client->socket_fd + 1, &read_fds, NULL, NULL, &timeout);
        
        if (activity < 0) {
            print_socket_error("select");
            break;
        }
        
        // 检查服务器消息
        if (FD_ISSET(client->socket_fd, &read_fds)) {
            handle_server_message(client, stats);
        }
        
        // 检查用户输入
        if (FD_ISSET(0, &read_fds)) {
            char input[MAX_MESSAGE_SIZE];
            if (fgets(input, sizeof(input), stdin) != NULL) {
                // 移除换行符
                size_t len = strlen(input);
                if (len > 0 && input[len - 1] == '\n') {
                    input[len - 1] = '\0';
                }
                
                // 检查退出命令
                if (strcmp(input, "quit") == 0) {
                    break;
                }
                
                // 发送聊天消息
                if (strlen(input) > 0) {
                    if (send_chat_message(client, input)) {
                        if (stats) update_statistics_message_sent(stats);
                    }
                }
            }
        }
    }
}

/* ================================
 * 界面显示函数实现
 * ================================ */

void print_welcome_message(void) {
    printf("╔══════════════════════════════════════╗\n");
    printf("║          TCP 聊天系统实验            ║\n");
    printf("║     计算机网络课程教学项目           ║\n");
    printf("╚══════════════════════════════════════╝\n\n");
}

void print_server_info(const server_state_t* server) {
    if (!server) return;
    
    printf("📊 服务端信息:\n");
    printf("   监听端口: %d\n", server->port);
    printf("   运行状态: %s\n", server->is_running ? "运行中" : "已停止");
    printf("   在线客户端: %d/%d\n", server->client_count, MAX_CLIENTS);
    printf("   最大文件描述符: %d\n", server->max_fd);
    printf("\n");
}

void print_client_info(const client_state_t* client) {
    if (!client) return;
    
    printf("📊 客户端信息:\n");
    printf("   用户名: %s\n", client->username);
    printf("   服务器: %s:%d\n", client->server_ip, client->server_port);
    printf("   连接状态: %s\n", client->is_connected ? "已连接" : "未连接");
    if (client->is_connected) {
        char time_str[64];
        format_timestamp(client->connect_time, time_str, sizeof(time_str));
        printf("   连接时间: %s\n", time_str);
    }
    printf("\n");
}

void print_online_users(const server_state_t* server) {
    if (!server) return;
    
    printf("👥 在线用户列表:\n");
    if (server->client_count == 0) {
        printf("   暂无在线用户\n");
    } else {
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (server->clients[i].is_active && strlen(server->clients[i].username) > 0) {
                char time_str[64];
                format_timestamp(server->clients[i].connect_time, time_str, sizeof(time_str));
                printf("   %s (ID: %d, 连接时间: %s)\n", 
                       server->clients[i].username, i, time_str);
            }
        }
    }
    printf("\n");
}

void print_chat_statistics(const chat_statistics_t* stats) {
    if (!stats) return;
    
    printf("📈 聊天统计信息:\n");
    printf("   发送消息数: %d\n", stats->total_messages_sent);
    printf("   接收消息数: %d\n", stats->total_messages_received);
    printf("   总连接数: %d\n", stats->total_clients_connected);
    printf("   当前在线: %d\n", stats->current_online_clients);
    printf("   连接错误: %d\n", stats->connection_errors);
    printf("   消息错误: %d\n", stats->message_errors);
    
    char time_str[64];
    format_timestamp(stats->server_start_time, time_str, sizeof(time_str));
    printf("   启动时间: %s\n", time_str);
    
    time_t current_time = time(NULL);
    int running_time = (int)difftime(current_time, stats->server_start_time);
    printf("   运行时长: %d 秒\n", running_time);
    printf("\n");
}

void print_message_formatted(const chat_message_t* message) {
    if (!message) return;
    
    char time_str[64];
    format_timestamp(message->timestamp, time_str, sizeof(time_str));
    
    switch (message->type) {
        case MSG_LOGIN:
            printf("[%s] 📥 %s 登录\n", time_str, message->username);
            break;
        case MSG_LOGOUT:
            printf("[%s] 📤 %s 退出\n", time_str, message->username);
            break;
        case MSG_CHAT:
            printf("[%s] 💬 %s: %s\n", time_str, message->username, message->content);
            break;
        case MSG_SYSTEM:
            printf("[%s] 🔔 系统: %s\n", time_str, message->content);
            break;
        case MSG_USER_LIST:
            printf("[%s] 👥 %s\n", time_str, message->content);
            break;
        default:
            printf("[%s] ❓ 未知消息类型\n", time_str);
            break;
    }
}

/* ================================
 * 测试函数实现
 * ================================ */

bool test_socket_creation(void) {
    printf("测试Socket创建...\n");
    
    int test_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (test_fd < 0) {
        print_socket_error("测试Socket创建");
        return false;
    }
    
    close(test_fd);
    printf("✓ Socket创建测试通过\n");
    return true;
}

bool test_address_binding(int port) {
    printf("测试地址绑定（端口 %d）...\n", port);
    
    int test_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (test_fd < 0) {
        print_socket_error("创建测试Socket");
        return false;
    }
    
    if (!set_socket_reuseaddr(test_fd)) {
        print_socket_error("设置Socket重用");
        close(test_fd);
        return false;
    }
    
    struct sockaddr_in test_addr;
    memset(&test_addr, 0, sizeof(test_addr));
    test_addr.sin_family = AF_INET;
    test_addr.sin_addr.s_addr = INADDR_ANY;
    test_addr.sin_port = htons(port);
    
    if (bind(test_fd, (struct sockaddr*)&test_addr, sizeof(test_addr)) < 0) {
        print_socket_error("测试地址绑定");
        close(test_fd);
        return false;
    }
    
    close(test_fd);
    printf("✓ 地址绑定测试通过\n");
    return true;
}

bool test_message_serialization(void) {
    printf("测试消息序列化...\n");
    
    // 创建测试消息
    chat_message_t test_msg;
    create_message(&test_msg, MSG_CHAT, "test_user", "Hello, World!", 123);
    
    // 验证消息
    if (!validate_message(&test_msg)) {
        printf("❌ 消息验证失败\n");
        return false;
    }
    
    // 检查字段
    if (test_msg.type != MSG_CHAT ||
        strcmp(test_msg.username, "test_user") != 0 ||
        strcmp(test_msg.content, "Hello, World!") != 0 ||
        test_msg.client_id != 123) {
        printf("❌ 消息内容不匹配\n");
        return false;
    }
    
    printf("✓ 消息序列化测试通过\n");
    return true;
}

void log_server_event(const server_state_t* server, const char* event) {
    if (!server || !event) return;
    
    time_t now = time(NULL);
    char time_str[64];
    format_timestamp(now, time_str, sizeof(time_str));
    
    printf("[%s] 服务端事件: %s (端口: %d, 客户端: %d)\n", 
           time_str, event, server->port, server->client_count);
}

void log_client_event(const client_state_t* client, const char* event) {
    if (!client || !event) return;
    
    time_t now = time(NULL);
    char time_str[64];
    format_timestamp(now, time_str, sizeof(time_str));
    
    printf("[%s] 客户端事件: %s (用户: %s, 服务器: %s:%d)\n", 
           time_str, event, client->username, client->server_ip, client->server_port);
}