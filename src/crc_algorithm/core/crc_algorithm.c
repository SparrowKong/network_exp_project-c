#include "crc_algorithm.h"

/* CRC标准预设配置 */
const crc_config_t CRC_PRESETS[] = {
    // CRC-8: 多项式 x^8 + x^2 + x^1 + 1 = 0x07
    {CRC_8, 0x07, 8, 0x00, 0x00, false, false, "CRC-8"},
    
    // CRC-16: 多项式 x^16 + x^15 + x^2 + 1 = 0x8005
    {CRC_16, 0x8005, 16, 0x0000, 0x0000, true, true, "CRC-16"},
    
    // CRC-16-CCITT: 多项式 x^16 + x^12 + x^5 + 1 = 0x1021
    {CRC_16_CCITT, 0x1021, 16, 0xFFFF, 0x0000, false, false, "CRC-16-CCITT"},
    
    // CRC-32: 多项式 x^32 + x^26 + x^23 + ... + 1 = 0x04C11DB7
    {CRC_32, 0x04C11DB7, 32, 0xFFFFFFFF, 0xFFFFFFFF, true, true, "CRC-32"}
};

/* 初始化CRC配置 */
void init_crc_config(crc_config_t* config, crc_type_t type) {
    if (config == NULL || type > CRC_32) return;
    *config = CRC_PRESETS[type];
}

/* 初始化CRC统计信息 */
void init_crc_statistics(crc_statistics_t* stats) {
    if (stats == NULL) return;
    stats->calculations_count = 0;
    stats->error_detections = 0;
    stats->total_time_ms = 0.0;
    stats->avg_time_ms = 0.0;
    stats->bit_errors_injected = 0;
    stats->bit_errors_detected = 0;
}

/* 初始化错误配置 */
void init_error_config(error_config_t* config) {
    if (config == NULL) return;
    config->enable_error_injection = false;
    config->error_probability = 0.01; // 1% 错误概率
    config->max_error_bits = 2;
}

/* 位反转函数 - 用于处理反射输入输出 */
uint32_t reflect_bits(uint32_t data, int width) {
    uint32_t reflection = 0;
    for (int i = 0; i < width; i++) {
        if ((data >> i) & 1) {
            reflection |= (1 << (width - 1 - i));
        }
    }
    return reflection;
}

/* 生成CRC查找表 */
void generate_crc_table(crc_table_t* table, const crc_config_t* config) {
    if (table == NULL || config == NULL) return;
    
    printf("正在生成 %s 的CRC查找表...\n", config->name);
    
    uint32_t polynomial = config->polynomial;
    if (config->width < 32) {
        polynomial <<= (32 - config->width);
    }
    
    for (int i = 0; i < CRC_TABLE_SIZE; i++) {
        uint32_t crc = i;
        if (config->reflect_in) {
            crc = reflect_bits(crc, 8);
        }
        
        if (config->width < 32) {
            crc <<= (32 - 8);
        } else {
            crc <<= (config->width - 8);
        }
        
        for (int j = 0; j < 8; j++) {
            if (config->width < 32) {
                if (crc & 0x80000000) {
                    crc = (crc << 1) ^ polynomial;
                } else {
                    crc <<= 1;
                }
            } else {
                if (crc & (1 << (config->width - 1))) {
                    crc = (crc << 1) ^ polynomial;
                } else {
                    crc <<= 1;
                }
            }
        }
        
        if (config->width < 32) {
            crc >>= (32 - config->width);
        }
        
        if (config->reflect_in) {
            crc = reflect_bits(crc, config->width);
        }
        
        table->table[i] = crc & ((1ULL << config->width) - 1);
    }
    
    table->is_generated = true;
    printf("CRC查找表生成完成！\n");
}

