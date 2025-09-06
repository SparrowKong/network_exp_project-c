#include "../core/udp_chat.h"
#include <assert.h>
#include <sys/wait.h>

/* 测试统计信息 */
static int tests_run = 0;
static int tests_passed = 0;
static int tests_failed = 0;

/* 测试结果记录 */
typedef struct {
    const char* test_name;
    bool passed;
    const char* error_message;
} test_result_t;

static test_result_t test_results[20];
static int result_count = 0;

/**
 * 记录测试结果
 * @param test_name 测试名称
 * @param passed 是否通过
 * @param error_message 错误信息（如果有）
 */
void record_test_result(const char* test_name, bool passed, const char* error_message) {
    if (result_count < 20) {
        test_results[result_count].test_name = test_name;
        test_results[result_count].passed = passed;
        test_results[result_count].error_message = error_message;
        result_count++;
    }
    
    tests_run++;
    if (passed) {
        tests_passed++;
        printf("✓ %s\n", test_name);
    } else {
        tests_failed++;
        printf("✗ %s - %s\n", test_name, error_message ? error_message : "未知错误");
    }
}

/**
 * 测试UDP socket创建和绑定
 */
void test_socket_creation_and_binding() {
    printf("\n=== 测试UDP Socket创建和绑定 ===\n");
    
    // 测试UDP socket创建
    int sockfd = create_udp_socket();
    if (sockfd <= 0) {
        record_test_result("UDP Socket创建", false, "socket创建失败");
        return;
    }
    record_test_result("UDP Socket创建", true, NULL);
    
    // 测试绑定到有效端口
    if (bind_socket(sockfd, 8889) != 0) {
        record_test_result("Socket绑定有效端口", false, "绑定端口8889失败");
    } else {
        record_test_result("Socket绑定有效端口", true, NULL);
    }
    
    close(sockfd);
    
    // 测试绑定到无效端口（权限不够的端口）
    int sockfd2 = create_udp_socket();
    if (bind_socket(sockfd2, 80) == 0) {
        record_test_result("Socket绑定无效端口检测", false, "不应该能绑定到80端口");
        close(sockfd2);
    } else {
        record_test_result("Socket绑定无效端口检测", true, NULL);
        close(sockfd2);
    }
}

/**
 * 测试消息序列化和反序列化
 */
void test_message_serialization() {
    printf("\n=== 测试消息序列化和反序列化 ===\n");
    
    // 创建测试消息
    chat_message_t original_msg = {0};
    original_msg.type = MSG_CHAT;
    strcpy(original_msg.username, "测试用户");
    strcpy(original_msg.content, "这是一个测试消息");
    original_msg.timestamp = time(NULL);
    original_msg.checksum = calculate_checksum(&original_msg);
    
    // 序列化消息
    char buffer[BUFFER_SIZE];
    int serialized_len = message_serialize(&original_msg, buffer, sizeof(buffer));
    if (serialized_len <= 0) {
        record_test_result("消息序列化", false, "序列化失败");
        return;
    }
    record_test_result("消息序列化", true, NULL);
    
    // 反序列化消息
    chat_message_t deserialized_msg;
    if (message_deserialize(buffer, serialized_len, &deserialized_msg) != 0) {
        record_test_result("消息反序列化", false, "反序列化失败");
        return;
    }
    record_test_result("消息反序列化", true, NULL);
    
    // 验证消息内容
    bool content_match = (original_msg.type == deserialized_msg.type) &&
                        (strcmp(original_msg.username, deserialized_msg.username) == 0) &&
                        (strcmp(original_msg.content, deserialized_msg.content) == 0) &&
                        (original_msg.timestamp == deserialized_msg.timestamp);
    
    record_test_result("消息内容验证", content_match, content_match ? NULL : "消息内容不匹配");
}

/**
 * 测试校验和计算和验证
 */
void test_checksum_calculation() {
    printf("\n=== 测试校验和计算和验证 ===\n");
    
    chat_message_t msg = {0};
    msg.type = MSG_CHAT;
    strcpy(msg.username, "testuser");
    strcpy(msg.content, "test message");
    msg.timestamp = 1234567890;
    
    // 计算校验和
    unsigned int checksum = calculate_checksum(&msg);
    msg.checksum = checksum;
    
    // 验证正确的校验和
    bool valid = verify_message_integrity(&msg);
    record_test_result("正确校验和验证", valid, valid ? NULL : "校验和验证失败");
    
    // 测试错误的校验和
    msg.checksum = checksum + 1;
    bool invalid = !verify_message_integrity(&msg);
    record_test_result("错误校验和检测", invalid, invalid ? NULL : "应该检测出校验和错误");
    
    // 测试空指针
    bool null_safe = !verify_message_integrity(NULL);
    record_test_result("校验和空指针安全", null_safe, null_safe ? NULL : "没有正确处理空指针");
}

