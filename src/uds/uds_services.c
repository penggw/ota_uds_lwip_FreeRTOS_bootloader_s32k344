#include "uds_services.h"
#include <string.h>
#include <stdio.h>

void uds_init(uds_context_t *context)
{
    if (context != NULL) {
        context->current_session = UDS_SESSION_DEFAULT;
        context->security_level = 0U;
        context->security_unlocked = false;
        context->seed = 0U;
        context->failed_security_attempts = 0U;
        context->last_tester_present_time = 0U;
    }
}

void uds_send_negative_response(
    uint8_t sid,
    uint8_t nrc,
    uds_response_t *response)
{
    if (response != NULL) {
        if (response->max_length >= 3U) {
            response->buffer[0] = 0x7FU;  /* Negative Response SID */
            response->buffer[1] = sid;
            response->buffer[2] = nrc;
            response->actual_length = 3U;
        }
    }
}

void uds_send_positive_response(
    uint8_t sid,
    const uint8_t *data,
    uint32_t length,
    uds_response_t *response)
{
    if (response != NULL) {
        uint32_t total_length = 1U + length;
        if (response->max_length >= total_length) {
            response->buffer[0] = sid + UDS_POSITIVE_RESPONSE_OFFSET;
            if ((data != NULL) && (length > 0U)) {
                (void)memcpy(&response->buffer[1], data, length);
            }
            response->actual_length = total_length;
        }
    }
}

void uds_handle_diagnostic_session_control(
    uds_context_t *context,
    const uds_request_t *request,
    uds_response_t *response)
{
    if ((request == NULL) || (response == NULL) || (context == NULL)) {
        return;
    }
    
    /* Check message length */
    if (request->length < 1U) {
        uds_send_negative_response(UDS_SID_DIAGNOSTIC_SESSION_CONTROL,
                                  UDS_NRC_INCORRECT_MESSAGE_LENGTH,
                                  response);
        return;
    }
    
    uint8_t session_type = request->data[0];
    
    /* Validate session type */
    if ((session_type != UDS_SESSION_DEFAULT) &&
        (session_type != UDS_SESSION_PROGRAMMING) &&
        (session_type != UDS_SESSION_EXTENDED)) {
        uds_send_negative_response(UDS_SID_DIAGNOSTIC_SESSION_CONTROL,
                                  UDS_NRC_SUB_FUNCTION_NOT_SUPPORTED,
                                  response);
        return;
    }
    
    /* Switch session */
    context->current_session = session_type;
    
    /* Send positive response */
    uint8_t resp_data[5];
    resp_data[0] = session_type;
    resp_data[1] = 0x00U;  /* P2 Server Max (high byte) */
    resp_data[2] = 0x32U;  /* P2 Server Max (low byte) = 50ms */
    resp_data[3] = 0x01U;  /* P2* Server Max (high byte) */
    resp_data[4] = 0xF4U;  /* P2* Server Max (low byte) = 5000ms */
    
    uds_send_positive_response(UDS_SID_DIAGNOSTIC_SESSION_CONTROL,
                              resp_data, sizeof(resp_data), response);
}

void uds_handle_ecu_reset(
    uds_context_t *context,
    const uds_request_t *request,
    uds_response_t *response)
{
    if ((request == NULL) || (response == NULL) || (context == NULL)) {
        return;
    }
    
    if (request->length < 1U) {
        uds_send_negative_response(UDS_SID_ECU_RESET,
                                  UDS_NRC_INCORRECT_MESSAGE_LENGTH,
                                  response);
        return;
    }
    
    uint8_t reset_type = request->data[0];
    
    /* Validate reset type */
    if ((reset_type != UDS_RESET_HARD) &&
        (reset_type != UDS_RESET_KEY_OFF_ON) &&
        (reset_type != UDS_RESET_SOFT)) {
        uds_send_negative_response(UDS_SID_ECU_RESET,
                                  UDS_NRC_SUB_FUNCTION_NOT_SUPPORTED,
                                  response);
        return;
    }
    
    /* Check security access */
    if (!context->security_unlocked &&
        (context->current_session != UDS_SESSION_DEFAULT)) {
        uds_send_negative_response(UDS_SID_ECU_RESET,
                                  UDS_NRC_SECURITY_ACCESS_DENIED,
                                  response);
        return;
    }
    
    /* Send positive response first */
    uint8_t resp_data[1];
    resp_data[0] = reset_type;
    uds_send_positive_response(UDS_SID_ECU_RESET, resp_data, 1U, response);
    
    /* TODO: Trigger actual ECU reset after response is sent */
    /* NVIC_SystemReset(); */
}

