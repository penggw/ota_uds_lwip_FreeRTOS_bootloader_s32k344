#ifndef DOIP_TASK_EXAMPLE_H
#define DOIP_TASK_EXAMPLE_H

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "app_doip_entity.h"
#include "app_doip_tester.h"
#include "doip_interface.h"

/**
 * @file doip_task_example.h
 * @brief FreeRTOS Task Integration Example for DoIP Protocol Stack
 *
 * This file provides example FreeRTOS task implementations for both
 * DoIP Entity (vehicle gateway) and Tester (diagnostic tool) roles.
 */

/* Task Configuration */
#define DOIP_ENTITY_TASK_PRIORITY       (tskIDLE_PRIORITY + 3)
#define DOIP_ENTITY_TASK_STACK_SIZE     (2048)
#define DOIP_ENTITY_TASK_NAME           "DoIP_Entity"

#define DOIP_TESTER_TASK_PRIORITY       (tskIDLE_PRIORITY + 3)
#define DOIP_TESTER_TASK_STACK_SIZE     (2048)
#define DOIP_TESTER_TASK_NAME           "DoIP_Tester"

#define DOIP_NETWORK_RX_TASK_PRIORITY   (tskIDLE_PRIORITY + 4)
#define DOIP_NETWORK_RX_TASK_STACK_SIZE (1024)
#define DOIP_NETWORK_RX_TASK_NAME       "DoIP_NetRX"

/* Task Cycle Times */
#define DOIP_ENTITY_CYCLE_TIME_MS       10
#define DOIP_TESTER_CYCLE_TIME_MS       10
#define DOIP_NETWORK_CYCLE_TIME_MS      5

/**
 * @brief DoIP Entity Task Handle
 */
extern TaskHandle_t g_doip_entity_task_handle;

/**
 * @brief DoIP Tester Task Handle
 */
extern TaskHandle_t g_doip_tester_task_handle;

/**
 * @brief Mutex for thread-safe access to DoIP resources
 */
extern SemaphoreHandle_t g_doip_mutex;

/**
 * @brief Initialize DoIP application with FreeRTOS tasks
 *
 * Creates and starts DoIP Entity task, network processing task,
 * and initializes synchronization primitives.
 */
void doip_application_init(void);

/**
 * @brief DoIP Entity Task Function
 *
 * Main task for DoIP Entity (vehicle gateway) implementation.
 * Handles vehicle announcement, routing activation, and diagnostic messages.
 *
 * @param pvParameters Task parameters (unused)
 */
void doip_entity_task(void *pvParameters);

/**
 * @brief DoIP Tester Task Function
 *
 * Main task for DoIP Tester (diagnostic tool) implementation.
 * Handles vehicle discovery, connection, and diagnostic requests.
 *
 * @param pvParameters Task parameters (unused)
 */
void doip_tester_task(void *pvParameters);

/**
 * @brief Network Reception Task
 *
 * High-priority task for processing incoming network packets
 * with minimal latency.
 *
 * @param pvParameters Task parameters (unused)
 */
void doip_network_rx_task(void *pvParameters);

/**
 * @brief UDS Diagnostic Request Handler (Entity side)
 *
 * Callback function called when Entity receives a diagnostic request.
 *
 * @param source_addr Source logical address (Tester)
 * @param target_addr Target logical address (Entity)
 * @param data UDS diagnostic data
 * @param length Data length in bytes
 * @param user_data User-defined context pointer
 */
void doip_entity_uds_rx_handler(
    uint16_t source_addr,
    uint16_t target_addr,
    const uint8_t *data,
    uint32_t length,
    void *user_data
);

/**
 * @brief UDS Diagnostic Response Handler (Tester side)
 *
 * Callback function called when Tester receives a diagnostic response.
 *
 * @param source_addr Source logical address (Entity)
 * @param data UDS diagnostic response data
 * @param length Data length in bytes
 * @param user_data User-defined context pointer
 */
void doip_tester_uds_indication_handler(
    uint16_t source_addr,
    const uint8_t *data,
    uint32_t length,
    void *user_data
);

/**
 * @brief Get DoIP Entity instance
 * @return Pointer to global Entity instance
 */
doip_entity_t* doip_get_entity_instance(void);

/**
 * @brief Get DoIP Tester instance
 * @return Pointer to global Tester instance
 */
doip_tester_t* doip_get_tester_instance(void);

/**
 * @brief Lock DoIP resources (thread-safe access)
 * @param timeout_ms Timeout in milliseconds (portMAX_DELAY for infinite)
 * @return pdTRUE if lock acquired, pdFALSE on timeout
 */
BaseType_t doip_lock(TickType_t timeout_ms);

/**
 * @brief Unlock DoIP resources
 */
void doip_unlock(void);

/**
 * @brief Suspend DoIP processing tasks
 */
void doip_suspend_tasks(void);

/**
 * @brief Resume DoIP processing tasks
 */
void doip_resume_tasks(void);

/**
 * @brief Get DoIP task statistics
 *
 * @param entity_cpu_usage Output: Entity task CPU usage (0-100%)
 * @param tester_cpu_usage Output: Tester task CPU usage (0-100%)
 * @param network_cpu_usage Output: Network task CPU usage (0-100%)
 */
void doip_get_task_statistics(
    uint8_t *entity_cpu_usage,
    uint8_t *tester_cpu_usage,
    uint8_t *network_cpu_usage
);

/**
 * @brief Print DoIP task information to console
 */
void doip_print_task_info(void);

#endif /* DOIP_TASK_EXAMPLE_H */
