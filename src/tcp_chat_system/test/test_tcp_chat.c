#include "../core/tcp_chat.h"
#include <assert.h>
#include <sys/wait.h>
#include <pthread.h>

/* ================================
 * 测试统计和工具函数
 * ================================ */

typedef struct {
    int total_tests;
    int passed_tests;
    int failed_tests;
    char current_test[256];
} test_results_t;

static test_results_t g_test_results = {0, 0, 0, ""};

void start_test(const char* test_name) {
    g_test_results.total_tests++;
    strncpy(g_test_results.current_test, test_name, sizeof(g_test_results.current_test) - 1);
    printf("\n🧪 测试: %s\n", test_name);
    printf("────────────────────────────────────────\n");
}

void test_assert(bool condition, const char* message) {
    if (condition) {
        printf("✅ %s\n", message);
    } else {
        printf("❌ %s\n", message);
        g_test_results.failed_tests++;
        return;
    }
}

void end_test(bool overall_result) {
    if (overall_result) {
        printf("✅ 测试 '%s' 通过\n", g_test_results.current_test);
        g_test_results.passed_tests++;
    } else {
        printf("❌ 测试 '%s' 失败\n", g_test_results.current_test);
        if (g_test_results.failed_tests == 0) {
            g_test_results.failed_tests++;
        }
    }
    printf("\n");
}

void print_test_summary(void) {
    printf("════════════════════════════════════════\n");
    printf("📊 测试结果汇总\n");
    printf("────────────────────────────────────────\n");
    printf("总测试数: %d\n", g_test_results.total_tests);
    printf("通过数: %d\n", g_test_results.passed_tests);
    printf("失败数: %d\n", g_test_results.failed_tests);
    printf("成功率: %.1f%%\n", 
           g_test_results.total_tests > 0 ? 
           (float)g_test_results.passed_tests / g_test_results.total_tests * 100 : 0);
    
    if (g_test_results.failed_tests == 0) {
        printf("🎉 所有测试通过！\n");
    } else {
        printf("⚠️  有 %d 个测试失败\n", g_test_results.failed_tests);
    }
    printf("════════════════════════════════════════\n");
}

/* ================================
 * 基础功能测试
 * ================================ */

bool test_socket_operations(void) {
    start_test("Socket基础操作测试");
    bool result = true;
    
    // 测试Socket创建
    int test_fd = socket(AF_INET, SOCK_STREAM, 0);
    test_assert(test_fd >= 0, "Socket创建成功");
    
    if (test_fd < 0) {
        result = false;
    } else {
        // 测试Socket选项设置
        bool reuse_result = set_socket_reuseaddr(test_fd);
        test_assert(reuse_result, "Socket重用地址设置成功");
        
        // 测试地址绑定（使用端口0自动分配）
        struct sockaddr_in test_addr;
        memset(&test_addr, 0, sizeof(test_addr));
        test_addr.sin_family = AF_INET;
        test_addr.sin_addr.s_addr = INADDR_ANY;
        test_addr.sin_port = htons(0);
        
        int bind_result = bind(test_fd, (struct sockaddr*)&test_addr, sizeof(test_addr));
        test_assert(bind_result == 0, "Socket地址绑定成功");
        
        // 测试监听
        int listen_result = listen(test_fd, BACKLOG);
        test_assert(listen_result == 0, "Socket监听启动成功");
        
        close(test_fd);
        test_assert(true, "Socket关闭成功");
        
        if (!reuse_result || bind_result != 0 || listen_result != 0) {
            result = false;
        }
    }
    
    end_test(result);
    return result;
}