void uds_handle_security_access(
    uds_context_t *context,
    const uds_request_t *request,
    uds_response_t *response)
{
    if ((request == NULL) || (response == NULL) || (context == NULL)) {
        return;
    }
    
    if (request->length < 1U) {
        uds_send_negative_response(UDS_SID_SECURITY_ACCESS,
                                  UDS_NRC_INCORRECT_MESSAGE_LENGTH,
                                  response);
        return;
    }
    
    uint8_t sub_function = request->data[0];
    
    if (sub_function == UDS_SECURITY_REQUEST_SEED_LEVEL_1) {
        /* Check if already unlocked */
        if (context->security_unlocked) {
            uint8_t resp_data[5];
            resp_data[0] = sub_function;
            resp_data[1] = 0x00U;
            resp_data[2] = 0x00U;
            resp_data[3] = 0x00U;
            resp_data[4] = 0x00U;
            uds_send_positive_response(UDS_SID_SECURITY_ACCESS,
                                      resp_data, 5U, response);
            return;
        }
        
        /* Generate seed (simplified - should use proper random generation) */
        context->seed = 0x12345678U;
        
        uint8_t resp_data[5];
        resp_data[0] = sub_function;
        resp_data[1] = (uint8_t)(context->seed >> 24);
        resp_data[2] = (uint8_t)(context->seed >> 16);
        resp_data[3] = (uint8_t)(context->seed >> 8);
        resp_data[4] = (uint8_t)(context->seed & 0xFFU);
        
        uds_send_positive_response(UDS_SID_SECURITY_ACCESS,
                                  resp_data, 5U, response);
    }
    else if (sub_function == UDS_SECURITY_SEND_KEY_LEVEL_1) {
        /* Verify key */
        if (request->length < 5U) {
            uds_send_negative_response(UDS_SID_SECURITY_ACCESS,
                                      UDS_NRC_INCORRECT_MESSAGE_LENGTH,
                                      response);
            return;
        }
        
        /* Extract received key */
        uint32_t received_key = ((uint32_t)request->data[1] << 24) |
                               ((uint32_t)request->data[2] << 16) |
                               ((uint32_t)request->data[3] << 8) |
                               (uint32_t)request->data[4];
        
        /* Calculate expected key (simplified algorithm) */
        uint32_t expected_key = context->seed ^ 0xA5A5A5A5U;
        
        if (received_key == expected_key) {
            /* Unlock security */
            context->security_unlocked = true;
            context->failed_security_attempts = 0U;
            
            uint8_t resp_data[1];
            resp_data[0] = sub_function;
            uds_send_positive_response(UDS_SID_SECURITY_ACCESS,
                                      resp_data, 1U, response);
        } else {
            /* Invalid key */
            context->failed_security_attempts++;
            
            if (context->failed_security_attempts >= 3U) {
                uds_send_negative_response(UDS_SID_SECURITY_ACCESS,
                                          UDS_NRC_EXCEEDED_NUMBER_OF_ATTEMPTS,
                                          response);
            } else {
                uds_send_negative_response(UDS_SID_SECURITY_ACCESS,
                                          UDS_NRC_INVALID_KEY,
                                          response);
            }
        }
    }
    else {
        uds_send_negative_response(UDS_SID_SECURITY_ACCESS,
                                  UDS_NRC_SUB_FUNCTION_NOT_SUPPORTED,
                                  response);
    }
}

