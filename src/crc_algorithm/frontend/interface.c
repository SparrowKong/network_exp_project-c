#include "../core/crc_algorithm.h"

/* 全局变量 */
static crc_statistics_t g_stats;
static crc_table_t g_tables[4]; // 为四种CRC标准分别准备表

/* 函数声明 */
void show_welcome_message(void);
void show_main_menu(void);
void handle_crc_calculation(void);
void handle_error_detection_demo(void);
void handle_algorithm_comparison(void);
void handle_teaching_mode(void);
void handle_batch_testing(void);
void show_statistics_report(void);
int get_user_choice(int min, int max);
void clear_input_buffer(void);
void press_enter_to_continue(void);

/* 主函数 */
int main(void) {
    // 初始化统计信息
    init_crc_statistics(&g_stats);
    
    // 预生成所有CRC表
    printf("正在初始化CRC算法演示系统...\n");
    for (int i = 0; i < 4; i++) {
        crc_config_t config;
        init_crc_config(&config, (crc_type_t)i);
        generate_crc_table(&g_tables[i], &config);
    }
    
    show_welcome_message();
    
    int choice;
    do {
        show_main_menu();
        choice = get_user_choice(0, 7);
        
        switch (choice) {
            case 1:
                handle_crc_calculation();
                break;
            case 2:
                handle_error_detection_demo();
                break;
            case 3:
                handle_algorithm_comparison();
                break;
            case 4:
                handle_teaching_mode();
                break;
            case 5:
                handle_batch_testing();
                break;
            case 6:
                show_statistics_report();
                break;
            case 7:
                printf("\n=== CRC算法帮助信息 ===\n");
                printf("CRC (Cyclic Redundancy Check) 循环冗余校验\n");
                printf("是一种根据网络数据包或计算机文件等数据产生简短固定位数校验码的一种信道编码技术。\n\n");
                printf("主要应用:\n");
                printf("• 网络通信中的错误检测\n");
                printf("• 存储设备的数据完整性验证\n");
                printf("• 文件传输的完整性检查\n\n");
                printf("本演示系统支持的CRC标准:\n");
                printf("• CRC-8: 8位CRC，适用于简单应用\n");
                printf("• CRC-16: 16位CRC，广泛应用于工业控制\n");
                printf("• CRC-16-CCITT: CCITT标准，用于电信\n");
                printf("• CRC-32: 32位CRC，用于以太网、ZIP等\n");
                press_enter_to_continue();
                break;
            case 0:
                printf("\n感谢使用CRC算法演示系统！\n");
                printf("最终统计报告:\n");
                print_statistics(&g_stats);
                break;
            default:
                printf("无效选择，请重试。\n");
        }
    } while (choice != 0);
    
    return 0;
}

/* 显示欢迎信息 */
void show_welcome_message(void) {
    printf("\n");
    printf("╔════════════════════════════════════════╗\n");
    printf("║        CRC算法教学演示系统             ║\n");
    printf("║     Cyclic Redundancy Check Demo      ║\n");
    printf("╠════════════════════════════════════════╣\n");
    printf("║  计算机网络课程 - CRC校验算法实验      ║\n");
    printf("║  支持多种CRC标准和算法演示             ║\n");
    printf("╚════════════════════════════════════════╝\n");
    printf("\n系统初始化完成，欢迎使用！\n\n");
}

/* 显示主菜单 */
void show_main_menu(void) {
    printf("\n");
    printf("┌────────────────────────────────────────┐\n");
    printf("│              主菜单                    │\n");
    printf("├────────────────────────────────────────┤\n");
    printf("│ 1. CRC计算与验证                       │\n");
    printf("│ 2. 错误检测能力演示                    │\n");
    printf("│ 3. 算法性能比较                        │\n");
    printf("│ 4. 教学模式 (详细步骤演示)             │\n");
    printf("│ 5. 批量测试                            │\n");
    printf("│ 6. 统计报告                            │\n");
    printf("│ 7. 帮助信息                            │\n");
    printf("│ 0. 退出程序                            │\n");
    printf("└────────────────────────────────────────┘\n");
    printf("请选择功能 (0-7): ");
}

