#ifndef CRC_ALGORITHM_H
#define CRC_ALGORITHM_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>

/* 常量定义 */
#define MAX_DATA_SIZE 4096          // 最大数据大小
#define CRC_TABLE_SIZE 256          // CRC查找表大小
#define MAX_MESSAGE_LEN 1024        // 最大消息长度

/* CRC标准类型枚举 */
typedef enum {
    CRC_8,          // CRC-8, 多项式: 0x07
    CRC_16,         // CRC-16, 多项式: 0x8005 
    CRC_16_CCITT,   // CRC-16-CCITT, 多项式: 0x1021
    CRC_32          // CRC-32, 多项式: 0x04C11DB7
} crc_type_t;

/* CRC算法配置结构体 */
typedef struct {
    crc_type_t type;            // CRC类型
    uint32_t polynomial;        // 生成多项式
    int width;                  // CRC位宽度
    uint32_t initial_value;     // 初始值
    uint32_t final_xor_value;   // 最终异或值
    bool reflect_in;            // 输入数据反转
    bool reflect_out;           // 输出结果反转
    const char* name;           // CRC标准名称
} crc_config_t;

/* CRC计算结果结构体 */
typedef struct {
    uint32_t checksum;          // CRC校验值
    bool has_error;             // 是否检测到错误
    int error_position;         // 错误位置（如果能确定）
    double calculation_time_ms; // 计算耗时（毫秒）
} crc_result_t;

/* CRC算法统计信息 */
typedef struct {
    int calculations_count;     // 计算次数
    int error_detections;       // 检测到的错误次数
    double total_time_ms;       // 总计算时间
    double avg_time_ms;         // 平均计算时间
    int bit_errors_injected;    // 注入的比特错误数量
    int bit_errors_detected;    // 检测到的比特错误数量
} crc_statistics_t;

/* 错误注入配置 */
typedef struct {
    bool enable_error_injection;  // 是否启用错误注入
    double error_probability;     // 错误概率 (0.0-1.0)
    int max_error_bits;           // 最大错误比特数
} error_config_t;

/* CRC查找表 */
typedef struct {
    uint32_t table[CRC_TABLE_SIZE];  // 查找表
    bool is_generated;               // 表是否已生成
} crc_table_t;

/* 全局CRC配置预设 */
extern const crc_config_t CRC_PRESETS[];

/* 函数声明 */

/* 初始化函数 */
void init_crc_config(crc_config_t* config, crc_type_t type);
void init_crc_statistics(crc_statistics_t* stats);
void init_error_config(error_config_t* config);

/* CRC表生成函数 */
void generate_crc_table(crc_table_t* table, const crc_config_t* config);
void print_crc_table(const crc_table_t* table, const crc_config_t* config);

/* 核心CRC计算函数 */
uint32_t calculate_crc_bitwise(const uint8_t* data, size_t length, 
                               const crc_config_t* config);
uint32_t calculate_crc_table(const uint8_t* data, size_t length, 
                             const crc_config_t* config, const crc_table_t* table);

/* 完整CRC计算（包含统计） */
crc_result_t compute_crc_complete(const uint8_t* data, size_t length,
                                  const crc_config_t* config, 
                                  const crc_table_t* table,
                                  crc_statistics_t* stats,
                                  bool use_table_method);

/* 数据验证函数 */
bool verify_crc(const uint8_t* data, size_t length, uint32_t expected_crc,
                const crc_config_t* config, const crc_table_t* table);

/* 错误检测和注入函数 */
void inject_bit_error(uint8_t* data, size_t length, error_config_t* error_config,
                      crc_statistics_t* stats);
bool detect_and_locate_error(const uint8_t* original_data, 
                            const uint8_t* received_data,
                            size_t length, const crc_config_t* config,
                            const crc_table_t* table, int* error_position);

/* 工具函数 */
uint32_t reflect_bits(uint32_t data, int width);
void print_binary(uint32_t value, int width);
void print_hex_data(const uint8_t* data, size_t length);

/* 教学演示函数 */
void demonstrate_crc_step_by_step(const uint8_t* data, size_t length,
                                  const crc_config_t* config);
void show_polynomial_division(const uint8_t* data, size_t length,
                             const crc_config_t* config);

/* 性能比较函数 */
void performance_comparison(const uint8_t* data, size_t length,
                           const crc_config_t* config, 
                           const crc_table_t* table);

/* 信息打印函数 */
void print_crc_config(const crc_config_t* config);
void print_crc_result(const crc_result_t* result, const crc_config_t* config);
void print_statistics(const crc_statistics_t* stats);
void print_error_detection_report(const crc_statistics_t* stats);

/* 字符串转换函数 */
size_t string_to_bytes(const char* str, uint8_t* buffer, size_t buffer_size);
size_t hex_string_to_bytes(const char* hex_str, uint8_t* buffer, size_t buffer_size);
void bytes_to_hex_string(const uint8_t* bytes, size_t length, char* hex_str);

/* 实用工具宏 */
#define CRC_MAX(a, b) ((a) > (b) ? (a) : (b))
#define CRC_MIN(a, b) ((a) < (b) ? (a) : (b))

/* 调试宏 */
#ifdef DEBUG_CRC
#define DEBUG_PRINT(fmt, ...) printf("[DEBUG] " fmt "\n", ##__VA_ARGS__)
#else
#define DEBUG_PRINT(fmt, ...)
#endif

#endif // CRC_ALGORITHM_H