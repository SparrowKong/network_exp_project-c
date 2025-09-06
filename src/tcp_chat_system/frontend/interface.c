#include "../core/tcp_chat.h"
#include <fcntl.h>
#include <pthread.h>

/* ================================
 * 全局变量和信号处理
 * ================================ */

static bool g_running = true;
static server_state_t* g_server = NULL;
static client_state_t* g_client = NULL;

// 信号处理函数
void signal_handler(int signum) {
    printf("\n收到信号 %d，正在优雅关闭...\n", signum);
    g_running = false;
    
    if (g_server) {
        stop_server(g_server);
    }
    
    if (g_client && g_client->is_connected) {
        disconnect_from_server(g_client);
    }
}

/* ================================
 * 用户界面辅助函数
 * ================================ */

void clear_screen(void) {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void press_enter_to_continue(void) {
    printf("\n按回车键继续...");
    while (getchar() != '\n');
}

int get_user_choice(int min_choice, int max_choice) {
    int choice;
    char input[100];
    
    while (1) {
        printf("请选择 (%d-%d): ", min_choice, max_choice);
        
        if (fgets(input, sizeof(input), stdin) != NULL) {
            if (sscanf(input, "%d", &choice) == 1) {
                if (choice >= min_choice && choice <= max_choice) {
                    return choice;
                } else {
                    printf("❌ 选择超出范围，请重新输入\n");
                }
            } else {
                printf("❌ 输入无效，请输入数字\n");
            }
        }
    }
}

bool get_user_string(const char* prompt, char* buffer, size_t buffer_size) {
    printf("%s: ", prompt);
    
    if (fgets(buffer, buffer_size, stdin) != NULL) {
        // 移除换行符
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
        }
        return strlen(buffer) > 0;
    }
    
    return false;
}

int get_port_number(void) {
    int port;
    char input[100];
    
    while (1) {
        printf("请输入端口号 (1-65535, 推荐8080): ");
        
        if (fgets(input, sizeof(input), stdin) != NULL) {
            if (sscanf(input, "%d", &port) == 1) {
                if (is_valid_port(port)) {
                    return port;
                } else {
                    printf("❌ 端口号无效，请输入1-65535之间的数字\n");
                }
            } else {
                printf("❌ 输入无效，请输入数字\n");
            }
        }
    }
}

void print_main_menu(void) {
    printf("\n╔══════════════════════════════════════╗\n");
    printf("║          TCP 聊天系统主菜单          ║\n");
    printf("╠══════════════════════════════════════╣\n");
    printf("║  1. 启动聊天服务器                   ║\n");
    printf("║  2. 连接聊天服务器（客户端）         ║\n");
    printf("║  3. 运行系统测试                     ║\n");
    printf("║  4. 查看系统信息                     ║\n");
    printf("║  5. 退出程序                         ║\n");
    printf("╚══════════════════════════════════════╝\n");
}

void print_server_menu(void) {
    printf("\n╔══════════════════════════════════════╗\n");
    printf("║          服务器管理菜单              ║\n");
    printf("╠══════════════════════════════════════╣\n");
    printf("║  1. 查看服务器状态                   ║\n");
    printf("║  2. 查看在线用户                     ║\n");
    printf("║  3. 查看统计信息                     ║\n");
    printf("║  4. 停止服务器                       ║\n");
    printf("║  5. 返回主菜单                       ║\n");
    printf("╚══════════════════════════════════════╝\n");
}

/* ================================
 * 服务器管理函数
 * ================================ */

