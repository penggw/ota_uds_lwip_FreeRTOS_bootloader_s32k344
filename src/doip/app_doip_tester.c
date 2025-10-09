#include "app_doip_tester.h"
#include <string.h>

static void tester_udp_rx_callback(
    const char *src_ip,
    uint16_t src_port,
    const uint8_t *data,
    uint32_t length,
    void *user_data);

static void tester_tcp_rx_callback(
    int connection_id,
    const uint8_t *data,
    uint32_t length,
    void *user_data);

doip_result_t doip_tester_init(
    doip_tester_t *tester,
    const doip_tester_config_t *config,
    doip_interface_t *interface)
{
    if ((tester == NULL) || (config == NULL) || (interface == NULL)) {
        return DOIP_RESULT_INVALID_PARAM;
    }
    
    (void)memcpy(&tester->config, config, sizeof(doip_tester_config_t));
    tester->interface = interface;
    tester->state = DOIP_TESTER_STATE_IDLE;
    tester->tcp_connection_id = -1;
    tester->routing_activated = false;
    tester->timeout_timer = 0U;
    
    return DOIP_RESULT_OK;
}

doip_result_t doip_tester_start_discovery(doip_tester_t *tester)
{
    uint8_t buffer[16];
    doip_header_t header;
    uint32_t encoded_length = 0U;
    doip_result_t result;
    
    if (tester == NULL) {
        return DOIP_RESULT_INVALID_PARAM;
    }
    
    /* Bind UDP socket */
    result = doip_interface_start_udp(tester->interface, 0U); /* Any port */
    if (result != DOIP_RESULT_OK) {
        return result;
    }
    
    /* Prepare Vehicle Identification Request */
    header.protocol_version = DOIP_PROTOCOL_VERSION_2019;
    header.inverse_protocol_version = DOIP_INVERSE_VERSION_2019;
    header.payload_type = DOIP_PAYLOAD_TYPE_VEHICLE_ID_REQ;
    header.payload_length = 0U;
    
    result = doip_encode_header(&header, buffer, sizeof(buffer));
    if (result != DOIP_RESULT_OK) {
        return result;
    }
    
    encoded_length = 8U;
    
    /* Broadcast discovery request */
    result = doip_interface_udp_broadcast(tester->interface, buffer,
                                         encoded_length,
                                         DOIP_UDP_DISCOVERY_PORT);
    if (result == DOIP_RESULT_OK) {
        tester->state = DOIP_TESTER_STATE_DISCOVERY;
        tester->timeout_timer = tester->config.response_timeout;
    }
    
    return result;
}

doip_result_t doip_tester_connect(
    doip_tester_t *tester,
    const char *entity_ip,
    uint16_t entity_port)
{
    int socket_fd;
    
    if ((tester == NULL) || (entity_ip == NULL)) {
        return DOIP_RESULT_INVALID_PARAM;
    }
    
    socket_fd = tester->interface->net_ops->tcp_connect(entity_ip, entity_port);
    if (socket_fd < 0) {
        return DOIP_RESULT_ERROR;
    }
    
    tester->tcp_connection_id = 0; /* Simplified - should find free slot */
    tester->interface->connections[0].socket_fd = socket_fd;
    tester->interface->connections[0].state = DOIP_CONN_STATE_PENDING_ACTIVATION;
    tester->state = DOIP_TESTER_STATE_CONNECTING;
    
    (void)strncpy(tester->entity.ip_address, entity_ip, sizeof(tester->entity.ip_address) - 1U);
    tester->entity.port = entity_port;
    
    return DOIP_RESULT_OK;
}

doip_result_t doip_tester_activate_routing(doip_tester_t *tester)
{
    doip_routing_activation_req_t request;
    uint8_t buffer[32];
    uint32_t encoded_length;
    doip_result_t result;
    
    if (tester == NULL) {
        return DOIP_RESULT_INVALID_PARAM;
    }
    
    if (tester->tcp_connection_id < 0) {
        return DOIP_RESULT_NOT_READY;
    }
    
    /* Prepare routing activation request */
    request.source_address = tester->config.logical_address;
    request.activation_type = tester->config.activation_type;
    request.reserved = 0U;
    request.oem_specific = 0U;
    
    result = doip_encode_routing_activation_req(&request, buffer,
                                               sizeof(buffer), &encoded_length);
    if (result != DOIP_RESULT_OK) {
        return result;
    }
    
    /* Send activation request */
    result = doip_interface_tcp_send(tester->interface, tester->tcp_connection_id,
                                    buffer, encoded_length);
    if (result == DOIP_RESULT_OK) {
        tester->state = DOIP_TESTER_STATE_ACTIVATING;
        tester->timeout_timer = tester->config.response_timeout;
    }
    
    return result;
}

doip_result_t doip_tester_send_diagnostic(
    doip_tester_t *tester,
    uint16_t target_addr,
    const uint8_t *uds_data,
    uint32_t length)
{
    doip_diagnostic_message_t diag_msg;
    uint8_t buffer[DOIP_MAX_PAYLOAD_SIZE + 8U];
    uint32_t encoded_length;
    doip_result_t result;
    
    if ((tester == NULL) || (uds_data == NULL)) {
        return DOIP_RESULT_INVALID_PARAM;
    }
    
    if (!tester->routing_activated) {
        return DOIP_RESULT_NOT_READY;
    }
    
    /* Prepare diagnostic message */
    diag_msg.source_address = tester->config.logical_address;
    diag_msg.target_address = target_addr;
    diag_msg.user_data = uds_data;
    diag_msg.user_data_length = length;
    
    result = doip_encode_diagnostic_message(&diag_msg, buffer,
                                           sizeof(buffer), &encoded_length);
    if (result != DOIP_RESULT_OK) {
        return result;
    }
    
    return doip_interface_tcp_send(tester->interface, tester->tcp_connection_id,
                                  buffer, encoded_length);
}

