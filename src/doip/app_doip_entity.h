#ifndef APP_DOIP_ENTITY_H
#define APP_DOIP_ENTITY_H

#include "doip_protocol.h"
#include "doip_interface.h"

/* Entity Configuration */
typedef struct {
    uint8_t vin[DOIP_VIN_LENGTH];
    uint8_t eid[DOIP_EID_LENGTH];
    uint8_t gid[DOIP_GID_LENGTH];
    uint16_t logical_address;
    uint32_t general_inactivity_time;    /* Default: 5000ms */
    uint32_t initial_inactivity_time;    /* Default: 2000ms */
    uint32_t alive_check_time;           /* Default: 500ms */
    uint8_t max_tester_connections;      /* Default: 1 */
} doip_entity_config_t;

/* Entity Timer Types */
typedef enum {
    DOIP_TIMER_INITIAL_INACTIVITY,
    DOIP_TIMER_GENERAL_INACTIVITY,
    DOIP_TIMER_ALIVE_CHECK
} doip_timer_type_t;

/* UDS Callback - Called when diagnostic message received */
typedef void (*doip_entity_uds_rx_callback_t)(
    uint16_t source_addr,
    uint16_t target_addr,
    const uint8_t *data,
    uint32_t length,
    void *user_data
);

/* Entity Connection Context */
typedef struct {
    int connection_id;
    uint16_t source_address;
    bool is_activated;
    uint32_t initial_inactivity_timer;
    uint32_t general_inactivity_timer;
    uint32_t alive_check_timer;
    bool alive_check_pending;
} doip_entity_connection_t;

/* Entity Context */
typedef struct {
    doip_entity_config_t config;
    doip_interface_t *interface;
    doip_entity_connection_t connections[DOIP_MAX_CONNECTIONS];
    uint32_t announcement_count;
    uint32_t announcement_timer;
    doip_entity_uds_rx_callback_t uds_callback;
    void *user_data;
} doip_entity_t;

/* Function Prototypes */
doip_result_t doip_entity_init(
    doip_entity_t *entity,
    const doip_entity_config_t *config,
    doip_interface_t *interface
);

doip_result_t doip_entity_start(
    doip_entity_t *entity
);

doip_result_t doip_entity_send_vehicle_announcement(
    doip_entity_t *entity
);

doip_result_t doip_entity_process(
    doip_entity_t *entity,
    doip_entity_uds_rx_callback_t uds_callback
);

doip_result_t doip_entity_send_diagnostic_response(
    doip_entity_t *entity,
    uint16_t target_addr,
    const uint8_t *data,
    uint32_t length
);

void doip_entity_update_timers(
    doip_entity_t *entity,
    uint32_t elapsed_ms
);

#endif /* APP_DOIP_ENTITY_H */
