#include "doip_task_example.h"
#include "doip_lwip_adapter.h"
#include "uds_services.h"
#include "doip_log.h"
#include <string.h>
#include <stdio.h>
#include "debug_print.h"

#include "lwipcfg.h"

#if defined(USING_OS_FREERTOS)
/* FreeRTOS kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#endif /* defined(USING_OS_FREERTOS) */

/* lwIP core includes */
#include "lwip/opt.h"

#include "lwip/sys.h"
#include "lwip/timeouts.h"
#include "lwip/debug.h"
#include "lwip/stats.h"
#include "lwip/init.h"
#include "lwip/tcpip.h"
#include "lwip/netif.h"
#include "lwip/api.h"
#include "lwip/arch.h"

/* lwIP netif includes */
#include "lwip/etharp.h"
#include "netif/ethernet.h"
#include "netifcfg.h"

#include "lwip/tcp.h"
#include "lwip/udp.h"
#include "lwip/dns.h"
#include "lwip/dhcp.h"
#include "lwip/autoip.h"

#ifndef ETHIF_NUMBER
#define ETHIF_NUMBER 1
#endif

/* ethernet interface initialization function */
#ifndef ETHIF_INIT
extern err_t ethif_ethernetif_init(struct netif *netif);
#define ETHIF_INIT ethif_ethernetif_init
#endif

/* Global Variables */
static doip_entity_t g_doip_entity;
static doip_interface_t g_doip_interface;
static uds_context_t g_uds_context;

/* Network interfaces global variables for DoIP */
struct netif doip_network_interfaces[ETHIF_NUMBER];

TaskHandle_t g_doip_entity_task_handle = NULL;
TaskHandle_t g_doip_tester_task_handle = NULL;
SemaphoreHandle_t g_doip_mutex = NULL;

/* Task Implementation */
void doip_entity_task(void *pvParameters)
{
    (void)pvParameters;

    TickType_t last_wake_time;
    const TickType_t cycle_time = pdMS_TO_TICKS(DOIP_ENTITY_CYCLE_TIME_MS);

    DOIP_LOG_INFO("Task", "DoIP Entity task started");
    
    /* Initialize Entity configuration */
    doip_entity_config_t config = {
        .vin = {'W','V','W','Z','Z','Z','1','K','Z','1','A','2','3','4','5','6','7'},
        .eid = {0x00, 0x1B, 0x2C, 0x3D, 0x4E, 0x5F},
        .gid = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
        .logical_address = 0x1000,
        .general_inactivity_time = 300000U,
        .initial_inactivity_time = 2000U,
        .alive_check_time = 500U,
        .max_tester_connections = 2U
    };
    
    /* Initialize DoIP Interface */
    if (doip_interface_init(&g_doip_interface, &g_lwip_net_ops) != DOIP_RESULT_OK) {
        DOIP_LOG_ERROR("Task", "Interface initialization failed");
        vTaskDelete(NULL);
        return;
    }
    
    /* Initialize Entity */
    if (doip_entity_init(&g_doip_entity, &config, &g_doip_interface) != DOIP_RESULT_OK) {
        DOIP_LOG_ERROR("Task", "Entity initialization failed");
        vTaskDelete(NULL);
        return;
    }
    
    /* Start Entity services */
    if (doip_entity_start(&g_doip_entity) != DOIP_RESULT_OK) {
        DOIP_LOG_ERROR("Task", "Entity start failed");
        vTaskDelete(NULL);
        return;
    }
    
    DOIP_LOG_INFO("Task", "Entity ready at address 0x%04X", config.logical_address);

    last_wake_time = xTaskGetTickCount();
    
    /* Main processing loop */
    for (;;) {
        if (doip_lock(pdMS_TO_TICKS(100)) == pdTRUE) {
            /* Process DoIP messages */
            doip_entity_process(&g_doip_entity, doip_entity_uds_rx_handler);
            
            /* Update timers */
            doip_entity_update_timers(&g_doip_entity, DOIP_ENTITY_CYCLE_TIME_MS);
            
            doip_unlock();
        }
        
        /* Wait for next cycle */
        vTaskDelayUntil(&last_wake_time, cycle_time);
    }
}