static void handle_vehicle_announcement(
    doip_tester_t *tester,
    const char *src_ip,
    uint16_t src_port,
    const uint8_t *payload,
    uint32_t payload_length)
{
    if (tester->state != DOIP_TESTER_STATE_DISCOVERY) {
        return;
    }
    
    if (payload_length < 33U) {
        return;
    }
    
    /* Store discovered entity information */
    (void)strncpy(tester->entity.ip_address, src_ip, sizeof(tester->entity.ip_address) - 1U);
    tester->entity.port = DOIP_TCP_DATA_PORT;
    (void)memcpy(tester->entity.vin, &payload[0], DOIP_VIN_LENGTH);
    tester->entity.logical_address = (uint16_t)((uint16_t)payload[17] << 8) |
                                    (uint16_t)payload[18];
    (void)memcpy(tester->entity.eid, &payload[19], DOIP_EID_LENGTH);
    
    tester->state = DOIP_TESTER_STATE_IDLE;
}

static void handle_routing_activation_response(
    doip_tester_t *tester,
    const uint8_t *payload,
    uint32_t payload_length)
{
    if (tester->state != DOIP_TESTER_STATE_ACTIVATING) {
        return;
    }
    
    if (payload_length < 13U) {
        return;
    }
    
    uint8_t response_code = payload[4];
    
    if ((response_code == DOIP_ROUTING_ACT_RES_SUCCESS) ||
        (response_code == DOIP_ROUTING_ACT_RES_CONFIRM_REQUIRED)) {
        tester->routing_activated = true;
        tester->state = DOIP_TESTER_STATE_READY;
    } else {
        tester->state = DOIP_TESTER_STATE_ERROR;
    }
}

static void handle_diagnostic_message(
    doip_tester_t *tester,
    const uint8_t *payload,
    uint32_t payload_length,
    doip_tester_uds_indication_t uds_indication)
{
    doip_diagnostic_message_t diag_msg;
    
    if (doip_decode_diagnostic_message(payload, payload_length,
            &diag_msg) != DOIP_RESULT_OK) {
        return;
    }
    
    /* Forward to UDS indication callback */
    if (uds_indication != NULL) {
        uds_indication(diag_msg.source_address, diag_msg.user_data,
                      diag_msg.user_data_length, tester->user_data);
    }
}

static void tester_udp_rx_callback(
    const char *src_ip,
    uint16_t src_port,
    const uint8_t *data,
    uint32_t length,
    void *user_data)
{
    doip_tester_t *tester = (doip_tester_t *)user_data;
    doip_header_t header;
    
    if (length < 8U) {
        return;
    }
    
    if (doip_decode_header(data, length, &header) != DOIP_RESULT_OK) {
        return;
    }
    
    if (header.payload_type == DOIP_PAYLOAD_TYPE_VEHICLE_ANNOUNCEMENT) {
        handle_vehicle_announcement(tester, src_ip, src_port,
                                   &data[8], header.payload_length);
    }
}

static void tester_tcp_rx_callback(
    int connection_id,
    const uint8_t *data,
    uint32_t length,
    void *user_data)
{
    doip_tester_t *tester = (doip_tester_t *)user_data;
    doip_header_t header;
    
    if (doip_decode_header(data, length, &header) != DOIP_RESULT_OK) {
        return;
    }
    
    switch (header.payload_type) {
        case DOIP_PAYLOAD_TYPE_ROUTING_ACTIVATION_RES:
            handle_routing_activation_response(tester, &data[8],
                                              header.payload_length);
            break;
        
        case DOIP_PAYLOAD_TYPE_DIAG_MESSAGE:
            handle_diagnostic_message(tester, &data[8],
                                     header.payload_length, NULL);
            break;
        
        case DOIP_PAYLOAD_TYPE_ALIVE_CHECK_REQ:
            /* Should send alive check response */
            break;
        
        default:
            break;
    }
}

doip_result_t doip_tester_process(
    doip_tester_t *tester,
    doip_tester_uds_indication_t uds_indication)
{
    if (tester == NULL) {
        return DOIP_RESULT_INVALID_PARAM;
    }
    
    tester->user_data = (void *)uds_indication;
    
    return doip_interface_process(
        tester->interface,
        tester_udp_rx_callback,
        tester_tcp_rx_callback,
        NULL,
        NULL,
        (void *)tester
    );
}

void doip_tester_update_timers(doip_tester_t *tester, uint32_t elapsed_ms)
{
    if (tester == NULL) {
        return;
    }
    
    if (tester->timeout_timer > 0U) {
        if (tester->timeout_timer > elapsed_ms) {
            tester->timeout_timer -= elapsed_ms;
        } else {
            tester->timeout_timer = 0U;
            
            /* Handle timeout based on state */
            if (tester->state == DOIP_TESTER_STATE_DISCOVERY) {
                tester->state = DOIP_TESTER_STATE_ERROR;
            } else if (tester->state == DOIP_TESTER_STATE_ACTIVATING) {
                tester->state = DOIP_TESTER_STATE_ERROR;
            } else {
                /* Other timeouts */
            }
        }
    }
}

bool doip_tester_is_ready(const doip_tester_t *tester)
{
    if (tester == NULL) {
        return false;
    }
    
    return (tester->state == DOIP_TESTER_STATE_READY) &&
           tester->routing_activated;
}
