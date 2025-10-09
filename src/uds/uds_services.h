#ifndef UDS_SERVICES_H
#define UDS_SERVICES_H

#include <stdint.h>
#include <stdbool.h>

/* UDS Service IDs (SID) */
#define UDS_SID_DIAGNOSTIC_SESSION_CONTROL      0x10U
#define UDS_SID_ECU_RESET                       0x11U
#define UDS_SID_SECURITY_ACCESS                 0x27U
#define UDS_SID_COMMUNICATION_CONTROL           0x28U
#define UDS_SID_TESTER_PRESENT                  0x3EU
#define UDS_SID_CONTROL_DTC_SETTING             0x85U
#define UDS_SID_READ_DATA_BY_IDENTIFIER         0x22U
#define UDS_SID_READ_MEMORY_BY_ADDRESS          0x23U
#define UDS_SID_WRITE_DATA_BY_IDENTIFIER        0x2EU
#define UDS_SID_WRITE_MEMORY_BY_ADDRESS         0x3DU
#define UDS_SID_CLEAR_DIAGNOSTIC_INFORMATION    0x14U
#define UDS_SID_READ_DTC_INFORMATION            0x19U
#define UDS_SID_INPUT_OUTPUT_CONTROL            0x2FU
#define UDS_SID_ROUTINE_CONTROL                 0x31U
#define UDS_SID_REQUEST_DOWNLOAD                0x34U
#define UDS_SID_REQUEST_UPLOAD                  0x35U
#define UDS_SID_TRANSFER_DATA                   0x36U
#define UDS_SID_REQUEST_TRANSFER_EXIT           0x37U

/* Positive Response SID Offset */
#define UDS_POSITIVE_RESPONSE_OFFSET            0x40U

/* Negative Response Code (NRC) */
#define UDS_NRC_GENERAL_REJECT                  0x10U
#define UDS_NRC_SERVICE_NOT_SUPPORTED           0x11U
#define UDS_NRC_SUB_FUNCTION_NOT_SUPPORTED      0x12U
#define UDS_NRC_INCORRECT_MESSAGE_LENGTH        0x13U
#define UDS_NRC_CONDITIONS_NOT_CORRECT          0x22U
#define UDS_NRC_REQUEST_SEQUENCE_ERROR          0x24U
#define UDS_NRC_REQUEST_OUT_OF_RANGE            0x31U
#define UDS_NRC_SECURITY_ACCESS_DENIED          0x33U
#define UDS_NRC_INVALID_KEY                     0x35U
#define UDS_NRC_EXCEEDED_NUMBER_OF_ATTEMPTS     0x36U
#define UDS_NRC_REQUIRED_TIME_DELAY_NOT_EXPIRED 0x37U

/* Diagnostic Session Types */
#define UDS_SESSION_DEFAULT                     0x01U
#define UDS_SESSION_PROGRAMMING                 0x02U
#define UDS_SESSION_EXTENDED                    0x03U

/* ECU Reset Types */
#define UDS_RESET_HARD                          0x01U
#define UDS_RESET_KEY_OFF_ON                    0x02U
#define UDS_RESET_SOFT                          0x03U

/* Security Access Types */
#define UDS_SECURITY_REQUEST_SEED_LEVEL_1       0x01U
#define UDS_SECURITY_SEND_KEY_LEVEL_1           0x02U

/* Routine Control Types */
#define UDS_ROUTINE_START                       0x01U
#define UDS_ROUTINE_STOP                        0x02U
#define UDS_ROUTINE_REQUEST_RESULTS             0x03U

/* Data Identifiers (DID) Examples */
#define UDS_DID_VIN                             0xF190U
#define UDS_DID_ECU_SERIAL_NUMBER               0xF18CU
#define UDS_DID_SYSTEM_SUPPLIER_ID              0xF18AU
#define UDS_DID_ECU_SOFTWARE_NUMBER             0xF194U
#define UDS_DID_ECU_HARDWARE_NUMBER             0xF191U
#define UDS_DID_FINGERPRINT                     0xF15BU

/* UDS Message Structure */
typedef struct {
    uint8_t sid;
    const uint8_t *data;
    uint32_t length;
} uds_request_t;

typedef struct {
    uint8_t *buffer;
    uint32_t max_length;
    uint32_t actual_length;
} uds_response_t;

/* UDS Context */
typedef struct {
    uint8_t current_session;
    uint8_t security_level;
    bool security_unlocked;
    uint32_t seed;
    uint8_t failed_security_attempts;
    uint32_t last_tester_present_time;
} uds_context_t;

/* Function Prototypes */
void uds_init(uds_context_t *context);

bool uds_process_request(
    uds_context_t *context,
    const uds_request_t *request,
    uds_response_t *response
);

/* Service Handlers */
void uds_handle_diagnostic_session_control(
    uds_context_t *context,
    const uds_request_t *request,
    uds_response_t *response
);

void uds_handle_ecu_reset(
    uds_context_t *context,
    const uds_request_t *request,
    uds_response_t *response
);

void uds_handle_security_access(
    uds_context_t *context,
    const uds_request_t *request,
    uds_response_t *response
);

void uds_handle_tester_present(
    uds_context_t *context,
    const uds_request_t *request,
    uds_response_t *response
);

void uds_handle_read_data_by_id(
    uds_context_t *context,
    const uds_request_t *request,
    uds_response_t *response
);

void uds_handle_write_data_by_id(
    uds_context_t *context,
    const uds_request_t *request,
    uds_response_t *response
);

void uds_handle_routine_control(
    uds_context_t *context,
    const uds_request_t *request,
    uds_response_t *response
);

/* Utility Functions */
void uds_send_negative_response(
    uint8_t sid,
    uint8_t nrc,
    uds_response_t *response
);

void uds_send_positive_response(
    uint8_t sid,
    const uint8_t *data,
    uint32_t length,
    uds_response_t *response
);

#endif /* UDS_SERVICES_H */
