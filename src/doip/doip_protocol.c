#include "doip_protocol.h"
#include <string.h>

/* Helper: Write 16-bit big-endian */
static void write_be16(uint8_t *buf, uint16_t val)
{
    buf[0] = (uint8_t)(val >> 8);
    buf[1] = (uint8_t)(val & 0xFFU);
}

/* Helper: Write 32-bit big-endian */
static void write_be32(uint8_t *buf, uint32_t val)
{
    buf[0] = (uint8_t)(val >> 24);
    buf[1] = (uint8_t)((val >> 16) & 0xFFU);
    buf[2] = (uint8_t)((val >> 8) & 0xFFU);
    buf[3] = (uint8_t)(val & 0xFFU);
}

/* Helper: Read 16-bit big-endian */
static uint16_t read_be16(const uint8_t *buf)
{
    return (uint16_t)((uint16_t)buf[0] << 8) | (uint16_t)buf[1];
}

/* Helper: Read 32-bit big-endian */
static uint32_t read_be32(const uint8_t *buf)
{
    return ((uint32_t)buf[0] << 24) | ((uint32_t)buf[1] << 16) |
           ((uint32_t)buf[2] << 8) | (uint32_t)buf[3];
}

doip_result_t doip_encode_header(
    const doip_header_t *header,
    uint8_t *buffer,
    uint32_t buffer_size)
{
    if ((header == NULL) || (buffer == NULL)) {
        return DOIP_RESULT_INVALID_PARAM;
    }
    
    if (buffer_size < 8U) {
        return DOIP_RESULT_BUFFER_TOO_SMALL;
    }
    
    buffer[0] = header->protocol_version;
    buffer[1] = header->inverse_protocol_version;
    write_be16(&buffer[2], header->payload_type);
    write_be32(&buffer[4], header->payload_length);
    
    return DOIP_RESULT_OK;
}

doip_result_t doip_decode_header(
    const uint8_t *buffer,
    uint32_t buffer_size,
    doip_header_t *header)
{
    if ((buffer == NULL) || (header == NULL)) {
        return DOIP_RESULT_INVALID_PARAM;
    }
    
    if (buffer_size < 8U) {
        return DOIP_RESULT_BUFFER_TOO_SMALL;
    }
    
    header->protocol_version = buffer[0];
    header->inverse_protocol_version = buffer[1];
    header->payload_type = read_be16(&buffer[2]);
    header->payload_length = read_be32(&buffer[4]);
    
    return DOIP_RESULT_OK;
}

bool doip_validate_header(const doip_header_t *header)
{
    bool is_valid = false;
    
    if (header != NULL) {
        /* Check protocol version and inverse version */
        if ((header->protocol_version == DOIP_PROTOCOL_VERSION_2012) &&
            (header->inverse_protocol_version == DOIP_INVERSE_VERSION_2012)) {
            is_valid = true;
        } else if ((header->protocol_version == DOIP_PROTOCOL_VERSION_2019) &&
                   (header->inverse_protocol_version == DOIP_INVERSE_VERSION_2019)) {
            is_valid = true;
        } else {
            is_valid = false;
        }
        
        /* Check payload length is reasonable */
        if (is_valid && (header->payload_length > DOIP_MAX_PAYLOAD_SIZE)) {
            is_valid = false;
        }
    }
    
    return is_valid;
}