/* 打印CRC查找表（用于教学演示） */
void print_crc_table(const crc_table_t* table, const crc_config_t* config) {
    if (table == NULL || config == NULL || !table->is_generated) return;
    
    printf("\n=== %s CRC查找表 ===\n", config->name);
    printf("索引    CRC值     (十六进制)\n");
    printf("------------------------\n");
    
    for (int i = 0; i < CRC_TABLE_SIZE; i += 8) {
        for (int j = 0; j < 8 && (i + j) < CRC_TABLE_SIZE; j++) {
            if (config->width <= 8) {
                printf("0x%02X: 0x%02X  ", i + j, (unsigned char)table->table[i + j]);
            } else if (config->width <= 16) {
                printf("0x%02X: 0x%04X  ", i + j, (unsigned short)table->table[i + j]);
            } else {
                printf("0x%02X: 0x%08X  ", i + j, table->table[i + j]);
            }
        }
        printf("\n");
        if (i > 64) { // 只显示前几行，避免输出过多
            printf("... (省略其余表项) ...\n");
            break;
        }
    }
    printf("\n");
}

/* 按位计算CRC（教学演示用，展示算法过程） */
uint32_t calculate_crc_bitwise(const uint8_t* data, size_t length, 
                               const crc_config_t* config) {
    if (data == NULL || config == NULL || length == 0) return 0;
    
    uint32_t crc = config->initial_value;
    uint32_t polynomial = config->polynomial;
    
    printf("\n=== 按位CRC计算过程 (%s) ===\n", config->name);
    printf("初始值: 0x");
    print_binary(crc, config->width);
    printf("\n生成多项式: 0x%0*X\n", (config->width + 3) / 4, polynomial);
    printf("\n");
    
    for (size_t i = 0; i < length; i++) {
        uint8_t byte = data[i];
        if (config->reflect_in) {
            byte = reflect_bits(byte, 8);
        }
        
        printf("处理字节 %zu: 0x%02X\n", i, data[i]);
        
        for (int bit = 7; bit >= 0; bit--) {
            bool data_bit = (byte >> bit) & 1;
            bool msb = 0;
            
            if (config->width == 8) {
                msb = (crc >> 7) & 1;
                crc = (crc << 1) & 0xFF;
            } else if (config->width == 16) {
                msb = (crc >> 15) & 1;
                crc = (crc << 1) & 0xFFFF;
            } else if (config->width == 32) {
                msb = (crc >> 31) & 1;
                crc = crc << 1;
            }
            
            if (data_bit) crc |= 1;
            
            if (msb) {
                crc ^= polynomial;
            }
            
            if (i < 2) { // 只显示前两个字节的详细过程
                printf("  位 %d: 数据位=%d, MSB=%d, CRC=0x", 7-bit, data_bit, msb);
                print_binary(crc, config->width);
                printf("\n");
            }
        }
        printf("\n");
    }
    
    if (config->reflect_out) {
        crc = reflect_bits(crc, config->width);
    }
    
    crc ^= config->final_xor_value;
    
    printf("最终CRC值: 0x%0*X\n", (config->width + 3) / 4, crc);
    return crc;
}

/* 使用查找表快速计算CRC */
uint32_t calculate_crc_table(const uint8_t* data, size_t length, 
                             const crc_config_t* config, const crc_table_t* table) {
    if (data == NULL || config == NULL || table == NULL || 
        !table->is_generated || length == 0) return 0;
    
    uint32_t crc = config->initial_value;
    
    for (size_t i = 0; i < length; i++) {
        uint8_t byte = data[i];
        
        if (config->reflect_in) {
            byte = reflect_bits(byte, 8);
        }
        
        if (config->width == 8) {
            crc = table->table[crc ^ byte];
        } else if (config->width == 16) {
            if (config->reflect_in) {
                crc = (crc >> 8) ^ table->table[(crc ^ byte) & 0xFF];
            } else {
                crc = (crc << 8) ^ table->table[((crc >> 8) ^ byte) & 0xFF];
            }
        } else if (config->width == 32) {
            if (config->reflect_in) {
                crc = (crc >> 8) ^ table->table[(crc ^ byte) & 0xFF];
            } else {
                crc = (crc << 8) ^ table->table[((crc >> 24) ^ byte) & 0xFF];
            }
        }
        
        // 确保CRC值在正确的位宽范围内
        if (config->width < 32) {
            crc &= (1ULL << config->width) - 1;
        }
    }
    
    if (config->reflect_out && !config->reflect_in) {
        crc = reflect_bits(crc, config->width);
    }
    
    crc ^= config->final_xor_value;
    
    return crc;
}

