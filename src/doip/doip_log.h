#ifndef DOIP_LOG_H
#define DOIP_LOG_H

#include <stdint.h>
#include <stdarg.h>

/* Log Levels */
typedef enum {
    DOIP_LOG_LEVEL_ERROR = 0,
    DOIP_LOG_LEVEL_WARN,
    DOIP_LOG_LEVEL_INFO,
    DOIP_LOG_LEVEL_DEBUG,
    DOIP_LOG_LEVEL_TRACE
} doip_log_level_t;

/* Log Configuration */
typedef struct {
    doip_log_level_t min_level;
    void (*output_func)(const char *message);
    uint8_t timestamp_enabled;
    uint8_t color_enabled;
} doip_log_config_t;

/* Initialize logging system */
void doip_log_init(const doip_log_config_t *config);

/* Log functions */
void doip_log_message(doip_log_level_t level, const char *tag,
                     const char *format, ...);

/* Convenience macros */
#define DOIP_LOG_ERROR(tag, fmt, ...) \
    doip_log_message(DOIP_LOG_LEVEL_ERROR, tag, fmt, ##__VA_ARGS__)

#define DOIP_LOG_WARN(tag, fmt, ...) \
    doip_log_message(DOIP_LOG_LEVEL_WARN, tag, fmt, ##__VA_ARGS__)

#define DOIP_LOG_INFO(tag, fmt, ...) \
    doip_log_message(DOIP_LOG_LEVEL_INFO, tag, fmt, ##__VA_ARGS__)

#define DOIP_LOG_DEBUG(tag, fmt, ...) \
    doip_log_message(DOIP_LOG_LEVEL_DEBUG, tag, fmt, ##__VA_ARGS__)

#define DOIP_LOG_TRACE(tag, fmt, ...) \
    doip_log_message(DOIP_LOG_LEVEL_TRACE, tag, fmt, ##__VA_ARGS__)

/* Hex dump utility */
void doip_log_hex_dump(doip_log_level_t level, const char *tag,
                      const uint8_t *data, uint32_t length);

#endif /* DOIP_LOG_H */