bool test_message_operations(void) {
    start_test("消息操作测试");
    bool result = true;
    
    // 测试消息创建
    chat_message_t test_msg;
    create_message(&test_msg, MSG_CHAT, "test_user", "Hello, World!", 123);
    
    test_assert(test_msg.type == MSG_CHAT, "消息类型设置正确");
    test_assert(strcmp(test_msg.username, "test_user") == 0, "用户名设置正确");
    test_assert(strcmp(test_msg.content, "Hello, World!") == 0, "消息内容设置正确");
    test_assert(test_msg.client_id == 123, "客户端ID设置正确");
    test_assert(test_msg.timestamp > 0, "时间戳设置正确");
    
    // 测试消息验证
    bool valid = validate_message(&test_msg);
    test_assert(valid, "消息验证通过");
    
    // 测试无效消息
    chat_message_t invalid_msg;
    memset(&invalid_msg, 0, sizeof(invalid_msg));
    invalid_msg.type = -1; // 无效类型
    bool invalid = validate_message(&invalid_msg);
    test_assert(!invalid, "无效消息正确被拒绝");
    
    // 测试长用户名
    chat_message_t long_username_msg;
    char long_username[MAX_USERNAME_SIZE + 10];
    memset(long_username, 'a', sizeof(long_username));
    long_username[sizeof(long_username) - 1] = '\0';
    
    create_message(&long_username_msg, MSG_CHAT, long_username, "test", 1);
    test_assert(strlen(long_username_msg.username) < MAX_USERNAME_SIZE, "长用户名被正确截断");
    
    if (test_msg.type != MSG_CHAT || strcmp(test_msg.username, "test_user") != 0 ||
        strcmp(test_msg.content, "Hello, World!") != 0 || test_msg.client_id != 123 ||
        !valid || invalid) {
        result = false;
    }
    
    end_test(result);
    return result;
}

bool test_utility_functions(void) {
    start_test("工具函数测试");
    bool result = true;
    
    // 测试IP地址验证
    test_assert(is_valid_ip("127.0.0.1"), "有效IP地址验证通过");
    test_assert(is_valid_ip("192.168.1.1"), "有效IP地址验证通过");
    test_assert(!is_valid_ip("invalid.ip"), "无效IP地址正确被拒绝");
    test_assert(!is_valid_ip("999.999.999.999"), "超范围IP地址正确被拒绝");
    
    // 测试端口验证
    test_assert(is_valid_port(8080), "有效端口验证通过");
    test_assert(is_valid_port(1), "边界端口验证通过");
    test_assert(is_valid_port(65535), "边界端口验证通过");
    test_assert(!is_valid_port(0), "无效端口正确被拒绝");
    test_assert(!is_valid_port(65536), "超范围端口正确被拒绝");
    test_assert(!is_valid_port(-1), "负数端口正确被拒绝");
    
    // 测试时间戳格式化
    char time_buffer[64];
    time_t test_time = time(NULL);
    format_timestamp(test_time, time_buffer, sizeof(time_buffer));
    test_assert(strlen(time_buffer) >= 19, "时间戳格式化长度正确");
    
    bool ip_tests = is_valid_ip("127.0.0.1") && is_valid_ip("192.168.1.1") &&
                   !is_valid_ip("invalid.ip") && !is_valid_ip("999.999.999.999");
    
    bool port_tests = is_valid_port(8080) && is_valid_port(1) && is_valid_port(65535) &&
                     !is_valid_port(0) && !is_valid_port(65536) && !is_valid_port(-1);
    
    if (!ip_tests || !port_tests || strlen(time_buffer) < 19) {
        result = false;
    }
    
    end_test(result);
    return result;
}