void handle_server_management(server_state_t* server, chat_statistics_t* stats) {
    int choice;
    
    while (g_running && server->is_running) {
        print_server_menu();
        choice = get_user_choice(1, 5);
        
        switch (choice) {
            case 1:
                clear_screen();
                printf("📊 服务器状态信息\n");
                printf("════════════════════════════════════════\n");
                print_server_info(server);
                press_enter_to_continue();
                break;
                
            case 2:
                clear_screen();
                printf("👥 在线用户信息\n");
                printf("════════════════════════════════════════\n");
                print_online_users(server);
                press_enter_to_continue();
                break;
                
            case 3:
                clear_screen();
                printf("📈 统计信息\n");
                printf("════════════════════════════════════════\n");
                print_chat_statistics(stats);
                press_enter_to_continue();
                break;
                
            case 4:
                printf("⚠️  确定要停止服务器吗？(y/N): ");
                {
                    char confirm[10];
                    if (fgets(confirm, sizeof(confirm), stdin) != NULL &&
                        (confirm[0] == 'y' || confirm[0] == 'Y')) {
                        stop_server(server);
                        printf("✓ 服务器已停止\n");
                        press_enter_to_continue();
                        return;
                    }
                }
                break;
                
            case 5:
                return;
        }
    }
}

void* server_thread_function(void* arg) {
    server_state_t* server = (server_state_t*)arg;
    chat_statistics_t stats;
    
    init_statistics(&stats);
    
    printf("🚀 服务器线程已启动，开始监听连接...\n");
    server_main_loop(server, &stats);
    
    printf("🔚 服务器线程已退出\n");
    return NULL;
}

void start_chat_server(void) {
    server_state_t server;
    chat_statistics_t stats;
    
    clear_screen();
    printf("🖥️  启动聊天服务器\n");
    printf("════════════════════════════════════════\n");
    
    // 获取端口号
    int port = get_port_number();
    
    // 初始化服务器
    if (!init_server(&server, port)) {
        printf("❌ 服务器初始化失败\n");
        press_enter_to_continue();
        return;
    }
    
    g_server = &server;
    init_statistics(&stats);
    
    // 启动服务器
    start_server(&server);
    
    printf("\n✅ 服务器启动成功！\n");
    printf("监听地址: 0.0.0.0:%d\n", port);
    printf("最大客户端数: %d\n", MAX_CLIENTS);
    printf("\n按 Ctrl+C 可以优雅关闭服务器\n");
    
    press_enter_to_continue();
    
    // 创建服务器线程
    pthread_t server_thread;
    if (pthread_create(&server_thread, NULL, server_thread_function, &server) != 0) {
        printf("❌ 创建服务器线程失败\n");
        cleanup_server(&server);
        press_enter_to_continue();
        return;
    }
    
    // 进入服务器管理界面
    handle_server_management(&server, &stats);
    
    // 停止服务器
    stop_server(&server);
    
    // 等待线程结束
    pthread_join(server_thread, NULL);
    
    // 清理资源
    cleanup_server(&server);
    g_server = NULL;
    
    printf("✓ 服务器已完全关闭\n");
    press_enter_to_continue();
}

/* ================================
 * 客户端管理函数
 * ================================ */

void connect_to_chat_server(void) {
    client_state_t client;
    chat_statistics_t stats;
    char server_ip[INET_ADDRSTRLEN];
    char username[MAX_USERNAME_SIZE];
    int port;
    
    clear_screen();
    printf("💻 连接聊天服务器\n");
    printf("════════════════════════════════════════\n");
    
    // 获取服务器信息
    if (!get_user_string("请输入服务器IP地址 (默认: 127.0.0.1)", server_ip, sizeof(server_ip))) {
        strcpy(server_ip, "127.0.0.1");
    }
    
    port = get_port_number();
    
    if (!get_user_string("请输入用户名", username, sizeof(username))) {
        printf("❌ 用户名不能为空\n");
        press_enter_to_continue();
        return;
    }
    
    // 初始化客户端
    if (!init_client(&client, server_ip, port, username)) {
        printf("❌ 客户端初始化失败\n");
        press_enter_to_continue();
        return;
    }
    
    g_client = &client;
    init_statistics(&stats);
    
    // 连接服务器
    printf("\n🔄 正在连接服务器 %s:%d...\n", server_ip, port);
    
    if (!connect_to_server(&client)) {
        printf("❌ 连接服务器失败\n");
        cleanup_client(&client);
        press_enter_to_continue();
        return;
    }
    
    // 发送登录消息
    if (!send_login_message(&client)) {
        printf("❌ 登录失败\n");
        disconnect_from_server(&client);
        cleanup_client(&client);
        press_enter_to_continue();
        return;
    }
    
    printf("✅ 已成功连接到服务器\n");
    print_client_info(&client);
    
    // 等待一下让登录消息处理完成
    sleep(1);
    
    // 进入聊天主循环
    client_main_loop(&client, &stats);
    
    // 断开连接
    disconnect_from_server(&client);
    cleanup_client(&client);
    g_client = NULL;
    
    printf("\n📊 会话统计:\n");
    print_chat_statistics(&stats);
    
    press_enter_to_continue();
}