/* CRC计算与验证 */
void handle_crc_calculation(void) {
    printf("\n=== CRC计算与验证 ===\n");
    
    // 选择CRC类型
    printf("请选择CRC类型:\n");
    printf("1. CRC-8\n");
    printf("2. CRC-16\n");
    printf("3. CRC-16-CCITT\n");
    printf("4. CRC-32\n");
    int crc_choice = get_user_choice(1, 4) - 1;
    
    crc_config_t config;
    init_crc_config(&config, (crc_type_t)crc_choice);
    print_crc_config(&config);
    
    // 选择输入方式
    printf("请选择数据输入方式:\n");
    printf("1. 文本字符串\n");
    printf("2. 十六进制字符串\n");
    int input_choice = get_user_choice(1, 2);
    
    uint8_t data_buffer[MAX_DATA_SIZE];
    size_t data_length = 0;
    char input_buffer[MAX_MESSAGE_LEN];
    
    clear_input_buffer();
    if (input_choice == 1) {
        printf("请输入文本字符串: ");
        if (fgets(input_buffer, sizeof(input_buffer), stdin) != NULL) {
            // 移除换行符
            input_buffer[strcspn(input_buffer, "\n")] = '\0';
            data_length = string_to_bytes(input_buffer, data_buffer, sizeof(data_buffer));
        }
    } else {
        printf("请输入十六进制字符串 (不带0x前缀): ");
        if (fgets(input_buffer, sizeof(input_buffer), stdin) != NULL) {
            input_buffer[strcspn(input_buffer, "\n")] = '\0';
            data_length = hex_string_to_bytes(input_buffer, data_buffer, sizeof(data_buffer));
        }
    }
    
    if (data_length == 0) {
        printf("输入数据为空或格式错误！\n");
        press_enter_to_continue();
        return;
    }
    
    print_hex_data(data_buffer, data_length);
    
    // 计算CRC
    crc_result_t result = compute_crc_complete(data_buffer, data_length, &config, 
                                               &g_tables[crc_choice], &g_stats, true);
    print_crc_result(&result, &config);
    
    // 询问是否验证
    printf("是否进行数据验证测试？(y/n): ");
    clear_input_buffer();
    char verify_choice = getchar();
    if (verify_choice == 'y' || verify_choice == 'Y') {
        bool is_valid = verify_crc(data_buffer, data_length, result.checksum, 
                                   &config, &g_tables[crc_choice]);
        printf("验证结果: %s\n", is_valid ? "✓ 数据完整" : "✗ 数据损坏");
        
        if (is_valid) {
            g_stats.error_detections++;
        }
    }
    
    press_enter_to_continue();
}

/* 错误检测能力演示 */
void handle_error_detection_demo(void) {
    printf("\n=== 错误检测能力演示 ===\n");
    
    // 使用预设测试数据
    const char* test_message = "Hello CRC!";
    uint8_t original_data[MAX_DATA_SIZE];
    uint8_t corrupted_data[MAX_DATA_SIZE];
    size_t data_length = string_to_bytes(test_message, original_data, sizeof(original_data));
    
    printf("测试数据: \"%s\"\n", test_message);
    print_hex_data(original_data, data_length);
    
    // 选择CRC类型
    printf("请选择CRC类型 (1-CRC8, 2-CRC16, 3-CRC16-CCITT, 4-CRC32): ");
    int crc_choice = get_user_choice(1, 4) - 1;
    
    crc_config_t config;
    init_crc_config(&config, (crc_type_t)crc_choice);
    
    // 计算原始数据的CRC
    uint32_t original_crc = calculate_crc_table(original_data, data_length, 
                                                &config, &g_tables[crc_choice]);
    printf("\n原始数据CRC: 0x%0*X\n", (config.width + 3) / 4, original_crc);
    
    // 复制数据并注入错误
    memcpy(corrupted_data, original_data, data_length);
    
    error_config_t error_config;
    init_error_config(&error_config);
    error_config.enable_error_injection = true;
    error_config.error_probability = 1.0; // 100%注入错误
    error_config.max_error_bits = 3;
    
    inject_bit_error(corrupted_data, data_length, &error_config, &g_stats);
    
    printf("损坏后的数据:\n");
    print_hex_data(corrupted_data, data_length);
    
    // 计算损坏数据的CRC
    uint32_t corrupted_crc = calculate_crc_table(corrupted_data, data_length, 
                                                  &config, &g_tables[crc_choice]);
    printf("损坏数据CRC: 0x%0*X\n", (config.width + 3) / 4, corrupted_crc);
    
    // 检测错误
    int error_position = -1;
    bool error_detected = detect_and_locate_error(original_data, corrupted_data, 
                                                   data_length, &config, 
                                                   &g_tables[crc_choice], &error_position);
    
    printf("\n=== 错误检测结果 ===\n");
    printf("错误检测: %s\n", error_detected ? "✓ 检测到错误" : "✗ 未检测到错误");
    if (error_detected) {
        g_stats.bit_errors_detected++;
        if (error_position >= 0) {
            printf("错误位置: 第 %d 比特 (字节 %d, 比特 %d)\n", 
                   error_position, error_position / 8, error_position % 8);
        } else {
            printf("错误位置: 无法精确定位 (可能是多比特错误)\n");
        }
    }
    
    press_enter_to_continue();
}