/* 完整CRC计算（包含时间统计） */
crc_result_t compute_crc_complete(const uint8_t* data, size_t length,
                                  const crc_config_t* config, 
                                  const crc_table_t* table,
                                  crc_statistics_t* stats,
                                  bool use_table_method) {
    crc_result_t result = {0};
    
    if (data == NULL || config == NULL) {
        result.has_error = true;
        return result;
    }
    
    clock_t start_time = clock();
    
    if (use_table_method && table != NULL && table->is_generated) {
        result.checksum = calculate_crc_table(data, length, config, table);
    } else {
        result.checksum = calculate_crc_bitwise(data, length, config);
    }
    
    clock_t end_time = clock();
    result.calculation_time_ms = ((double)(end_time - start_time)) / CLOCKS_PER_SEC * 1000.0;
    
    if (stats != NULL) {
        stats->calculations_count++;
        stats->total_time_ms += result.calculation_time_ms;
        stats->avg_time_ms = stats->total_time_ms / stats->calculations_count;
    }
    
    result.has_error = false;
    result.error_position = -1;
    
    return result;
}

/* 验证CRC */
bool verify_crc(const uint8_t* data, size_t length, uint32_t expected_crc,
                const crc_config_t* config, const crc_table_t* table) {
    if (data == NULL || config == NULL) return false;
    
    uint32_t calculated_crc = calculate_crc_table(data, length, config, table);
    return calculated_crc == expected_crc;
}

/* 注入比特错误（用于测试错误检测能力） */
void inject_bit_error(uint8_t* data, size_t length, error_config_t* error_config,
                      crc_statistics_t* stats) {
    if (data == NULL || error_config == NULL || length == 0) return;
    if (!error_config->enable_error_injection) return;
    
    srand(time(NULL));
    
    if (((double)rand() / RAND_MAX) > error_config->error_probability) {
        return; // 不注入错误
    }
    
    int error_bits = 1 + (rand() % error_config->max_error_bits);
    
    printf("注入 %d 个比特错误:\n", error_bits);
    
    for (int i = 0; i < error_bits; i++) {
        size_t byte_pos = rand() % length;
        int bit_pos = rand() % 8;
        
        uint8_t old_value = data[byte_pos];
        data[byte_pos] ^= (1 << bit_pos);
        uint8_t new_value = data[byte_pos];
        
        printf("  错误 %d: 字节位置 %zu, 比特位置 %d, 0x%02X -> 0x%02X\n",
               i + 1, byte_pos, bit_pos, old_value, new_value);
        
        if (stats != NULL) {
            stats->bit_errors_injected++;
        }
    }
    printf("\n");
}

/* 检测并定位错误 */
bool detect_and_locate_error(const uint8_t* original_data, 
                            const uint8_t* received_data,
                            size_t length, const crc_config_t* config,
                            const crc_table_t* table, int* error_position) {
    if (original_data == NULL || received_data == NULL || 
        config == NULL || table == NULL) return false;
    
    uint32_t original_crc = calculate_crc_table(original_data, length, config, table);
    uint32_t received_crc = calculate_crc_table(received_data, length, config, table);
    
    if (original_crc == received_crc) {
        return false; // 没有检测到错误
    }
    
    // 尝试定位单比特错误
    if (error_position != NULL) {
        *error_position = -1;
        
        for (size_t byte_pos = 0; byte_pos < length; byte_pos++) {
            if (original_data[byte_pos] != received_data[byte_pos]) {
                uint8_t diff = original_data[byte_pos] ^ received_data[byte_pos];
                
                // 检查是否是单比特错误
                if ((diff & (diff - 1)) == 0) {
                    for (int bit_pos = 0; bit_pos < 8; bit_pos++) {
                        if (diff & (1 << bit_pos)) {
                            *error_position = byte_pos * 8 + bit_pos;
                            break;
                        }
                    }
                    break;
                }
            }
        }
    }
    
    return true; // 检测到错误
}

/* 工具函数：打印二进制 */
void print_binary(uint32_t value, int width) {
    for (int i = width - 1; i >= 0; i--) {
        printf("%d", (value >> i) & 1);
        if (i > 0 && i % 4 == 0) printf(" ");
    }
}