/* ================================
 * 测试函数实现
 * ================================ */

void run_system_tests(void) {
    clear_screen();
    printf("🧪 运行系统测试\n");
    printf("════════════════════════════════════════\n");
    
    bool all_tests_passed = true;
    
    // 测试1: Socket创建
    if (!test_socket_creation()) {
        all_tests_passed = false;
    }
    
    // 测试2: 地址绑定
    if (!test_address_binding(0)) { // 使用端口0让系统自动分配
        all_tests_passed = false;
    }
    
    // 测试3: 消息序列化
    if (!test_message_serialization()) {
        all_tests_passed = false;
    }
    
    // 测试4: IP地址验证
    printf("测试IP地址验证...\n");
    if (!is_valid_ip("127.0.0.1") || !is_valid_ip("192.168.1.1") || 
        is_valid_ip("invalid.ip") || is_valid_ip("999.999.999.999")) {
        printf("❌ IP地址验证测试失败\n");
        all_tests_passed = false;
    } else {
        printf("✓ IP地址验证测试通过\n");
    }
    
    // 测试5: 端口验证
    printf("测试端口验证...\n");
    if (!is_valid_port(8080) || !is_valid_port(1) || !is_valid_port(65535) ||
        is_valid_port(0) || is_valid_port(65536) || is_valid_port(-1)) {
        printf("❌ 端口验证测试失败\n");
        all_tests_passed = false;
    } else {
        printf("✓ 端口验证测试通过\n");
    }
    
    // 测试6: 消息创建和验证
    printf("测试消息创建和验证...\n");
    chat_message_t test_msg;
    create_message(&test_msg, MSG_CHAT, "testuser", "Hello, World!", 1);
    
    if (!validate_message(&test_msg) ||
        test_msg.type != MSG_CHAT ||
        strcmp(test_msg.username, "testuser") != 0 ||
        strcmp(test_msg.content, "Hello, World!") != 0 ||
        test_msg.client_id != 1) {
        printf("❌ 消息创建和验证测试失败\n");
        all_tests_passed = false;
    } else {
        printf("✓ 消息创建和验证测试通过\n");
    }
    
    // 测试7: 时间戳格式化
    printf("测试时间戳格式化...\n");
    char time_buffer[64];
    time_t test_time = time(NULL);
    format_timestamp(test_time, time_buffer, sizeof(time_buffer));
    
    if (strlen(time_buffer) < 19) { // 最短格式: "YYYY-MM-DD HH:MM:SS"
        printf("❌ 时间戳格式化测试失败\n");
        all_tests_passed = false;
    } else {
        printf("✓ 时间戳格式化测试通过\n");
    }
    
    // 测试8: 统计信息
    printf("测试统计信息管理...\n");
    chat_statistics_t test_stats;
    init_statistics(&test_stats);
    
    update_statistics_message_sent(&test_stats);
    update_statistics_message_received(&test_stats);
    update_statistics_client_connected(&test_stats);
    
    if (test_stats.total_messages_sent != 1 ||
        test_stats.total_messages_received != 1 ||
        test_stats.total_clients_connected != 1 ||
        test_stats.current_online_clients != 1) {
        printf("❌ 统计信息管理测试失败\n");
        all_tests_passed = false;
    } else {
        printf("✓ 统计信息管理测试通过\n");
    }
    
    // 输出测试结果
    printf("\n════════════════════════════════════════\n");
    if (all_tests_passed) {
        printf("🎉 所有测试通过！系统功能正常\n");
    } else {
        printf("⚠️  部分测试失败，请检查系统配置\n");
    }
    
    press_enter_to_continue();
}