/* 算法性能比较 */
void handle_algorithm_comparison(void) {
    printf("\n=== 算法性能比较 ===\n");
    
    // 生成测试数据
    size_t test_sizes[] = {16, 64, 256, 1024, 4096};
    int num_sizes = sizeof(test_sizes) / sizeof(test_sizes[0]);
    
    printf("请选择CRC类型 (1-CRC8, 2-CRC16, 3-CRC16-CCITT, 4-CRC32): ");
    int crc_choice = get_user_choice(1, 4) - 1;
    
    crc_config_t config;
    init_crc_config(&config, (crc_type_t)crc_choice);
    
    printf("\n=== %s 性能测试 ===\n", config.name);
    printf("数据大小\t位级算法\t查表算法\t性能提升\n");
    printf("--------\t--------\t--------\t--------\n");
    
    for (int i = 0; i < num_sizes; i++) {
        uint8_t* test_data = malloc(test_sizes[i]);
        if (test_data == NULL) continue;
        
        // 生成随机测试数据
        for (size_t j = 0; j < test_sizes[i]; j++) {
            test_data[j] = rand() % 256;
        }
        
        // 测试位级算法
        clock_t start = clock();
        for (int iter = 0; iter < 100; iter++) { // 多次测试求平均
            calculate_crc_bitwise(test_data, test_sizes[i], &config);
        }
        clock_t end = clock();
        double bitwise_time = ((double)(end - start)) / CLOCKS_PER_SEC * 1000.0 / 100.0;
        
        // 测试查表算法
        start = clock();
        for (int iter = 0; iter < 100; iter++) {
            calculate_crc_table(test_data, test_sizes[i], &config, &g_tables[crc_choice]);
        }
        end = clock();
        double table_time = ((double)(end - start)) / CLOCKS_PER_SEC * 1000.0 / 100.0;
        
        double speedup = (table_time > 0) ? bitwise_time / table_time : 1.0;
        
        printf("%zu 字节\t%.4f ms\t%.4f ms\t%.2fx\n", 
               test_sizes[i], bitwise_time, table_time, speedup);
        
        free(test_data);
    }
    
    printf("\n结论: 查表算法在处理大数据时具有明显的性能优势！\n");
    press_enter_to_continue();
}

/* 教学模式 - 详细步骤演示 */
void handle_teaching_mode(void) {
    printf("\n=== CRC算法教学模式 ===\n");
    
    printf("请选择演示内容:\n");
    printf("1. CRC算法基本原理\n");
    printf("2. 逐步计算演示\n");
    printf("3. 多项式除法演示\n");
    printf("4. 查找表生成过程\n");
    int demo_choice = get_user_choice(1, 4);
    
    // 使用简单的测试数据便于理解
    const char* demo_data = "ABC";
    uint8_t data_buffer[16];
    size_t data_length = string_to_bytes(demo_data, data_buffer, sizeof(data_buffer));
    
    printf("演示数据: \"%s\"\n", demo_data);
    
    // 使用CRC-8便于演示
    crc_config_t config;
    init_crc_config(&config, CRC_8);
    
    switch (demo_choice) {
        case 1:
            printf("\n=== CRC算法基本原理 ===\n");
            printf("CRC算法基于多项式除法运算:\n");
            printf("1. 将输入数据视为多项式的系数\n");
            printf("2. 将数据多项式左移n位 (n为CRC位宽)\n");
            printf("3. 用生成多项式进行模2除法运算\n");
            printf("4. 除法的余数即为CRC校验值\n\n");
            print_crc_config(&config);
            break;
            
        case 2:
            demonstrate_crc_step_by_step(data_buffer, data_length, &config);
            break;
            
        case 3:
            show_polynomial_division(data_buffer, data_length, &config);
            break;
            
        case 4:
            printf("\n=== CRC查找表生成过程 ===\n");
            print_crc_table(&g_tables[0], &config);
            printf("查找表的作用:\n");
            printf("• 预计算所有可能的8位输入对应的CRC值\n");
            printf("• 将O(n*w)的计算复杂度降到O(n)\n");
            printf("• 其中n是数据长度，w是CRC位宽\n");
            break;
    }
    
    press_enter_to_continue();
}

