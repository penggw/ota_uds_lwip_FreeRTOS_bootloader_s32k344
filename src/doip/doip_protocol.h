#ifndef DOIP_PROTOCOL_H
#define DOIP_PROTOCOL_H

#include <stdint.h>
#include <stdbool.h>

/* Protocol Version */
#define DOIP_PROTOCOL_VERSION_2012    0x02U
#define DOIP_PROTOCOL_VERSION_2019    0x03U
#define DOIP_INVERSE_VERSION_2012     0xFDU
#define DOIP_INVERSE_VERSION_2019     0xFCU

/* Payload Types */
#define DOIP_PAYLOAD_TYPE_GENERIC_NACK              0x0000U
#define DOIP_PAYLOAD_TYPE_VEHICLE_ID_REQ            0x0001U
#define DOIP_PAYLOAD_TYPE_VEHICLE_ID_REQ_EID        0x0002U
#define DOIP_PAYLOAD_TYPE_VEHICLE_ID_REQ_VIN        0x0003U
#define DOIP_PAYLOAD_TYPE_VEHICLE_ANNOUNCEMENT      0x0004U
#define DOIP_PAYLOAD_TYPE_ROUTING_ACTIVATION_REQ    0x0005U
#define DOIP_PAYLOAD_TYPE_ROUTING_ACTIVATION_RES    0x0006U
#define DOIP_PAYLOAD_TYPE_ALIVE_CHECK_REQ           0x0007U
#define DOIP_PAYLOAD_TYPE_ALIVE_CHECK_RES           0x0008U
#define DOIP_PAYLOAD_TYPE_ENTITY_STATUS_REQ         0x4001U
#define DOIP_PAYLOAD_TYPE_ENTITY_STATUS_RES         0x4002U
#define DOIP_PAYLOAD_TYPE_DIAG_POWER_MODE_REQ       0x4003U
#define DOIP_PAYLOAD_TYPE_DIAG_POWER_MODE_RES       0x4004U
#define DOIP_PAYLOAD_TYPE_DIAG_MESSAGE              0x8001U
#define DOIP_PAYLOAD_TYPE_DIAG_MESSAGE_ACK          0x8002U
#define DOIP_PAYLOAD_TYPE_DIAG_MESSAGE_NACK         0x8003U

/* NACK Codes */
#define DOIP_NACK_INCORRECT_PATTERN                 0x00U
#define DOIP_NACK_UNKNOWN_PAYLOAD_TYPE              0x01U
#define DOIP_NACK_MESSAGE_TOO_LARGE                 0x02U
#define DOIP_NACK_OUT_OF_MEMORY                     0x03U
#define DOIP_NACK_INVALID_PAYLOAD_LENGTH            0x04U

/* Routing Activation Response Codes */
#define DOIP_ROUTING_ACT_RES_UNKNOWN_SOURCE         0x00U
#define DOIP_ROUTING_ACT_RES_NO_SOCKETS             0x01U
#define DOIP_ROUTING_ACT_RES_DIFF_SOURCE            0x02U
#define DOIP_ROUTING_ACT_RES_ACTIVE                 0x03U
#define DOIP_ROUTING_ACT_RES_AUTH_MISSING           0x04U
#define DOIP_ROUTING_ACT_RES_CONFIRM_REJECTED       0x05U
#define DOIP_ROUTING_ACT_RES_UNSUPPORTED_TYPE       0x06U
#define DOIP_ROUTING_ACT_RES_SUCCESS                0x10U
#define DOIP_ROUTING_ACT_RES_CONFIRM_REQUIRED       0x11U

/* Diagnostic Message NACK Codes */
#define DOIP_DIAG_NACK_INVALID_SOURCE               0x02U
#define DOIP_DIAG_NACK_UNKNOWN_TARGET               0x03U
#define DOIP_DIAG_NACK_MESSAGE_TOO_LARGE            0x04U
#define DOIP_DIAG_NACK_OUT_OF_MEMORY                0x05U
#define DOIP_DIAG_NACK_TARGET_UNREACHABLE           0x06U
#define DOIP_DIAG_NACK_UNKNOWN_NETWORK              0x07U
#define DOIP_DIAG_NACK_TRANSPORT_PROTOCOL           0x08U