/* ================================
 * 系统信息显示
 * ================================ */

void show_system_info(void) {
    clear_screen();
    printf("ℹ️  系统信息\n");
    printf("════════════════════════════════════════\n");
    
    printf("🔧 系统配置:\n");
    printf("   最大消息长度: %d 字节\n", MAX_MESSAGE_SIZE);
    printf("   最大用户名长度: %d 字符\n", MAX_USERNAME_SIZE);
    printf("   最大客户端数量: %d\n", MAX_CLIENTS);
    printf("   默认端口号: %d\n", DEFAULT_PORT);
    printf("   缓冲区大小: %d 字节\n", BUFFER_SIZE);
    printf("   监听队列长度: %d\n", BACKLOG);
    printf("\n");
    
    printf("📡 支持的消息类型:\n");
    printf("   登录消息 (MSG_LOGIN)\n");
    printf("   退出消息 (MSG_LOGOUT)\n");
    printf("   聊天消息 (MSG_CHAT)\n");
    printf("   用户列表 (MSG_USER_LIST)\n");
    printf("   系统消息 (MSG_SYSTEM)\n");
    printf("   心跳消息 (MSG_HEARTBEAT)\n");
    printf("\n");
    
    printf("🌐 网络功能:\n");
    printf("   ✓ TCP/IP 协议支持\n");
    printf("   ✓ 多客户端并发连接\n");
    printf("   ✓ 消息广播功能\n");
    printf("   ✓ 用户状态管理\n");
    printf("   ✓ 连接错误处理\n");
    printf("   ✓ 统计信息收集\n");
    printf("\n");
    
    printf("🛠️  开发信息:\n");
    printf("   项目名称: TCP聊天系统实验\n");
    printf("   编程语言: C (C99标准)\n");
    printf("   编译器: GCC\n");
    printf("   平台支持: Linux, macOS, Unix\n");
    printf("   架构设计: 客户端/服务端模式\n");
    printf("\n");
    
    printf("📚 教学目标:\n");
    printf("   • 理解TCP/IP协议原理\n");
    printf("   • 掌握Socket编程技术\n");
    printf("   • 学习多进程/多线程处理\n");
    printf("   • 了解网络应用程序设计\n");
    printf("   • 培养网络编程实践能力\n");
    
    press_enter_to_continue();
}

/* ================================
 * 主程序入口
 * ================================ */

int main(void) {
    // 设置信号处理
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    int choice;
    
    // 显示欢迎信息
    clear_screen();
    print_welcome_message();
    
    printf("🔧 系统初始化中...\n");
    
    // 基础系统测试
    printf("正在进行基础系统检查...\n");
    if (!test_socket_creation()) {
        printf("❌ 基础系统检查失败，程序无法正常运行\n");
        printf("请检查系统是否支持Socket编程\n");
        return 1;
    }
    
    printf("✅ 系统检查完成，程序就绪\n");
    sleep(2);
    
    // 主循环
    while (g_running) {
        clear_screen();
        print_welcome_message();
        print_main_menu();
        
        choice = get_user_choice(1, 5);
        
        switch (choice) {
            case 1:
                start_chat_server();
                break;
                
            case 2:
                connect_to_chat_server();
                break;
                
            case 3:
                run_system_tests();
                break;
                
            case 4:
                show_system_info();
                break;
                
            case 5:
                printf("\n👋 感谢使用TCP聊天系统！\n");
                g_running = false;
                break;
        }
    }
    
    printf("程序已退出。\n");
    return 0;
}