/* 批量测试 */
void handle_batch_testing(void) {
    printf("\n=== 批量测试模式 ===\n");
    
    const char* test_vectors[] = {
        "123456789",
        "The quick brown fox jumps over the lazy dog",
        "CRC",
        "A",
        "",
        "Hello World!",
        "0123456789ABCDEF"
    };
    int num_vectors = sizeof(test_vectors) / sizeof(test_vectors[0]);
    
    printf("将对 %d 组测试数据进行所有CRC类型的计算...\n\n", num_vectors);
    
    // 已知的标准CRC-32测试向量
    uint32_t expected_crc32[] = {
        0xCBF43926,  // "123456789"
        0x414FA339,  // "The quick brown fox..."
        0x784DD132,  // "CRC"
        0xE8B7BE43,  // "A"
        0x00000000,  // ""
        0x1C291CA3,  // "Hello World!"
        0xA3830348   // "0123456789ABCDEF"
    };
    
    printf("测试向量验证:\n");
    printf("%-30s CRC-8  CRC-16 CCITT  CRC-32    验证\n", "数据");
    printf("--------------------------------------------------------\n");
    
    for (int i = 0; i < num_vectors; i++) {
        uint8_t data_buffer[MAX_DATA_SIZE];
        size_t data_length = string_to_bytes(test_vectors[i], data_buffer, sizeof(data_buffer));
        
        printf("%-30s", test_vectors[i][0] ? test_vectors[i] : "(空字符串)");
        
        // 计算各种CRC
        for (int crc_type = 0; crc_type < 4; crc_type++) {
            crc_config_t config;
            init_crc_config(&config, (crc_type_t)crc_type);
            uint32_t crc = calculate_crc_table(data_buffer, data_length, 
                                               &config, &g_tables[crc_type]);
            
            if (crc_type == 0) {
                printf(" %02X    ", crc & 0xFF);
            } else if (crc_type <= 2) {
                printf(" %04X  ", crc & 0xFFFF);
            } else {
                printf(" %08X ", crc);
            }
        }
        
        // CRC-32验证
        crc_config_t crc32_config;
        init_crc_config(&crc32_config, CRC_32);
        uint32_t calculated_crc32 = calculate_crc_table(data_buffer, data_length, 
                                                         &crc32_config, &g_tables[3]);
        
        if (calculated_crc32 == expected_crc32[i]) {
            printf("  ✓");
        } else {
            printf("  ✗");
        }
        printf("\n");
    }
    
    printf("\n批量测试完成！\n");
    press_enter_to_continue();
}

/* 显示统计报告 */
void show_statistics_report(void) {
    printf("\n=== 系统统计报告 ===\n");
    print_statistics(&g_stats);
    print_error_detection_report(&g_stats);
    
    // 显示系统信息
    printf("=== 系统信息 ===\n");
    printf("支持的CRC标准: 4种\n");
    printf("最大数据长度: %d 字节\n", MAX_DATA_SIZE);
    printf("查找表大小: %d 项\n", CRC_TABLE_SIZE);
    printf("内存使用: 约 %.1f KB\n", 
           (sizeof(g_tables) + sizeof(g_stats)) / 1024.0);
    printf("\n");
    
    press_enter_to_continue();
}

/* 获取用户选择 */
int get_user_choice(int min, int max) {
    int choice;
    while (1) {
        if (scanf("%d", &choice) == 1) {
            if (choice >= min && choice <= max) {
                return choice;
            }
        }
        clear_input_buffer();
        printf("输入无效，请输入 %d-%d 之间的数字: ", min, max);
    }
}

/* 清除输入缓冲区 */
void clear_input_buffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

/* 等待用户按回车继续 */
void press_enter_to_continue(void) {
    printf("\n按回车键继续...");
    clear_input_buffer();
    getchar();
}