/* Timer Defaults (milliseconds) */
#define DOIP_DEFAULT_GENERAL_INACTIVITY_TIME        5000U
#define DOIP_DEFAULT_INITIAL_INACTIVITY_TIME        2000U
#define DOIP_DEFAULT_ALIVE_CHECK_TIME               500U

/* Buffer Sizes */
#define DOIP_MAX_PAYLOAD_SIZE                       (4 * 1024U)
#define DOIP_VIN_LENGTH                             17U
#define DOIP_EID_LENGTH                             6U
#define DOIP_GID_LENGTH                             6U

/* Result Codes */
typedef enum {
    DOIP_RESULT_OK = 0,
    DOIP_RESULT_ERROR,
    DOIP_RESULT_INVALID_PARAM,
    DOIP_RESULT_BUFFER_TOO_SMALL,
    DOIP_RESULT_INVALID_FORMAT,
    DOIP_RESULT_TIMEOUT,
    DOIP_RESULT_NOT_READY,
    DOIP_RESULT_NO_MEMORY
} doip_result_t;

/* DoIP Generic Header (8 bytes) */
typedef struct {
    uint8_t protocol_version;
    uint8_t inverse_protocol_version;
    uint16_t payload_type;
    uint32_t payload_length;
} doip_header_t;

/* Vehicle Identification Response/Announcement */
typedef struct {
    uint8_t vin[DOIP_VIN_LENGTH];
    uint16_t logical_address;
    uint8_t eid[DOIP_EID_LENGTH];
    uint8_t gid[DOIP_GID_LENGTH];
    uint8_t further_action_required;
    uint8_t sync_status;
} doip_vehicle_id_response_t;

/* Routing Activation Request */
typedef struct {
    uint16_t source_address;
    uint8_t activation_type;
    uint32_t reserved;
    uint32_t oem_specific;
} doip_routing_activation_req_t;

/* Routing Activation Response */
typedef struct {
    uint16_t tester_address;
    uint16_t entity_address;
    uint8_t response_code;
    uint32_t reserved;
    uint32_t oem_specific;
} doip_routing_activation_res_t;

/* Diagnostic Message */
typedef struct {
    uint16_t source_address;
    uint16_t target_address;
    uint32_t user_data_length;
    const uint8_t *user_data;
} doip_diagnostic_message_t;

/* Function Prototypes */
doip_result_t doip_encode_header(
    const doip_header_t *header,
    uint8_t *buffer,
    uint32_t buffer_size
);

doip_result_t doip_decode_header(
    const uint8_t *buffer,
    uint32_t buffer_size,
    doip_header_t *header
);

doip_result_t doip_encode_vehicle_id_response(
    const doip_vehicle_id_response_t *response,
    uint8_t *buffer,
    uint32_t buffer_size,
    uint32_t *encoded_length
);

doip_result_t doip_decode_vehicle_id_request(
    uint16_t payload_type,
    const uint8_t *payload,
    uint32_t payload_length,
    uint8_t *vin_or_eid
);

doip_result_t doip_encode_routing_activation_req(
    const doip_routing_activation_req_t *request,
    uint8_t *buffer,
    uint32_t buffer_size,
    uint32_t *encoded_length
);

doip_result_t doip_decode_routing_activation_req(
    const uint8_t *payload,
    uint32_t payload_length,
    doip_routing_activation_req_t *request
);

doip_result_t doip_encode_routing_activation_res(
    const doip_routing_activation_res_t *response,
    uint8_t *buffer,
    uint32_t buffer_size,
    uint32_t *encoded_length
);

doip_result_t doip_encode_diagnostic_message(
    const doip_diagnostic_message_t *message,
    uint8_t *buffer,
    uint32_t buffer_size,
    uint32_t *encoded_length
);

doip_result_t doip_decode_diagnostic_message(
    const uint8_t *payload,
    uint32_t payload_length,
    doip_diagnostic_message_t *message
);

doip_result_t doip_encode_generic_nack(
    uint8_t nack_code,
    uint8_t *buffer,
    uint32_t buffer_size,
    uint32_t *encoded_length
);

doip_result_t doip_encode_diag_message_ack(
    uint16_t source_address,
    uint16_t target_address,
    uint8_t ack_code,
    uint8_t *buffer,
    uint32_t buffer_size,
    uint32_t *encoded_length
);

bool doip_validate_header(const doip_header_t *header);

#endif /* DOIP_PROTOCOL_H */