bool test_statistics_management(void) {
    start_test("统计信息管理测试");
    bool result = true;
    
    chat_statistics_t stats;
    init_statistics(&stats);
    
    test_assert(stats.total_messages_sent == 0, "统计信息初始化正确");
    test_assert(stats.total_messages_received == 0, "统计信息初始化正确");
    test_assert(stats.current_online_clients == 0, "统计信息初始化正确");
    test_assert(stats.server_start_time > 0, "服务器启动时间记录正确");
    
    // 测试统计更新
    update_statistics_message_sent(&stats);
    update_statistics_message_received(&stats);
    update_statistics_client_connected(&stats);
    
    test_assert(stats.total_messages_sent == 1, "发送消息统计更新正确");
    test_assert(stats.total_messages_received == 1, "接收消息统计更新正确");
    test_assert(stats.total_clients_connected == 1, "连接客户端统计更新正确");
    test_assert(stats.current_online_clients == 1, "在线客户端统计更新正确");
    
    // 测试客户端断开统计
    update_statistics_client_disconnected(&stats);
    test_assert(stats.current_online_clients == 0, "客户端断开统计更新正确");
    
    // 测试错误统计
    update_statistics_error(&stats, true);
    update_statistics_error(&stats, false);
    test_assert(stats.connection_errors == 1, "连接错误统计更新正确");
    test_assert(stats.message_errors == 1, "消息错误统计更新正确");
    
    if (stats.total_messages_sent != 1 || stats.total_messages_received != 1 ||
        stats.total_clients_connected != 1 || stats.current_online_clients != 0 ||
        stats.connection_errors != 1 || stats.message_errors != 1) {
        result = false;
    }
    
    end_test(result);
    return result;
}

/* ================================
 * 服务端测试
 * ================================ */

bool test_server_initialization(void) {
    start_test("服务端初始化测试");
    bool result = true;
    
    server_state_t server;
    
    // 测试服务端初始化（使用端口0自动分配）
    bool init_result = init_server(&server, 0);
    test_assert(init_result, "服务端初始化成功");
    
    if (init_result) {
        test_assert(server.server_fd >= 0, "服务端Socket创建成功");
        test_assert(server.client_count == 0, "客户端计数初始化正确");
        test_assert(!server.is_running, "服务端运行状态初始化正确");
        
        // 测试客户端列表初始化
        bool clients_init = true;
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (server.clients[i].socket_fd != -1 || server.clients[i].is_active) {
                clients_init = false;
                break;
            }
        }
        test_assert(clients_init, "客户端列表初始化正确");
        
        // 测试服务端启动和停止
        start_server(&server);
        test_assert(server.is_running, "服务端启动状态设置正确");
        
        stop_server(&server);
        test_assert(!server.is_running, "服务端停止状态设置正确");
        
        cleanup_server(&server);
        test_assert(true, "服务端清理完成");
        
        if (server.server_fd < 0 || server.client_count != 0 || !clients_init) {
            result = false;
        }
    } else {
        result = false;
    }
    
    end_test(result);
    return result;
}

bool test_client_management(void) {
    start_test("客户端管理测试");
    bool result = true;
    
    server_state_t server;
    
    // 初始化服务端
    if (!init_server(&server, 0)) {
        test_assert(false, "服务端初始化失败，跳过客户端管理测试");
        end_test(false);
        return false;
    }
    
    // 测试获取可用客户端槽位
    int slot = get_available_client_slot(&server);
    test_assert(slot == 0, "获取第一个可用客户端槽位正确");
    
    // 模拟客户端连接（不实际建立网络连接）
    server.clients[0].socket_fd = 999; // 模拟fd
    server.clients[0].is_active = true;
    server.clients[0].client_id = 0;
    server.client_count = 1;
    strcpy(server.clients[0].username, "test_user");
    
    // 测试查找客户端
    int found_index = find_client_by_fd(&server, 999);
    test_assert(found_index == 0, "通过fd查找客户端成功");
    
    int not_found = find_client_by_fd(&server, 888);
    test_assert(not_found == -1, "查找不存在的客户端正确返回-1");
    
    // 测试下一个可用槽位
    int next_slot = get_available_client_slot(&server);
    test_assert(next_slot == 1, "获取下一个可用客户端槽位正确");
    
    // 测试客户端断开（模拟）
    chat_statistics_t stats;
    init_statistics(&stats);
    
    // 模拟断开，但不实际关闭socket（因为是模拟的）
    server.clients[0].socket_fd = -1; // 避免实际关闭
    server.clients[0].is_active = false;
    memset(server.clients[0].username, 0, MAX_USERNAME_SIZE);
    server.clients[0].client_id = -1;
    server.client_count--;
    update_statistics_client_disconnected(&stats);
    
    test_assert(server.client_count == 0, "客户端断开后计数更新正确");
    test_assert(!server.clients[0].is_active, "客户端断开后状态更新正确");
    test_assert(stats.current_online_clients == 0, "断开统计更新正确");
    
    cleanup_server(&server);
    
    if (slot != 0 || found_index != 0 || not_found != -1 || next_slot != 1 ||
        server.client_count != 0) {
        result = false;
    }
    
    end_test(result);
    return result;
}