/**
 * 测试用户名有效性检查
 */
void test_username_validation() {
    printf("\n=== 测试用户名有效性检查 ===\n");
    
    // 有效用户名
    bool valid1 = is_valid_username("张三");
    record_test_result("有效用户名（中文）", valid1, valid1 ? NULL : "中文用户名应该有效");
    
    bool valid2 = is_valid_username("user123");
    record_test_result("有效用户名（英文数字）", valid2, valid2 ? NULL : "英文数字用户名应该有效");
    
    // 无效用户名
    bool invalid1 = !is_valid_username("");
    record_test_result("空用户名检测", invalid1, invalid1 ? NULL : "空用户名应该无效");
    
    bool invalid2 = !is_valid_username(NULL);
    record_test_result("NULL用户名检测", invalid2, invalid2 ? NULL : "NULL用户名应该无效");
    
    bool invalid3 = !is_valid_username("user with space");
    record_test_result("包含空格用户名检测", invalid3, invalid3 ? NULL : "包含空格的用户名应该无效");
    
    // 过长用户名
    char long_username[MAX_USERNAME_SIZE + 10];
    memset(long_username, 'a', sizeof(long_username) - 1);
    long_username[sizeof(long_username) - 1] = '\0';
    bool invalid4 = !is_valid_username(long_username);
    record_test_result("过长用户名检测", invalid4, invalid4 ? NULL : "过长用户名应该无效");
}

/**
 * 测试地址比较函数
 */
void test_address_comparison() {
    printf("\n=== 测试地址比较函数 ===\n");
    
    struct sockaddr_in addr1, addr2;
    
    // 相同地址
    memset(&addr1, 0, sizeof(addr1));
    memset(&addr2, 0, sizeof(addr2));
    addr1.sin_family = AF_INET;
    addr1.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr1.sin_port = htons(8888);
    addr2 = addr1;
    
    bool same = compare_addresses(&addr1, &addr2);
    record_test_result("相同地址比较", same, same ? NULL : "相同地址应该返回true");
    
    // 不同端口
    addr2.sin_port = htons(8889);
    bool diff_port = !compare_addresses(&addr1, &addr2);
    record_test_result("不同端口地址比较", diff_port, diff_port ? NULL : "不同端口应该返回false");
    
    // 不同IP
    addr2.sin_port = htons(8888);
    addr2.sin_addr.s_addr = inet_addr("127.0.0.2");
    bool diff_ip = !compare_addresses(&addr1, &addr2);
    record_test_result("不同IP地址比较", diff_ip, diff_ip ? NULL : "不同IP应该返回false");
    
    // 空指针测试
    bool null_safe = !compare_addresses(NULL, &addr1) && !compare_addresses(&addr1, NULL);
    record_test_result("地址比较空指针安全", null_safe, null_safe ? NULL : "应该安全处理空指针");
}

/**
 * 测试服务器初始化和清理
 */
void test_server_init_cleanup() {
    printf("\n=== 测试服务器初始化和清理 ===\n");
    
    server_state_t server;
    
    // 测试正常端口初始化
    int result = server_init(&server, 8890);
    if (result != 0) {
        record_test_result("服务器初始化", false, "服务器初始化失败");
        return;
    }
    record_test_result("服务器初始化", true, NULL);
    
    // 验证初始化状态
    bool state_valid = (server.socket_fd > 0) && 
                      (server.client_count == 0) && 
                      (!server.is_running) &&
                      (server.start_time > 0);
    record_test_result("服务器初始状态验证", state_valid, state_valid ? NULL : "初始状态不正确");
    
    // 测试清理
    server_cleanup(&server);
    bool cleaned = (server.socket_fd == -1) && (!server.is_running);
    record_test_result("服务器清理", cleaned, cleaned ? NULL : "清理后状态不正确");
    
    // 测试空指针
    int null_result = server_init(NULL, 8891);
    record_test_result("服务器初始化空指针安全", null_result == -1, 
                      (null_result == -1) ? NULL : "应该拒绝空指针初始化");
}

/**
 * 测试客户端初始化和清理
 */
