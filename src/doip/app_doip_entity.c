#include "app_doip_entity.h"
#include "doip_config.h"
#include <string.h>
#include <stdio.h>
#include "debug_print.h"

static void entity_udp_rx_callback(
    const char *src_ip,
    uint16_t src_port,
    const uint8_t *data,
    uint32_t length,
    void *user_data);

static void entity_tcp_rx_callback(
    int connection_id,
    const uint8_t *data,
    uint32_t length,
    void *user_data);

static void entity_tcp_connected_callback(
    int connection_id,
    void *user_data);

static void entity_tcp_disconnected_callback(
    int connection_id,
    void *user_data);

doip_result_t doip_entity_init(
    doip_entity_t *entity,
    const doip_entity_config_t *config,
    doip_interface_t *interface)
{
    uint32_t i;
    
    if ((entity == NULL) || (config == NULL) || (interface == NULL)) {
        return DOIP_RESULT_INVALID_PARAM;
    }
    
    (void)memcpy(&entity->config, config, sizeof(doip_entity_config_t));
    entity->interface = interface;
    entity->announcement_count = 0U;
    entity->announcement_timer = 0U;
    
    /* Initialize connection contexts */
    for (i = 0U; i < DOIP_MAX_CONNECTIONS; i++) {
        entity->connections[i].connection_id = -1;
        entity->connections[i].source_address = 0U;
        entity->connections[i].is_activated = false;
        entity->connections[i].initial_inactivity_timer = 0U;
        entity->connections[i].general_inactivity_timer = 0U;
        entity->connections[i].alive_check_timer = 0U;
        entity->connections[i].alive_check_pending = false;
    }
    
    return DOIP_RESULT_OK;
}

doip_result_t doip_entity_start(doip_entity_t *entity)
{
    doip_result_t result;
    
    if (entity == NULL) {
        return DOIP_RESULT_INVALID_PARAM;
    }
    
    /* Start UDP for vehicle discovery */
    result = doip_interface_start_udp(entity->interface, DOIP_UDP_DISCOVERY_PORT);
    if (result != DOIP_RESULT_OK) {
        return result;
    }
    
    /* Start TCP server for diagnostic communication */
    result = doip_interface_start_tcp_server(entity->interface, DOIP_TCP_DATA_PORT);
    if (result != DOIP_RESULT_OK) {
        return result;
    }
    
    /* Send initial vehicle announcements (3 times per ISO 13400) */
    entity->announcement_count = 0U;
    entity->announcement_timer = 0U; /* Random 0-500ms in real implementation */
    
    return DOIP_RESULT_OK;
}

doip_result_t doip_entity_send_vehicle_announcement(doip_entity_t *entity)
{
    uint8_t buffer[64];
    uint32_t encoded_length;
    doip_vehicle_id_response_t response;
    doip_result_t result;
    
    if (entity == NULL) {
        return DOIP_RESULT_INVALID_PARAM;
    }
    
    /* Prepare vehicle identification response */
    (void)memcpy(response.vin, entity->config.vin, DOIP_VIN_LENGTH);
    response.logical_address = entity->config.logical_address;
    (void)memcpy(response.eid, entity->config.eid, DOIP_EID_LENGTH);
    (void)memcpy(response.gid, entity->config.gid, DOIP_GID_LENGTH);
    response.further_action_required = 0x00U; /* No further action */
    response.sync_status = 0x00U; /* Complete */
    
    result = doip_encode_vehicle_id_response(&response, buffer,
                                            sizeof(buffer), &encoded_length);
    if (result != DOIP_RESULT_OK) {
        return result;
    }
    
    debug_print("[DOIP TX UDP] Payload type: 0x%04X, length: %u\r\n", DOIP_PAYLOAD_TYPE_VEHICLE_ANNOUNCEMENT, encoded_length);
    /* Broadcast announcement */
    return doip_interface_udp_broadcast(entity->interface, buffer,
                                       encoded_length, DOIP_UDP_DISCOVERY_PORT);
}