/* 工具函数：打印十六进制数据 */
void print_hex_data(const uint8_t* data, size_t length) {
    if (data == NULL || length == 0) return;
    
    printf("数据内容 (%zu 字节): ", length);
    for (size_t i = 0; i < length; i++) {
        printf("%02X ", data[i]);
        if ((i + 1) % 16 == 0) printf("\n                    ");
    }
    printf("\n");
}

/* 教学演示：逐步展示CRC计算 */
void demonstrate_crc_step_by_step(const uint8_t* data, size_t length,
                                  const crc_config_t* config) {
    if (data == NULL || config == NULL) return;
    
    printf("\n=== CRC算法教学演示 ===\n");
    printf("算法: %s\n", config->name);
    printf("生成多项式: 0x%0*X (二进制: ", (config->width + 3) / 4, config->polynomial);
    print_binary(config->polynomial, config->width + 1);
    printf(")\n");
    printf("位宽: %d 位\n", config->width);
    printf("初始值: 0x%0*X\n", (config->width + 3) / 4, config->initial_value);
    printf("最终异或值: 0x%0*X\n", (config->width + 3) / 4, config->final_xor_value);
    printf("输入反射: %s\n", config->reflect_in ? "是" : "否");
    printf("输出反射: %s\n", config->reflect_out ? "是" : "否");
    printf("\n");
    
    print_hex_data(data, length);
    
    uint32_t crc = calculate_crc_bitwise(data, length, config);
    
    printf("\n=== 算法总结 ===\n");
    printf("计算得到的CRC值: 0x%0*X\n", (config->width + 3) / 4, crc);
    printf("二进制表示: ");
    print_binary(crc, config->width);
    printf("\n");
}

/* 展示多项式除法过程（简化版） */
void show_polynomial_division(const uint8_t* data, size_t length,
                             const crc_config_t* config) {
    if (data == NULL || config == NULL || length == 0) return;
    
    printf("\n=== 多项式除法演示 ===\n");
    printf("这是CRC算法的数学本质：多项式除法\n\n");
    
    printf("1. 将数据看作多项式的系数\n");
    printf("   数据: ");
    for (size_t i = 0; i < CRC_MIN(length, 4); i++) {
        printf("0x%02X ", data[i]);
    }
    if (length > 4) printf("...");
    printf("\n\n");
    
    printf("2. 数据多项式左移 %d 位（相当于乘以 x^%d）\n", config->width, config->width);
    printf("3. 用生成多项式 G(x) = 0x%0*X 进行除法运算\n", 
           (config->width + 3) / 4, config->polynomial);
    printf("4. 余数即为CRC校验值\n\n");
    
    uint32_t crc = calculate_crc_table(data, length, config, NULL);
    printf("计算结果: 余数 = 0x%0*X\n", (config->width + 3) / 4, crc);
}

/* 性能比较：位级算法 vs 查表算法 */
void performance_comparison(const uint8_t* data, size_t length,
                           const crc_config_t* config, 
                           const crc_table_t* table) {
    if (data == NULL || config == NULL || table == NULL) return;
    
    printf("\n=== 算法性能比较 ===\n");
    printf("测试数据长度: %zu 字节\n", length);
    printf("CRC类型: %s\n\n", config->name);
    
    // 测试位级算法
    clock_t start = clock();
    uint32_t crc1 = calculate_crc_bitwise(data, length, config);
    clock_t end = clock();
    double bitwise_time = ((double)(end - start)) / CLOCKS_PER_SEC * 1000.0;
    
    // 测试查表算法
    start = clock();
    uint32_t crc2 = calculate_crc_table(data, length, config, table);
    end = clock();
    double table_time = ((double)(end - start)) / CLOCKS_PER_SEC * 1000.0;
    
    printf("位级算法:\n");
    printf("  计算时间: %.4f 毫秒\n", bitwise_time);
    printf("  CRC结果: 0x%0*X\n", (config->width + 3) / 4, crc1);
    printf("\n查表算法:\n");
    printf("  计算时间: %.4f 毫秒\n", table_time);
    printf("  CRC结果: 0x%0*X\n", (config->width + 3) / 4, crc2);
    printf("\n");
    
    if (bitwise_time > 0) {
        printf("性能提升: %.2fx 倍\n", bitwise_time / table_time);
    }
    
    printf("结果一致性: %s\n", (crc1 == crc2) ? "✓ 一致" : "✗ 不一致");
}