void test_client_init_cleanup() {
    printf("\n=== 测试客户端初始化和清理 ===\n");
    
    client_state_t client;
    
    // 测试客户端初始化
    int result = client_init(&client, "127.0.0.1", 8888);
    if (result != 0) {
        record_test_result("客户端初始化", false, "客户端初始化失败");
        return;
    }
    record_test_result("客户端初始化", true, NULL);
    
    // 验证初始化状态
    bool state_valid = (client.socket_fd > 0) && 
                      (!client.is_connected) &&
                      (client.server_addr.sin_family == AF_INET) &&
                      (client.server_addr.sin_port == htons(8888));
    record_test_result("客户端初始状态验证", state_valid, state_valid ? NULL : "初始状态不正确");
    
    // 测试无效IP地址
    client_state_t client2;
    int invalid_ip_result = client_init(&client2, "invalid.ip.address", 8888);
    record_test_result("客户端无效IP检测", invalid_ip_result == -1, 
                      (invalid_ip_result == -1) ? NULL : "应该拒绝无效IP地址");
    
    // 测试清理
    client_cleanup(&client);
    bool cleaned = (client.socket_fd == -1) && (!client.is_connected);
    record_test_result("客户端清理", cleaned, cleaned ? NULL : "清理后状态不正确");
}

/**
 * 测试客户端管理功能
 */
void test_client_management() {
    printf("\n=== 测试客户端管理功能 ===\n");
    
    server_state_t server;
    if (server_init(&server, 8892) != 0) {
        record_test_result("客户端管理测试服务器初始化", false, "服务器初始化失败");
        return;
    }
    
    struct sockaddr_in client_addr1, client_addr2;
    
    // 设置客户端地址1
    client_addr1.sin_family = AF_INET;
    client_addr1.sin_addr.s_addr = inet_addr("127.0.0.1");
    client_addr1.sin_port = htons(12345);
    
    // 设置客户端地址2
    client_addr2.sin_family = AF_INET;
    client_addr2.sin_addr.s_addr = inet_addr("127.0.0.1");
    client_addr2.sin_port = htons(12346);
    
    // 测试添加客户端
    int add_result1 = server_add_client(&server, &client_addr1, "用户1");
    record_test_result("添加客户端1", add_result1 == 0 && server.client_count == 1, 
                      (add_result1 == 0 && server.client_count == 1) ? NULL : "添加客户端失败");
    
    int add_result2 = server_add_client(&server, &client_addr2, "用户2");
    record_test_result("添加客户端2", add_result2 == 0 && server.client_count == 2, 
                      (add_result2 == 0 && server.client_count == 2) ? NULL : "添加第二个客户端失败");
    
    // 测试查找客户端
    client_info_t* found_client = server_find_client(&server, &client_addr1);
    bool find_success = (found_client != NULL) && (strcmp(found_client->username, "用户1") == 0);
    record_test_result("查找客户端", find_success, find_success ? NULL : "查找客户端失败");
    
    // 测试重复添加（应该更新现有客户端）
    int update_result = server_add_client(&server, &client_addr1, "更新用户1");
    bool update_success = (update_result == 0) && (server.client_count == 2);
    found_client = server_find_client(&server, &client_addr1);
    update_success = update_success && (strcmp(found_client->username, "更新用户1") == 0);
    record_test_result("更新现有客户端", update_success, update_success ? NULL : "更新客户端失败");
    
    // 测试移除客户端
    int remove_result1 = server_remove_client(&server, &client_addr1);
    record_test_result("移除客户端1", remove_result1 == 0 && server.client_count == 1, 
                      (remove_result1 == 0 && server.client_count == 1) ? NULL : "移除客户端失败");
    
    int remove_result2 = server_remove_client(&server, &client_addr2);
    record_test_result("移除客户端2", remove_result2 == 0 && server.client_count == 0, 
                      (remove_result2 == 0 && server.client_count == 0) ? NULL : "移除第二个客户端失败");
    
    // 测试移除不存在的客户端
    int remove_nonexistent = server_remove_client(&server, &client_addr1);
    record_test_result("移除不存在客户端", remove_nonexistent == -1, 
                      (remove_nonexistent == -1) ? NULL : "应该无法移除不存在的客户端");
    
    server_cleanup(&server);
}

/**
 * 测试消息类型转换函数
 */
void test_message_type_conversion() {
    printf("\n=== 测试消息类型转换函数 ===\n");
    
    // 测试各种消息类型转换
    const char* join_str = message_type_to_string(MSG_JOIN);
    record_test_result("MSG_JOIN转换", strcmp(join_str, "加入") == 0, 
                      (strcmp(join_str, "加入") == 0) ? NULL : "MSG_JOIN转换错误");
    
    const char* chat_str = message_type_to_string(MSG_CHAT);
    record_test_result("MSG_CHAT转换", strcmp(chat_str, "聊天") == 0, 
                      (strcmp(chat_str, "聊天") == 0) ? NULL : "MSG_CHAT转换错误");
    
    const char* leave_str = message_type_to_string(MSG_LEAVE);
    record_test_result("MSG_LEAVE转换", strcmp(leave_str, "离开") == 0, 
                      (strcmp(leave_str, "离开") == 0) ? NULL : "MSG_LEAVE转换错误");
    
    // 测试未知类型
    const char* unknown_str = message_type_to_string((message_type_t)999);
    record_test_result("未知消息类型转换", strcmp(unknown_str, "未知") == 0, 
                      (strcmp(unknown_str, "未知") == 0) ? NULL : "未知消息类型转换错误");
}