static void handle_vehicle_id_request(
    doip_entity_t *entity,
    const char *src_ip,
    uint16_t src_port,
    uint16_t payload_type,
    const uint8_t *payload,
    uint32_t payload_length)
{
    uint8_t buffer[64];
    uint32_t encoded_length;
    doip_vehicle_id_response_t response;
    bool should_respond = false;
    
    /* Check request type */
    if (payload_type == DOIP_PAYLOAD_TYPE_VEHICLE_ID_REQ) {
        should_respond = true;
    } else if (payload_type == DOIP_PAYLOAD_TYPE_VEHICLE_ID_REQ_VIN) {
        if ((payload_length >= DOIP_VIN_LENGTH) &&
            (memcmp(payload, entity->config.vin, DOIP_VIN_LENGTH) == 0)) {
            should_respond = true;
        }
    } else if (payload_type == DOIP_PAYLOAD_TYPE_VEHICLE_ID_REQ_EID) {
        if ((payload_length >= DOIP_EID_LENGTH) &&
            (memcmp(payload, entity->config.eid, DOIP_EID_LENGTH) == 0)) {
            should_respond = true;
        }
    } else {
        /* Unknown request type */
    }
    
    if (should_respond) {
        (void)memcpy(response.vin, entity->config.vin, DOIP_VIN_LENGTH);
        response.logical_address = entity->config.logical_address;
        (void)memcpy(response.eid, entity->config.eid, DOIP_EID_LENGTH);
        (void)memcpy(response.gid, entity->config.gid, DOIP_GID_LENGTH);
        response.further_action_required = 0x00U;
        response.sync_status = 0x00U;
        
        if (doip_encode_vehicle_id_response(&response, buffer,
                sizeof(buffer), &encoded_length) == DOIP_RESULT_OK) {
            debug_print("[DOIP TX UDP] Payload type: 0x%04X, length: %u\r\n", DOIP_PAYLOAD_TYPE_VEHICLE_ANNOUNCEMENT, encoded_length);
            (void)doip_interface_udp_send(entity->interface, src_ip,
                                         src_port, buffer, encoded_length);
        }
    }
}

static void handle_routing_activation_request(
    doip_entity_t *entity,
    int connection_id,
    const uint8_t *payload,
    uint32_t payload_length)
{
    doip_routing_activation_req_t request;
    doip_routing_activation_res_t response;
    uint8_t buffer[32];
    uint32_t encoded_length;
    uint8_t response_code = DOIP_ROUTING_ACT_RES_SUCCESS;
    
    /* Decode request */
    if (doip_decode_routing_activation_req(payload, payload_length,
            &request) != DOIP_RESULT_OK) {
        response_code = DOIP_ROUTING_ACT_RES_UNKNOWN_SOURCE;
    } else {

        //printf("[DOIP RX] routing_activation_request source_address:%x activation_type:%x oem_specific:%x", request.source_address , request.activation_type , request.oem_specific );

        /* Validate source address (should be in tester range 0x0E00-0x0FFF) */
        if ((request.source_address < DOIP_TESTER_ADDRESS_MIN) ||
            (request.source_address > DOIP_TESTER_ADDRESS_MAX)) {
            response_code = DOIP_ROUTING_ACT_RES_UNKNOWN_SOURCE;
        } else {
            /* Activate routing */
            entity->connections[connection_id].is_activated = true;
            entity->connections[connection_id].source_address = request.source_address;
            entity->connections[connection_id].general_inactivity_timer =
                entity->config.general_inactivity_time;
            response_code = DOIP_ROUTING_ACT_RES_SUCCESS;
        }
    }
    
    /* Send response */
    response.tester_address = request.source_address;
    response.entity_address = entity->config.logical_address;
    response.response_code = response_code;
    response.reserved = 0U;
    response.oem_specific = 0U;
    
    if (doip_encode_routing_activation_res(&response, buffer,
            sizeof(buffer), &encoded_length) == DOIP_RESULT_OK) {
        debug_print("[DOIP TX TCP] Payload type: 0x%04X, length: %u\r\n", DOIP_PAYLOAD_TYPE_ROUTING_ACTIVATION_RES, encoded_length);
        (void)doip_interface_tcp_send(entity->interface, connection_id,
                                     buffer, encoded_length);
    }
}