void uds_handle_tester_present(
    uds_context_t *context,
    const uds_request_t *request,
    uds_response_t *response)
{
    if ((request == NULL) || (response == NULL) || (context == NULL)) {
        return;
    }
    
    if (request->length < 1U) {
        uds_send_negative_response(UDS_SID_TESTER_PRESENT,
                                  UDS_NRC_INCORRECT_MESSAGE_LENGTH,
                                  response);
        return;
    }
    
    /* Update last tester present time */
    /* context->last_tester_present_time = get_system_time_ms(); */
    
    /* Send positive response */
    uint8_t resp_data[1];
    resp_data[0] = request->data[0];  /* Sub-function (0x00 or 0x80) */
    uds_send_positive_response(UDS_SID_TESTER_PRESENT, resp_data, 1U, response);
}

void uds_handle_read_data_by_id(
    uds_context_t *context,
    const uds_request_t *request,
    uds_response_t *response)
{
    if ((request == NULL) || (response == NULL) || (context == NULL)) {
        return;
    }
    
    if (request->length < 2U) {
        uds_send_negative_response(UDS_SID_READ_DATA_BY_IDENTIFIER,
                                  UDS_NRC_INCORRECT_MESSAGE_LENGTH,
                                  response);
        return;
    }
    
    uint16_t did = ((uint16_t)request->data[0] << 8) | (uint16_t)request->data[1];
    
    uint8_t resp_data[32];
    uint32_t resp_length = 0U;
    
    /* Store DID in response */
    resp_data[0] = request->data[0];
    resp_data[1] = request->data[1];
    resp_length = 2U;
    
    switch (did) {
        case UDS_DID_VIN:
            /* Return VIN */
            (void)memcpy(&resp_data[resp_length], "WVWZZZ1KZ1A234567", 17U);
            resp_length += 17U;
            break;
        
        case UDS_DID_ECU_SERIAL_NUMBER:
            /* Return serial number */
            (void)memcpy(&resp_data[resp_length], "SN123456789", 11U);
            resp_length += 11U;
            break;
        
        case UDS_DID_ECU_SOFTWARE_NUMBER:
            /* Return software version */
            resp_data[resp_length++] = 0x01U;  /* Major */
            resp_data[resp_length++] = 0x00U;  /* Minor */
            resp_data[resp_length++] = 0x05U;  /* Patch */
            break;
        
        default:
            uds_send_negative_response(UDS_SID_READ_DATA_BY_IDENTIFIER,
                                      UDS_NRC_REQUEST_OUT_OF_RANGE,
                                      response);
            return;
    }
    
    uds_send_positive_response(UDS_SID_READ_DATA_BY_IDENTIFIER,
                              resp_data, resp_length, response);
}

bool uds_process_request(
    uds_context_t *context,
    const uds_request_t *request,
    uds_response_t *response)
{
    if ((context == NULL) || (request == NULL) || (response == NULL)) {
        return false;
    }
    
    /* Route to appropriate service handler */
    switch (request->sid) {
        case UDS_SID_DIAGNOSTIC_SESSION_CONTROL:
            uds_handle_diagnostic_session_control(context, request, response);
            break;
        
        case UDS_SID_ECU_RESET:
            uds_handle_ecu_reset(context, request, response);
            break;
        
        case UDS_SID_SECURITY_ACCESS:
            uds_handle_security_access(context, request, response);
            break;
        
        case UDS_SID_TESTER_PRESENT:
            uds_handle_tester_present(context, request, response);
            break;
        
        case UDS_SID_READ_DATA_BY_IDENTIFIER:
            uds_handle_read_data_by_id(context, request, response);
            break;
        
        default:
            uds_send_negative_response(request->sid,
                                      UDS_NRC_SERVICE_NOT_SUPPORTED,
                                      response);
            break;
    }
    
    return (response->actual_length > 0U);
}