/* 打印CRC配置信息 */
void print_crc_config(const crc_config_t* config) {
    if (config == NULL) return;
    
    printf("\n=== CRC配置信息 ===\n");
    printf("标准名称: %s\n", config->name);
    printf("多项式: 0x%0*X\n", (config->width + 3) / 4, config->polynomial);
    printf("位宽: %d 位\n", config->width);
    printf("初始值: 0x%0*X\n", (config->width + 3) / 4, config->initial_value);
    printf("最终异或: 0x%0*X\n", (config->width + 3) / 4, config->final_xor_value);
    printf("输入反射: %s\n", config->reflect_in ? "是" : "否");
    printf("输出反射: %s\n", config->reflect_out ? "是" : "否");
    printf("\n");
}

/* 打印CRC计算结果 */
void print_crc_result(const crc_result_t* result, const crc_config_t* config) {
    if (result == NULL || config == NULL) return;
    
    printf("=== CRC计算结果 ===\n");
    if (result->has_error) {
        printf("❌ 计算出错\n");
        return;
    }
    
    printf("CRC值: 0x%0*X\n", (config->width + 3) / 4, result->checksum);
    printf("计算时间: %.4f 毫秒\n", result->calculation_time_ms);
    
    if (result->error_position >= 0) {
        printf("错误位置: 第 %d 比特\n", result->error_position);
    }
    printf("\n");
}

/* 打印统计信息 */
void print_statistics(const crc_statistics_t* stats) {
    if (stats == NULL) return;
    
    printf("=== 运行统计 ===\n");
    printf("计算次数: %d\n", stats->calculations_count);
    printf("总计算时间: %.4f 毫秒\n", stats->total_time_ms);
    printf("平均计算时间: %.4f 毫秒\n", stats->avg_time_ms);
    printf("错误检测次数: %d\n", stats->error_detections);
    printf("\n");
}

/* 打印错误检测报告 */
void print_error_detection_report(const crc_statistics_t* stats) {
    if (stats == NULL) return;
    
    printf("=== 错误检测报告 ===\n");
    printf("注入的错误比特数: %d\n", stats->bit_errors_injected);
    printf("检测到的错误比特数: %d\n", stats->bit_errors_detected);
    
    if (stats->bit_errors_injected > 0) {
        double detection_rate = (double)stats->bit_errors_detected / 
                               stats->bit_errors_injected * 100.0;
        printf("错误检测率: %.2f%%\n", detection_rate);
    }
    printf("\n");
}

/* 字符串转字节数组 */
size_t string_to_bytes(const char* str, uint8_t* buffer, size_t buffer_size) {
    if (str == NULL || buffer == NULL || buffer_size == 0) return 0;
    
    size_t len = strlen(str);
    if (len > buffer_size) len = buffer_size;
    
    memcpy(buffer, str, len);
    return len;
}

/* 十六进制字符串转字节数组 */
size_t hex_string_to_bytes(const char* hex_str, uint8_t* buffer, size_t buffer_size) {
    if (hex_str == NULL || buffer == NULL || buffer_size == 0) return 0;
    
    size_t hex_len = strlen(hex_str);
    size_t bytes_len = 0;
    
    for (size_t i = 0; i < hex_len && bytes_len < buffer_size; i += 2) {
        if (i + 1 < hex_len) {
            char hex_byte[3] = {hex_str[i], hex_str[i + 1], '\0'};
            buffer[bytes_len++] = (uint8_t)strtol(hex_byte, NULL, 16);
        }
    }
    
    return bytes_len;
}

/* 字节数组转十六进制字符串 */
void bytes_to_hex_string(const uint8_t* bytes, size_t length, char* hex_str) {
    if (bytes == NULL || hex_str == NULL || length == 0) return;
    
    for (size_t i = 0; i < length; i++) {
        sprintf(hex_str + i * 2, "%02X", bytes[i]);
    }
    hex_str[length * 2] = '\0';
}