static void handle_diagnostic_message(
    doip_entity_t *entity,
    int connection_id,
    const uint8_t *payload,
    uint32_t payload_length)
{
    doip_diagnostic_message_t diag_msg;
    uint8_t ack_buffer[32];
    uint32_t ack_length;

    uint16_t ack_target_address;

    /* Verify connection is activated */
    if (!entity->connections[connection_id].is_activated) {
        /* Send NACK - routing not activated */
        (void)doip_encode_diag_message_ack(
            entity->config.logical_address,
            0x0000U,
            DOIP_DIAG_NACK_INVALID_SOURCE,
            ack_buffer,
            sizeof(ack_buffer),
            &ack_length
        );
        debug_print("[DOIP TX TCP] Payload type: 0x%04X, length: %u\r\n", DOIP_PAYLOAD_TYPE_DIAG_MESSAGE_NACK, ack_length);
        (void)doip_interface_tcp_send(entity->interface, connection_id,
                                     ack_buffer, ack_length);
        return;
    }

    /* Decode diagnostic message */
    if (doip_decode_diagnostic_message(payload, payload_length,
            &diag_msg) != DOIP_RESULT_OK) {
        return;
    }

    /* ack address setup */
    ack_target_address = diag_msg.source_address;

    /* Verify target address matches this entity */
    if (diag_msg.target_address != entity->config.logical_address) {
        (void)doip_encode_diag_message_ack(
            entity->config.logical_address,
            diag_msg.target_address,
            DOIP_DIAG_NACK_UNKNOWN_TARGET,
            ack_buffer,
            sizeof(ack_buffer),
            &ack_length
        );
        debug_print("[DOIP TX TCP] Payload type: 0x%04X, length: %u\r\n", DOIP_PAYLOAD_TYPE_DIAG_MESSAGE_NACK, ack_length);
        (void)doip_interface_tcp_send(entity->interface, connection_id,
                                     ack_buffer, ack_length);
        return;
    }

    /* Send positive ACK */
    (void)doip_encode_diag_message_ack(
        entity->config.logical_address,
        ack_target_address,
        0x00U, /* ACK */
        ack_buffer,
        sizeof(ack_buffer),
        &ack_length
    );
    debug_print("[DOIP TX TCP] Payload type: 0x%04X, length: %u\r\n", DOIP_PAYLOAD_TYPE_DIAG_MESSAGE_ACK, ack_length);
    (void)doip_interface_tcp_send(entity->interface, connection_id,
                                 ack_buffer, ack_length);

    /* Forward to UDS layer */
    if (entity->uds_callback != NULL) {
        entity->uds_callback(diag_msg.source_address, diag_msg.target_address,
                           diag_msg.user_data, diag_msg.user_data_length,
                           entity);
    }

    /* Reset inactivity timer */
    entity->connections[connection_id].general_inactivity_timer =
        entity->config.general_inactivity_time;
}

static void entity_udp_rx_callback(
    const char *src_ip,
    uint16_t src_port,
    const uint8_t *data,
    uint32_t length,
    void *user_data)
{
    doip_entity_t *entity = (doip_entity_t *)user_data;
    doip_header_t header;
    
    if (length < 8U) {
        return;
    }
    
    if (doip_decode_header(data, length, &header) != DOIP_RESULT_OK) {
        return;
    }
    
    if (!doip_validate_header(&header)) {
        /* Send Generic NACK */
        return;
    }

    debug_print("[DOIP RX UDP] Payload type: 0x%04X, length: %u\r\n", header.payload_type, header.payload_length);

    switch (header.payload_type) {
        case DOIP_PAYLOAD_TYPE_VEHICLE_ID_REQ:
        case DOIP_PAYLOAD_TYPE_VEHICLE_ID_REQ_EID:
        case DOIP_PAYLOAD_TYPE_VEHICLE_ID_REQ_VIN:
            handle_vehicle_id_request(entity, src_ip, src_port,
                header.payload_type, &data[8], header.payload_length);
            break;
        
        default:
            /* Unsupported payload type on UDP */
            break;
    }
}

