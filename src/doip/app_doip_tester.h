#ifndef APP_DOIP_TESTER_H
#define APP_DOIP_TESTER_H

#include "doip_protocol.h"
#include "doip_interface.h"

/* Activation Types */
#define DOIP_ACTIVATION_TYPE_DEFAULT           0x00U
#define DOIP_ACTIVATION_TYPE_WWH_OBD           0x01U
#define DOIP_ACTIVATION_TYPE_CENTRAL_SECURITY  0xE0U

/* Tester Configuration */
typedef struct {
    uint16_t logical_address;        /* 0x0E00-0x0FFF */
    uint8_t activation_type;
    uint32_t response_timeout;       /* Default: 2000ms */
    uint32_t max_retry_count;
} doip_tester_config_t;

/* Tester State */
typedef enum {
    DOIP_TESTER_STATE_IDLE = 0,
    DOIP_TESTER_STATE_DISCOVERY,
    DOIP_TESTER_STATE_CONNECTING,
    DOIP_TESTER_STATE_ACTIVATING,
    DOIP_TESTER_STATE_READY,
    DOIP_TESTER_STATE_ERROR
} doip_tester_state_t;

/* Discovered Entity Information */
typedef struct {
    char ip_address[16];
    uint16_t port;
    uint8_t vin[DOIP_VIN_LENGTH];
    uint16_t logical_address;
    uint8_t eid[DOIP_EID_LENGTH];
} doip_discovered_entity_t;

/* Tester Context */
typedef struct {
    doip_tester_config_t config;
    doip_interface_t *interface;
    doip_tester_state_t state;
    int tcp_connection_id;
    doip_discovered_entity_t entity;
    bool routing_activated;
    uint32_t timeout_timer;
    void *user_data;
} doip_tester_t;

/* UDS Indication Callback */
typedef void (*doip_tester_uds_indication_t)(
    uint16_t source_addr,
    const uint8_t *data,
    uint32_t length,
    void *user_data
);

/* Function Prototypes */
doip_result_t doip_tester_init(
    doip_tester_t *tester,
    const doip_tester_config_t *config,
    doip_interface_t *interface
);

doip_result_t doip_tester_start_discovery(
    doip_tester_t *tester
);

doip_result_t doip_tester_connect(
    doip_tester_t *tester,
    const char *entity_ip,
    uint16_t entity_port
);

doip_result_t doip_tester_activate_routing(
    doip_tester_t *tester
);

doip_result_t doip_tester_send_diagnostic(
    doip_tester_t *tester,
    uint16_t target_addr,
    const uint8_t *uds_data,
    uint32_t length
);

doip_result_t doip_tester_process(
    doip_tester_t *tester,
    doip_tester_uds_indication_t uds_indication
);

void doip_tester_update_timers(
    doip_tester_t *tester,
    uint32_t elapsed_ms
);

bool doip_tester_is_ready(const doip_tester_t *tester);

#endif /* APP_DOIP_TESTER_H */