void doip_entity_uds_rx_handler(
    uint16_t source_addr,
    uint16_t target_addr,
    const uint8_t *data,
    uint32_t length,
    void *user_data)
{
    DOIP_LOG_INFO("UDS", "Request from 0x%04X: SID=0x%02X", source_addr, data[0]);

    /* Process UDS request */
    uds_request_t request = {
        .sid = data[0],
        .data = &data[1],
        .length = length - 1U
    };

    uint8_t response_buffer[256];
    uds_response_t response = {
        .buffer = response_buffer,
        .max_length = sizeof(response_buffer),
        .actual_length = 0U
    };

    if (uds_process_request(&g_uds_context, &request, &response)) {
        /* Send UDS response via DoIP */
        doip_entity_send_diagnostic_response(
            (doip_entity_t*)user_data,
            source_addr,
            response.buffer,
            response.actual_length
        );
    }
}

/* This function initializes all network interfaces
 * For DoIP example with simplified network setup
 */
static void interface_init(void)
{
  for(int i = 0; i < ETHIF_NUMBER; i++)
  {
#if LWIP_IPV4
    ip4_addr_t ipaddr, netmask, gw;
#endif /* LWIP_IPV4 */

#if LWIP_IPV4
    ip4_addr_set_zero(&gw);
    ip4_addr_set_zero(&ipaddr);
    ip4_addr_set_zero(&netmask);
    /* doip_network_interfaces[i] takes static IP addresses for DoIP */
    IP4_ADDR((&gw), 10, 42, 0, 1);
    IP4_ADDR((&ipaddr), 10, 42, 0, 200 + i);
    IP4_ADDR((&netmask), 255, 255, 255, 0);
#endif /* LWIP_IPV4 */

#if NO_SYS
    netif_set_default(netif_add(&doip_network_interfaces[i], &ipaddr, &netmask, &gw, NULL, ETHIF_INIT, netif_input));
#else /* NO_SYS */
    netif_set_default(netif_add(&doip_network_interfaces[i], &ipaddr, &netmask, &gw, NULL, ETHIF_INIT, tcpip_input));
#endif /* NO_SYS */

    netif_set_up(&doip_network_interfaces[i]);
  }
}

void doip_application_init(void)
{
    /* init DoIP lwIP adapter */
    doip_lwip_adapter_init();

    tcpip_init(NULL, NULL);
    netconn_thread_init();

    /* Initialize network interfaces */
    interface_init();

    /* Initialize UDS context */
    uds_init(&g_uds_context);

    /* Create mutex for thread safety */
    g_doip_mutex = xSemaphoreCreateMutex();
    if (g_doip_mutex == NULL) {
        DOIP_LOG_ERROR("Task", "Failed to create mutex");
        while (1); /* Halt on error */
    }

    /* Create DoIP Entity task */
    if (xTaskCreate(
            doip_entity_task,
            DOIP_ENTITY_TASK_NAME,
            DOIP_ENTITY_TASK_STACK_SIZE,
            NULL,
            DOIP_ENTITY_TASK_PRIORITY,
            &g_doip_entity_task_handle) != pdPASS) {
        DOIP_LOG_ERROR("Task", "Failed to create task");
        while (1); /* Halt on error */
    }

  vTaskStartScheduler();

  /* If all is well, the scheduler will now be running, and the following
  line will never be reached.  If the following line does execute, then
  there was insufficient FreeRTOS heap memory available for the idle and/or
  timer tasks to be created.  See the memory management section on the
  FreeRTOS web site for more details. */
  for (;;)
  {
  }


}

BaseType_t doip_lock(TickType_t timeout_ms)
{
    if (g_doip_mutex != NULL) {
        return xSemaphoreTake(g_doip_mutex, timeout_ms);
    }
    return pdFALSE;
}

void doip_unlock(void)
{
    if (g_doip_mutex != NULL) {
        xSemaphoreGive(g_doip_mutex);
    }
}

doip_entity_t* doip_get_entity_instance(void)
{
    return &g_doip_entity;
}

void doip_print_task_info(void)
{
    char task_list[512];
    vTaskList(task_list);
    debug_print("=== FreeRTOS Task List ===\r\n%s\n", task_list);
}