doip_result_t doip_encode_vehicle_id_response(
    const doip_vehicle_id_response_t *response,
    uint8_t *buffer,
    uint32_t buffer_size,
    uint32_t *encoded_length)
{
    doip_header_t header;
    uint32_t offset = 0U;
    
    if ((response == NULL) || (buffer == NULL) || (encoded_length == NULL)) {
        return DOIP_RESULT_INVALID_PARAM;
    }
    
    /* Total: 8 (header) + 33 (payload) */
    if (buffer_size < 41U) {
        return DOIP_RESULT_BUFFER_TOO_SMALL;
    }
    
    /* Encode header */
    header.protocol_version = DOIP_PROTOCOL_VERSION_2019;
    header.inverse_protocol_version = DOIP_INVERSE_VERSION_2019;
    header.payload_type = DOIP_PAYLOAD_TYPE_VEHICLE_ANNOUNCEMENT;
    header.payload_length = 33U;
    
    if (doip_encode_header(&header, buffer, buffer_size) != DOIP_RESULT_OK) {
        return DOIP_RESULT_ERROR;
    }
    offset = 8U;
    
    /* Encode payload */
    (void)memcpy(&buffer[offset], response->vin, DOIP_VIN_LENGTH);
    offset += DOIP_VIN_LENGTH;
    
    write_be16(&buffer[offset], response->logical_address);
    offset += 2U;
    
    (void)memcpy(&buffer[offset], response->eid, DOIP_EID_LENGTH);
    offset += DOIP_EID_LENGTH;
    
    (void)memcpy(&buffer[offset], response->gid, DOIP_GID_LENGTH);
    offset += DOIP_GID_LENGTH;
    
    buffer[offset++] = response->further_action_required;
    buffer[offset++] = response->sync_status;
    
    *encoded_length = offset;
    return DOIP_RESULT_OK;
}

doip_result_t doip_encode_routing_activation_req(
    const doip_routing_activation_req_t *request,
    uint8_t *buffer,
    uint32_t buffer_size,
    uint32_t *encoded_length)
{
    doip_header_t header;
    uint32_t offset = 0U;
    
    if ((request == NULL) || (buffer == NULL) || (encoded_length == NULL)) {
        return DOIP_RESULT_INVALID_PARAM;
    }
    
    if (buffer_size < 19U) { /* 8 + 11 */
        return DOIP_RESULT_BUFFER_TOO_SMALL;
    }
    
    header.protocol_version = DOIP_PROTOCOL_VERSION_2019;
    header.inverse_protocol_version = DOIP_INVERSE_VERSION_2019;
    header.payload_type = DOIP_PAYLOAD_TYPE_ROUTING_ACTIVATION_REQ;
    header.payload_length = 11U;
    
    if (doip_encode_header(&header, buffer, buffer_size) != DOIP_RESULT_OK) {
        return DOIP_RESULT_ERROR;
    }
    offset = 8U;
    
    write_be16(&buffer[offset], request->source_address);
    offset += 2U;
    buffer[offset++] = request->activation_type;
    write_be32(&buffer[offset], request->reserved);
    offset += 4U;
    write_be32(&buffer[offset], request->oem_specific);
    offset += 4U;
    
    *encoded_length = offset;
    return DOIP_RESULT_OK;
}

doip_result_t doip_decode_routing_activation_req(
    const uint8_t *payload,
    uint32_t payload_length,
    doip_routing_activation_req_t *request)
{
    if ((payload == NULL) || (request == NULL)) {
        return DOIP_RESULT_INVALID_PARAM;
    }
    
    //if (payload_length < 11U) {
    //    return DOIP_RESULT_INVALID_FORMAT;
    //}
    
    request->source_address = read_be16(&payload[0]);
    request->activation_type = payload[2];
    request->reserved = read_be32(&payload[3]);
    request->oem_specific = (payload_length >= 11U) ? read_be32(&payload[7]) : 0U;
    
    return DOIP_RESULT_OK;
}