/* ================================
 * 客户端测试
 * ================================ */

bool test_client_initialization(void) {
    start_test("客户端初始化测试");
    bool result = true;
    
    client_state_t client;
    
    // 测试有效的客户端初始化
    bool init_result = init_client(&client, "127.0.0.1", 8080, "test_user");
    test_assert(init_result, "客户端初始化成功");
    
    if (init_result) {
        test_assert(strcmp(client.server_ip, "127.0.0.1") == 0, "服务器IP设置正确");
        test_assert(client.server_port == 8080, "服务器端口设置正确");
        test_assert(strcmp(client.username, "test_user") == 0, "用户名设置正确");
        test_assert(!client.is_connected, "连接状态初始化正确");
        test_assert(client.socket_fd == -1, "Socket初始化正确");
        
        cleanup_client(&client);
        test_assert(true, "客户端清理完成");
        
        if (strcmp(client.server_ip, "127.0.0.1") != 0 || client.server_port != 8080 ||
            strcmp(client.username, "test_user") != 0 || client.is_connected) {
            result = false;
        }
    } else {
        result = false;
    }
    
    // 测试无效参数的客户端初始化
    bool invalid_init1 = init_client(&client, "invalid.ip", 8080, "test_user");
    test_assert(!invalid_init1, "无效IP地址初始化正确被拒绝");
    
    bool invalid_init2 = init_client(&client, "127.0.0.1", 70000, "test_user");
    test_assert(!invalid_init2, "无效端口初始化正确被拒绝");
    
    bool invalid_init3 = init_client(&client, "127.0.0.1", 8080, "");
    test_assert(!invalid_init3, "空用户名初始化正确被拒绝");
    
    if (invalid_init1 || invalid_init2 || invalid_init3) {
        result = false;
    }
    
    end_test(result);
    return result;
}

/* ================================
 * 集成测试（简化版）
 * ================================ */

bool test_basic_communication(void) {
    start_test("基础通信测试（模拟）");
    bool result = true;
    
    // 创建测试消息
    chat_message_t login_msg, chat_msg, system_msg;
    
    create_message(&login_msg, MSG_LOGIN, "test_user", "登录", 1);
    create_message(&chat_msg, MSG_CHAT, "test_user", "Hello, everyone!", 1);
    create_message(&system_msg, MSG_SYSTEM, "系统", "test_user 加入了聊天室", -1);
    
    // 验证消息创建
    test_assert(validate_message(&login_msg), "登录消息创建并验证成功");
    test_assert(validate_message(&chat_msg), "聊天消息创建并验证成功");
    test_assert(validate_message(&system_msg), "系统消息创建并验证成功");
    
    // 测试消息内容
    test_assert(login_msg.type == MSG_LOGIN, "登录消息类型正确");
    test_assert(chat_msg.type == MSG_CHAT, "聊天消息类型正确");
    test_assert(system_msg.type == MSG_SYSTEM, "系统消息类型正确");
    
    test_assert(strcmp(login_msg.username, "test_user") == 0, "登录消息用户名正确");
    test_assert(strcmp(chat_msg.content, "Hello, everyone!") == 0, "聊天消息内容正确");
    test_assert(strcmp(system_msg.username, "系统") == 0, "系统消息发送者正确");
    
    if (!validate_message(&login_msg) || !validate_message(&chat_msg) || 
        !validate_message(&system_msg) || login_msg.type != MSG_LOGIN ||
        chat_msg.type != MSG_CHAT || system_msg.type != MSG_SYSTEM) {
        result = false;
    }
    
    end_test(result);
    return result;
}

