#ifndef DOIP_CONFIG_H
#define DOIP_CONFIG_H

/* Version Information */
#define DOIP_VERSION_MAJOR 1
#define DOIP_VERSION_MINOR 0
#define DOIP_VERSION_PATCH 0

/* Feature Configuration */
#define DOIP_ENABLE_ENTITY
//#define DOIP_ENABLE_TESTER
#define DOIP_USE_FREERTOS
#define DOIP_USE_LWIP

/* Buffer Size Configuration */
#ifndef DOIP_MAX_PAYLOAD_SIZE
#define DOIP_MAX_PAYLOAD_SIZE           (4096U)
#endif

#ifndef DOIP_RX_BUFFER_SIZE
#define DOIP_RX_BUFFER_SIZE             (4096U)
#endif

#ifndef DOIP_MAX_CONNECTIONS
#define DOIP_MAX_CONNECTIONS            (8U)
#endif

#ifndef DOIP_ANNOUNCEMENT_INTERVAL
#define DOIP_ANNOUNCEMENT_INTERVAL              (500U)
#endif

#ifndef DOIP_ANNOUNCEMENT_COUNT
#define DOIP_ANNOUNCEMENT_COUNT                 (3U)
#endif

/* Address Configuration */
#ifndef DOIP_TESTER_ADDRESS_MIN
#define DOIP_TESTER_ADDRESS_MIN                 (0x0E00U)
#endif

#ifndef DOIP_TESTER_ADDRESS_MAX
#define DOIP_TESTER_ADDRESS_MAX                 (0x0FFFU)
#endif

/* Protocol Version */
#ifndef DOIP_DEFAULT_PROTOCOL_VERSION
#define DOIP_DEFAULT_PROTOCOL_VERSION           (0x03U)  /* ISO 13400-2:2019 */
#endif

/* Debugging */
#ifdef DOIP_DEBUG
#include <stdio.h>
#define DOIP_LOG(fmt, ...) printf("[DoIP] " fmt "\n", ##__VA_ARGS__)
#else
#define DOIP_LOG(fmt, ...)
#endif

#endif /* DOIP_CONFIG_H */