static void entity_tcp_rx_callback(
    int connection_id,
    const uint8_t *data,
    uint32_t length,
    void *user_data)
{
    doip_entity_t *entity = (doip_entity_t *)user_data;
    doip_header_t header;
    
    if (doip_decode_header(data, length, &header) != DOIP_RESULT_OK) {
        return;
    }
    
    if (!doip_validate_header(&header)) {
        return;
    }

    debug_print("[DOIP RX TCP] Payload type: 0x%04X, length: %u\r\n", header.payload_type, header.payload_length);

    switch (header.payload_type) {
        case DOIP_PAYLOAD_TYPE_ROUTING_ACTIVATION_REQ:
            handle_routing_activation_request(entity, connection_id,
                &data[8], header.payload_length);
            break;
        
        case DOIP_PAYLOAD_TYPE_DIAG_MESSAGE:
            handle_diagnostic_message(entity, connection_id,
                &data[8], header.payload_length);
            break;
        
        case DOIP_PAYLOAD_TYPE_ALIVE_CHECK_RES:
            /* Reset alive check timer */
            entity->connections[connection_id].alive_check_pending = false;
            break;
        
        default:
            break;
    }
}

static void entity_tcp_connected_callback(
    int connection_id,
    void *user_data)
{
    doip_entity_t *entity = (doip_entity_t *)user_data;

    /* Start initial inactivity timer */
    entity->connections[connection_id].connection_id = connection_id;
    entity->connections[connection_id].initial_inactivity_timer =
        entity->config.initial_inactivity_time;
    entity->connections[connection_id].is_activated = false;  /* Only activate after routing activation */
    entity->connections[connection_id].source_address = 0U;   /* Clear any stale source address */
}

static void entity_tcp_disconnected_callback(
    int connection_id,
    void *user_data)
{
    doip_entity_t *entity = (doip_entity_t *)user_data;

    /* Reset connection to initial state */
    entity->connections[connection_id].connection_id = -1;
    entity->connections[connection_id].source_address = 0U;
    entity->connections[connection_id].is_activated = false;
    entity->connections[connection_id].initial_inactivity_timer = 0U;
    entity->connections[connection_id].general_inactivity_timer = 0U;
    entity->connections[connection_id].alive_check_timer = 0U;
    entity->connections[connection_id].alive_check_pending = false;

    debug_print("[DOIP] Connection %d disconnected and cleaned up\r\n", connection_id);
}

doip_result_t doip_entity_process(
    doip_entity_t *entity,
    doip_entity_uds_rx_callback_t uds_callback)
{
    if (entity == NULL) {
        return DOIP_RESULT_INVALID_PARAM;
    }

    entity->uds_callback = uds_callback;

    return doip_interface_process(
        entity->interface,
        entity_udp_rx_callback,
        entity_tcp_rx_callback,
        entity_tcp_connected_callback,
        entity_tcp_disconnected_callback,
        (void *)entity
    );
}

doip_result_t doip_entity_send_diagnostic_response(
    doip_entity_t *entity,
    uint16_t target_addr,
    const uint8_t *data,
    uint32_t length)
{
    doip_diagnostic_message_t diag_msg;
    uint8_t buffer[DOIP_MAX_PAYLOAD_SIZE + 8U];
    uint32_t encoded_length;
    uint32_t i;
    int target_connection = -1;
    
    if ((entity == NULL) || (data == NULL)) {
        return DOIP_RESULT_INVALID_PARAM;
    }
    
    /* Find connection for target address */
    for (i = 0U; i < DOIP_MAX_CONNECTIONS; i++) {
        if (entity->connections[i].is_activated &&
            (entity->connections[i].source_address == target_addr)) {
            target_connection = entity->connections[i].connection_id;
            break;
        }
    }
    
    if (target_connection < 0) {
        return DOIP_RESULT_NOT_READY;
    }
    
    /* Encode diagnostic message */
    diag_msg.source_address = entity->config.logical_address;
    diag_msg.target_address = target_addr;
    diag_msg.user_data = data;
    diag_msg.user_data_length = length;
    
    if (doip_encode_diagnostic_message(&diag_msg, buffer,
            sizeof(buffer), &encoded_length) != DOIP_RESULT_OK) {
        return DOIP_RESULT_ERROR;
    }

    debug_print("[DOIP TX TCP] Payload type: 0x%04X, length: %u\r\n", DOIP_PAYLOAD_TYPE_DIAG_MESSAGE, encoded_length);
    return doip_interface_tcp_send(entity->interface, target_connection,
                                  buffer, encoded_length);
}