doip_result_t doip_encode_routing_activation_res(
    const doip_routing_activation_res_t *response,
    uint8_t *buffer,
    uint32_t buffer_size,
    uint32_t *encoded_length)
{
    doip_header_t header;
    uint32_t offset = 0U;
    
    if ((response == NULL) || (buffer == NULL) || (encoded_length == NULL)) {
        return DOIP_RESULT_INVALID_PARAM;
    }
    
    if (buffer_size < 21U) { /* 8 + 13 */
        return DOIP_RESULT_BUFFER_TOO_SMALL;
    }
    
    header.protocol_version = DOIP_PROTOCOL_VERSION_2019;
    header.inverse_protocol_version = DOIP_INVERSE_VERSION_2019;
    header.payload_type = DOIP_PAYLOAD_TYPE_ROUTING_ACTIVATION_RES;
    header.payload_length = 13U;
    
    if (doip_encode_header(&header, buffer, buffer_size) != DOIP_RESULT_OK) {
        return DOIP_RESULT_ERROR;
    }
    offset = 8U;
    
    write_be16(&buffer[offset], response->tester_address);
    offset += 2U;
    write_be16(&buffer[offset], response->entity_address);
    offset += 2U;
    buffer[offset++] = response->response_code;
    write_be32(&buffer[offset], response->reserved);
    offset += 4U;
    write_be32(&buffer[offset], response->oem_specific);
    offset += 4U;
    
    *encoded_length = offset;
    return DOIP_RESULT_OK;
}

doip_result_t doip_encode_diagnostic_message(
    const doip_diagnostic_message_t *message,
    uint8_t *buffer,
    uint32_t buffer_size,
    uint32_t *encoded_length)
{
    doip_header_t header;
    uint32_t offset = 0U;
    uint32_t total_length;
    
    if ((message == NULL) || (buffer == NULL) || (encoded_length == NULL)) {
        return DOIP_RESULT_INVALID_PARAM;
    }
    
    if (message->user_data == NULL) {
        return DOIP_RESULT_INVALID_PARAM;
    }
    
    total_length = 8U + 4U + message->user_data_length;
    if (buffer_size < total_length) {
        return DOIP_RESULT_BUFFER_TOO_SMALL;
    }
    
    header.protocol_version = DOIP_PROTOCOL_VERSION_2019;
    header.inverse_protocol_version = DOIP_INVERSE_VERSION_2019;
    header.payload_type = DOIP_PAYLOAD_TYPE_DIAG_MESSAGE;
    header.payload_length = 4U + message->user_data_length;
    
    if (doip_encode_header(&header, buffer, buffer_size) != DOIP_RESULT_OK) {
        return DOIP_RESULT_ERROR;
    }
    offset = 8U;
    
    write_be16(&buffer[offset], message->source_address);
    offset += 2U;
    write_be16(&buffer[offset], message->target_address);
    offset += 2U;
    
    (void)memcpy(&buffer[offset], message->user_data, message->user_data_length);
    offset += message->user_data_length;
    
    *encoded_length = offset;
    return DOIP_RESULT_OK;
}

doip_result_t doip_decode_diagnostic_message(
    const uint8_t *payload,
    uint32_t payload_length,
    doip_diagnostic_message_t *message)
{
    if ((payload == NULL) || (message == NULL)) {
        return DOIP_RESULT_INVALID_PARAM;
    }
    
    if (payload_length < 4U) {
        return DOIP_RESULT_INVALID_FORMAT;
    }
    
    message->source_address = read_be16(&payload[0]);
    message->target_address = read_be16(&payload[2]);
    message->user_data = &payload[4];
    message->user_data_length = payload_length - 4U;
    
    return DOIP_RESULT_OK;
}

doip_result_t doip_encode_generic_nack(
    uint8_t nack_code,
    uint8_t *buffer,
    uint32_t buffer_size,
    uint32_t *encoded_length)
{
    doip_header_t header;
    
    if ((buffer == NULL) || (encoded_length == NULL)) {
        return DOIP_RESULT_INVALID_PARAM;
    }
    
    if (buffer_size < 9U) {
        return DOIP_RESULT_BUFFER_TOO_SMALL;
    }
    
    header.protocol_version = DOIP_PROTOCOL_VERSION_2019;
    header.inverse_protocol_version = DOIP_INVERSE_VERSION_2019;
    header.payload_type = DOIP_PAYLOAD_TYPE_GENERIC_NACK;
    header.payload_length = 1U;
    
    if (doip_encode_header(&header, buffer, buffer_size) != DOIP_RESULT_OK) {
        return DOIP_RESULT_ERROR;
    }
    
    buffer[8] = nack_code;
    *encoded_length = 9U;
    
    return DOIP_RESULT_OK;
}