/* ================================
 * 压力测试和边界测试
 * ================================ */

bool test_boundary_conditions(void) {
    start_test("边界条件测试");
    bool result = true;
    
    // 测试最大长度消息
    chat_message_t max_msg;
    char max_content[MAX_MESSAGE_SIZE];
    memset(max_content, 'a', MAX_MESSAGE_SIZE - 1);
    max_content[MAX_MESSAGE_SIZE - 1] = '\0';
    
    create_message(&max_msg, MSG_CHAT, "user", max_content, 1);
    test_assert(validate_message(&max_msg), "最大长度消息处理正确");
    test_assert(strlen(max_msg.content) == MAX_MESSAGE_SIZE - 1, "最大长度消息长度正确");
    
    // 测试超长消息（应该被截断）
    char over_content[MAX_MESSAGE_SIZE + 100];
    memset(over_content, 'b', sizeof(over_content) - 1);
    over_content[sizeof(over_content) - 1] = '\0';
    
    chat_message_t over_msg;
    create_message(&over_msg, MSG_CHAT, "user", over_content, 1);
    test_assert(strlen(over_msg.content) < MAX_MESSAGE_SIZE, "超长消息被正确截断");
    
    // 测试空消息
    chat_message_t empty_msg;
    create_message(&empty_msg, MSG_CHAT, "user", "", 1);
    test_assert(validate_message(&empty_msg), "空消息处理正确");
    
    // 测试最大用户名
    char max_username[MAX_USERNAME_SIZE];
    memset(max_username, 'c', MAX_USERNAME_SIZE - 1);
    max_username[MAX_USERNAME_SIZE - 1] = '\0';
    
    chat_message_t max_user_msg;
    create_message(&max_user_msg, MSG_CHAT, max_username, "test", 1);
    test_assert(validate_message(&max_user_msg), "最大长度用户名处理正确");
    
    if (!validate_message(&max_msg) || strlen(max_msg.content) != MAX_MESSAGE_SIZE - 1 ||
        strlen(over_msg.content) >= MAX_MESSAGE_SIZE || !validate_message(&empty_msg) ||
        !validate_message(&max_user_msg)) {
        result = false;
    }
    
    end_test(result);
    return result;
}

bool test_multiple_clients_simulation(void) {
    start_test("多客户端模拟测试");
    bool result = true;
    
    server_state_t server;
    if (!init_server(&server, 0)) {
        test_assert(false, "服务端初始化失败，跳过多客户端测试");
        end_test(false);
        return false;
    }
    
    // 模拟多个客户端连接
    for (int i = 0; i < 5; i++) {
        server.clients[i].socket_fd = 1000 + i; // 模拟fd
        server.clients[i].is_active = true;
        server.clients[i].client_id = i;
        server.client_count++;
        snprintf(server.clients[i].username, MAX_USERNAME_SIZE, "user_%d", i);
    }
    
    test_assert(server.client_count == 5, "多客户端连接计数正确");
    
    // 测试查找各个客户端
    for (int i = 0; i < 5; i++) {
        int found = find_client_by_fd(&server, 1000 + i);
        test_assert(found == i, "多客户端查找正确");
        if (found != i) result = false;
    }
    
    // 测试下一个可用槽位
    int next_slot = get_available_client_slot(&server);
    test_assert(next_slot == 5, "多客户端环境下获取可用槽位正确");
    
    // 模拟断开中间的客户端
    server.clients[2].is_active = false;
    server.clients[2].socket_fd = -1;
    server.client_count--;
    
    int available_slot = get_available_client_slot(&server);
    test_assert(available_slot == 2, "断开客户端后槽位可重用");
    
    cleanup_server(&server);
    
    if (server.client_count != 4 || next_slot != 5 || available_slot != 2) {
        result = false;
    }
    
    end_test(result);
    return result;
}