void doip_entity_update_timers(doip_entity_t *entity, uint32_t elapsed_ms)
{
    uint32_t i;
    
    if (entity == NULL) {
        return;
    }
    
    /* Update announcement timer */
    if (entity->announcement_count < 3U) {
        if (entity->announcement_timer > elapsed_ms) {
            entity->announcement_timer -= elapsed_ms;
        } else {
            entity->announcement_timer = 500U; /* 500ms interval per ISO 13400 */
            (void)doip_entity_send_vehicle_announcement(entity);
            entity->announcement_count++;
        }
    }
    
    /* Update connection timers */
    for (i = 0U; i < DOIP_MAX_CONNECTIONS; i++) {
        if (entity->connections[i].connection_id >= 0) {
            if (!entity->connections[i].is_activated) {
                /* Initial inactivity timer */
                if (entity->connections[i].initial_inactivity_timer > elapsed_ms) {
                    entity->connections[i].initial_inactivity_timer -= elapsed_ms;
                } else {
                    /* Timeout - close connection */
                    doip_interface_close_connection(entity->interface,
                        entity->connections[i].connection_id);
                }
            } else {
                /* General inactivity timer */
                if (entity->connections[i].general_inactivity_timer > elapsed_ms) {
                    entity->connections[i].general_inactivity_timer -= elapsed_ms;
                } else {
                    /* Timeout - close connection */
                    doip_interface_close_connection(entity->interface,
                        entity->connections[i].connection_id);
                }
            }
        }
    }
}

doip_result_t doip_encode_diag_message_ack(
    uint16_t source_address,
    uint16_t target_address,
    uint8_t ack_code,
    uint8_t *buffer,
    uint32_t buffer_size,
    uint32_t *encoded_length)
{
    doip_header_t header;
    uint32_t offset = 0U;
    uint16_t payload_type;
    
    if ((buffer == NULL) || (encoded_length == NULL)) {
        return DOIP_RESULT_INVALID_PARAM;
    }
    
    if (buffer_size < 13U) {
        return DOIP_RESULT_BUFFER_TOO_SMALL;
    }
    
    payload_type = (ack_code == 0x00U) ? DOIP_PAYLOAD_TYPE_DIAG_MESSAGE_ACK :
                                         DOIP_PAYLOAD_TYPE_DIAG_MESSAGE_NACK;
    
    header.protocol_version = DOIP_PROTOCOL_VERSION_2019;
    header.inverse_protocol_version = DOIP_INVERSE_VERSION_2019;
    header.payload_type = payload_type;
    header.payload_length = 5U;
    
    if (doip_encode_header(&header, buffer, buffer_size) != DOIP_RESULT_OK) {
        return DOIP_RESULT_ERROR;
    }
    offset = 8U;
    
    buffer[offset] = (uint8_t)(source_address >> 8);
    buffer[offset + 1U] = (uint8_t)(source_address & 0xFFU);
    offset += 2U;
    
    buffer[offset] = (uint8_t)(target_address >> 8);
    buffer[offset + 1U] = (uint8_t)(target_address & 0xFFU);
    offset += 2U;
    
    buffer[offset++] = ack_code;
    
    *encoded_length = offset;
    return DOIP_RESULT_OK;
}
