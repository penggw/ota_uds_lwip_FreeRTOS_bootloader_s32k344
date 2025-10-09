#include "doip_log.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "debug_print.h"

#ifdef USE_FREERTOS
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

static SemaphoreHandle_t log_mutex = NULL;
#endif

static doip_log_config_t g_log_config = {
    .min_level = DOIP_LOG_LEVEL_INFO,
    .output_func = NULL,
    .timestamp_enabled = true,
    .color_enabled = false
};

/* ANSI color codes */
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

static const char* level_strings[] = {
    "ERROR",
    "WARN ",
    "INFO ",
    "DEBUG",
    "TRACE"
};

static const char* level_colors[] = {
    ANSI_COLOR_RED,
    ANSI_COLOR_YELLOW,
    ANSI_COLOR_GREEN,
    ANSI_COLOR_CYAN,
    ANSI_COLOR_RESET
};

void doip_log_init(const doip_log_config_t *config)
{
    if (config != NULL) {
        (void)memcpy(&g_log_config, config, sizeof(doip_log_config_t));
    }
    
#ifdef USE_FREERTOS
    if (log_mutex == NULL) {
        log_mutex = xSemaphoreCreateMutex();
    }
#endif
}

static void default_output(const char *message)
{
    /* Default: output to stdout */
    debug_print("%s", message);
}

void doip_log_message(doip_log_level_t level, const char *tag,
                     const char *format, ...)
{
    if (level > g_log_config.min_level) {
        return;
    }
    
#ifdef USE_FREERTOS
    if (log_mutex != NULL) {
        (void)xSemaphoreTake(log_mutex, portMAX_DELAY);
    }
#endif
    
    char buffer[256];
    int offset = 0;
    
    /* Add color prefix if enabled */
    if (g_log_config.color_enabled) {
        offset += snprintf(&buffer[offset], sizeof(buffer) - (uint32_t)offset,
                          "%s", level_colors[level]);
    }
    
    /* Add timestamp if enabled */
    if (g_log_config.timestamp_enabled) {
#ifdef USE_FREERTOS
        uint32_t tick = xTaskGetTickCount();
        offset += snprintf(&buffer[offset], sizeof(buffer) - (uint32_t)offset,
                          "[%lu] ", tick);
#endif
    }
    
    /* Add level and tag */
    offset += snprintf(&buffer[offset], sizeof(buffer) - (uint32_t)offset,
                      "[%s] [%s] ", level_strings[level], tag);
    
    /* Add message */
    va_list args;
    va_start(args, format);
    offset += vsnprintf(&buffer[offset], sizeof(buffer) - (uint32_t)offset,
                       format, args);
    va_end(args);
    
    /* Add color reset if enabled */
    if (g_log_config.color_enabled) {
        offset += snprintf(&buffer[offset], sizeof(buffer) - (uint32_t)offset,
                          "%s", ANSI_COLOR_RESET);
    }
    
    /* Add newline */
    offset += snprintf(&buffer[offset], sizeof(buffer) - (uint32_t)offset, "\r\n");
    
    /* Output */
    if (g_log_config.output_func != NULL) {
        g_log_config.output_func(buffer);
    } else {
        default_output(buffer);
    }
    
#ifdef USE_FREERTOS
    if (log_mutex != NULL) {
        (void)xSemaphoreGive(log_mutex);
    }
#endif
}

void doip_log_hex_dump(doip_log_level_t level, const char *tag,
                      const uint8_t *data, uint32_t length)
{
    if ((level > g_log_config.min_level) || (data == NULL)) {
        return;
    }
    
    char hex_buffer[128];
    char ascii_buffer[32];
    uint32_t i;
    
    doip_log_message(level, tag, "Hex dump (%lu bytes):", length);
    
    for (i = 0U; i < length; i += 16U) {
        int hex_offset = 0;
        int ascii_offset = 0;
        uint32_t j;
        
        /* Offset */
        hex_offset += snprintf(hex_buffer, sizeof(hex_buffer),
                             "%04lX: ", i);
        
        /* Hex bytes */
        for (j = 0U; j < 16U; j++) {
            if ((i + j) < length) {
                hex_offset += snprintf(&hex_buffer[hex_offset],
                                     sizeof(hex_buffer) - (uint32_t)hex_offset,
                                     "%02X ", data[i + j]);
                
                /* ASCII representation */
                char c = (char)data[i + j];
                if ((c >= 32) && (c <= 126)) {
                    ascii_buffer[ascii_offset++] = c;
                } else {
                    ascii_buffer[ascii_offset++] = '.';
                }
            } else {
                hex_offset += snprintf(&hex_buffer[hex_offset],
                                     sizeof(hex_buffer) - (uint32_t)hex_offset,
                                     "   ");
            }
        }
        
        ascii_buffer[ascii_offset] = '\0';
        
        doip_log_message(level, tag, "%s | %s", hex_buffer, ascii_buffer);
    }
}