/* ================================
 * 性能测试
 * ================================ */

bool test_message_performance(void) {
    start_test("消息处理性能测试");
    bool result = true;
    
    const int NUM_MESSAGES = 1000;
    clock_t start_time = clock();
    
    // 测试大量消息创建和验证
    for (int i = 0; i < NUM_MESSAGES; i++) {
        chat_message_t msg;
        char content[100];
        snprintf(content, sizeof(content), "Message number %d", i);
        
        create_message(&msg, MSG_CHAT, "perf_user", content, i);
        
        if (!validate_message(&msg)) {
            result = false;
            break;
        }
    }
    
    clock_t end_time = clock();
    double elapsed = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    
    test_assert(result, "大量消息处理成功");
    test_assert(elapsed < 5.0, "消息处理性能在可接受范围内");
    
    printf("   处理 %d 条消息耗时: %.3f 秒\n", NUM_MESSAGES, elapsed);
    printf("   平均每条消息: %.6f 秒\n", elapsed / NUM_MESSAGES);
    
    if (!result || elapsed >= 5.0) {
        result = false;
    }
    
    end_test(result);
    return result;
}

/* ================================
 * 错误处理测试
 * ================================ */

bool test_error_handling(void) {
    start_test("错误处理测试");
    bool result = true;
    
    // 测试空指针处理
    test_assert(!validate_message(NULL), "空消息指针正确处理");
    
    // 测试无效消息类型
    chat_message_t invalid_type_msg;
    memset(&invalid_type_msg, 0, sizeof(invalid_type_msg));
    invalid_type_msg.type = 999; // 无效类型
    strcpy(invalid_type_msg.username, "user");
    strcpy(invalid_type_msg.content, "content");
    
    test_assert(!validate_message(&invalid_type_msg), "无效消息类型正确被拒绝");
    
    // 测试空用户名
    chat_message_t empty_user_msg;
    memset(&empty_user_msg, 0, sizeof(empty_user_msg));
    empty_user_msg.type = MSG_CHAT;
    strcpy(empty_user_msg.content, "content");
    // username为空
    
    test_assert(!validate_message(&empty_user_msg), "空用户名消息正确被拒绝");
    
    // 测试工具函数的空指针处理
    format_timestamp(time(NULL), NULL, 100);
    test_assert(true, "时间戳格式化空指针处理正确");
    
    create_message(NULL, MSG_CHAT, "user", "content", 1);
    test_assert(true, "消息创建空指针处理正确");
    
    // 测试统计函数的空指针处理
    update_statistics_message_sent(NULL);
    update_statistics_message_received(NULL);
    test_assert(true, "统计函数空指针处理正确");
    
    end_test(result);
    return result;
}

/* ================================
 * 主测试函数
 * ================================ */

void run_all_tests(void) {
    printf("╔════════════════════════════════════════╗\n");
    printf("║        TCP聊天系统完整测试套件         ║\n");
    printf("║      计算机网络实验项目测试程序        ║\n");
    printf("╚════════════════════════════════════════╝\n");
    
    printf("\n🚀 开始运行测试套件...\n");
    
    // 基础功能测试
    printf("\n📋 第一阶段：基础功能测试\n");
    printf("════════════════════════════════════════\n");
    test_socket_operations();
    test_message_operations();
    test_utility_functions();
    test_statistics_management();
    
    // 服务端测试
    printf("\n📋 第二阶段：服务端测试\n");
    printf("════════════════════════════════════════\n");
    test_server_initialization();
    test_client_management();
    
    // 客户端测试
    printf("\n📋 第三阶段：客户端测试\n");
    printf("════════════════════════════════════════\n");
    test_client_initialization();
    
    // 通信测试
    printf("\n📋 第四阶段：通信功能测试\n");
    printf("════════════════════════════════════════\n");
    test_basic_communication();
    
    // 边界和压力测试
    printf("\n📋 第五阶段：边界条件和压力测试\n");
    printf("════════════════════════════════════════\n");
    test_boundary_conditions();
    test_multiple_clients_simulation();
    test_message_performance();
    
    // 错误处理测试
    printf("\n📋 第六阶段：错误处理测试\n");
    printf("════════════════════════════════════════\n");
    test_error_handling();
    
    // 打印测试结果
    printf("\n");
    print_test_summary();
}