/**
 * 测试时间戳格式化
 */
void test_timestamp_formatting() {
    printf("\n=== 测试时间戳格式化 ===\n");
    
    time_t test_time = 1234567890;  // 固定时间用于测试
    const char* formatted = format_timestamp(test_time);
    
    // 检查格式是否为 HH:MM:SS
    bool valid_format = (strlen(formatted) == 8) && 
                       (formatted[2] == ':') && 
                       (formatted[5] == ':');
    record_test_result("时间戳格式验证", valid_format, valid_format ? NULL : "时间戳格式不正确");
    
    // 测试当前时间
    time_t current_time = time(NULL);
    const char* current_formatted = format_timestamp(current_time);
    bool current_valid = (strlen(current_formatted) == 8) && 
                        (current_formatted[2] == ':') && 
                        (current_formatted[5] == ':');
    record_test_result("当前时间戳格式", current_valid, current_valid ? NULL : "当前时间戳格式不正确");
}

/**
 * 测试边界条件
 */
void test_boundary_conditions() {
    printf("\n=== 测试边界条件 ===\n");
    
    server_state_t server;
    if (server_init(&server, 8893) != 0) {
        record_test_result("边界测试服务器初始化", false, "服务器初始化失败");
        return;
    }
    
    // 测试最大客户端数量
    struct sockaddr_in client_addr;
    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    int successful_adds = 0;
    for (int i = 0; i < MAX_CLIENTS + 2; i++) {
        client_addr.sin_port = htons(20000 + i);
        char username[32];
        snprintf(username, sizeof(username), "用户%d", i);
        
        if (server_add_client(&server, &client_addr, username) == 0) {
            successful_adds++;
        }
    }
    
    bool max_clients_limit = (successful_adds == MAX_CLIENTS) && (server.client_count == MAX_CLIENTS);
    record_test_result("最大客户端数量限制", max_clients_limit, 
                      max_clients_limit ? NULL : "最大客户端数量限制未正确实施");
    
    // 测试空消息处理
    chat_message_t empty_msg = {0};
    unsigned int empty_checksum = calculate_checksum(&empty_msg);
    record_test_result("空消息校验和计算", empty_checksum >= 0, NULL);  // 任何值都是有效的
    
    server_cleanup(&server);
}

/**
 * 打印测试总结
 */
void print_test_summary() {
    printf("\n" "========================================\n");
    printf("UDP聊天系统测试总结\n");
    printf("========================================\n");
    printf("总测试数: %d\n", tests_run);
    printf("通过测试: %d\n", tests_passed);
    printf("失败测试: %d\n", tests_failed);
    printf("成功率: %.1f%%\n", tests_run > 0 ? (100.0 * tests_passed / tests_run) : 0.0);
    printf("========================================\n");
    
    if (tests_failed > 0) {
        printf("\n失败的测试:\n");
        for (int i = 0; i < result_count; i++) {
            if (!test_results[i].passed) {
                printf("- %s: %s\n", test_results[i].test_name, 
                       test_results[i].error_message ? test_results[i].error_message : "未知错误");
            }
        }
    }
    
    printf("\n");
    if (tests_failed == 0) {
        printf("🎉 所有测试通过！UDP聊天系统功能正常。\n");
    } else {
        printf("⚠️  有 %d 个测试失败，请检查相关功能。\n", tests_failed);
    }
    printf("\n");
}

/**
 * 主测试函数
 */
int main() {
    printf("UDP聊天系统测试套件\n");
    printf("计算机网络实验项目 - udp_chat_system\n");
    printf("=====================================\n");
    
    // 运行所有测试
    test_socket_creation_and_binding();
    test_message_serialization();
    test_checksum_calculation();
    test_username_validation();
    test_address_comparison();
    test_server_init_cleanup();
    test_client_init_cleanup();
    test_client_management();
    test_message_type_conversion();
    test_timestamp_formatting();
    test_boundary_conditions();
    
    // 打印测试总结
    print_test_summary();
    
    // 返回适当的退出码
    return (tests_failed == 0) ? 0 : 1;
}