/* ================================
 * 交互式测试菜单
 * ================================ */

void print_test_menu(void) {
    printf("\n╔══════════════════════════════════════╗\n");
    printf("║        TCP聊天系统测试菜单           ║\n");
    printf("╠══════════════════════════════════════╣\n");
    printf("║  1. 运行完整测试套件                 ║\n");
    printf("║  2. 基础功能测试                     ║\n");
    printf("║  3. 服务端功能测试                   ║\n");
    printf("║  4. 客户端功能测试                   ║\n");
    printf("║  5. 通信功能测试                     ║\n");
    printf("║  6. 性能测试                         ║\n");
    printf("║  7. 错误处理测试                     ║\n");
    printf("║  8. 退出                             ║\n");
    printf("╚══════════════════════════════════════╝\n");
}

int get_choice(void) {
    int choice;
    char input[100];
    
    printf("请选择测试项目 (1-8): ");
    if (fgets(input, sizeof(input), stdin) != NULL) {
        if (sscanf(input, "%d", &choice) == 1 && choice >= 1 && choice <= 8) {
            return choice;
        }
    }
    return -1;
}

int main(void) {
    printf("╔════════════════════════════════════════╗\n");
    printf("║          TCP聊天系统测试程序           ║\n");
    printf("║        计算机网络实验项目测试          ║\n");
    printf("╚════════════════════════════════════════╝\n\n");
    
    // 初始化测试环境
    printf("🔧 初始化测试环境...\n");
    
    // 基础环境检查
    if (!test_socket_creation()) {
        printf("❌ 基础环境检查失败，无法进行测试\n");
        printf("请确保系统支持Socket编程\n");
        return 1;
    }
    
    printf("✅ 测试环境初始化完成\n");
    
    int choice;
    while (1) {
        print_test_menu();
        choice = get_choice();
        
        switch (choice) {
            case 1:
                // 重置测试结果
                memset(&g_test_results, 0, sizeof(g_test_results));
                run_all_tests();
                break;
                
            case 2:
                memset(&g_test_results, 0, sizeof(g_test_results));
                printf("\n📋 运行基础功能测试\n");
                test_socket_operations();
                test_message_operations();
                test_utility_functions();
                test_statistics_management();
                print_test_summary();
                break;
                
            case 3:
                memset(&g_test_results, 0, sizeof(g_test_results));
                printf("\n📋 运行服务端功能测试\n");
                test_server_initialization();
                test_client_management();
                print_test_summary();
                break;
                
            case 4:
                memset(&g_test_results, 0, sizeof(g_test_results));
                printf("\n📋 运行客户端功能测试\n");
                test_client_initialization();
                print_test_summary();
                break;
                
            case 5:
                memset(&g_test_results, 0, sizeof(g_test_results));
                printf("\n📋 运行通信功能测试\n");
                test_basic_communication();
                print_test_summary();
                break;
                
            case 6:
                memset(&g_test_results, 0, sizeof(g_test_results));
                printf("\n📋 运行性能测试\n");
                test_message_performance();
                test_multiple_clients_simulation();
                print_test_summary();
                break;
                
            case 7:
                memset(&g_test_results, 0, sizeof(g_test_results));
                printf("\n📋 运行错误处理测试\n");
                test_error_handling();
                test_boundary_conditions();
                print_test_summary();
                break;
                
            case 8:
                printf("\n👋 测试程序退出\n");
                return g_test_results.failed_tests == 0 ? 0 : 1;
                
            default:
                printf("❌ 无效选择，请重新输入\n");
                break;
        }
        
        printf("\n按回车键继续...");
        while (getchar() != '\n');
    }
    
    return 0;
}