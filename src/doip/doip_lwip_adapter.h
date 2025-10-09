#ifndef DOIP_LWIP_ADAPTER_H
#define DOIP_LWIP_ADAPTER_H

#include <stddef.h>
#include "doip_interface.h"

/**
 * @file doip_lwip_adapter.h
 * @brief lwIP TCP/IP Stack Adapter for DoIP Protocol
 *
 * This adapter provides integration between the DoIP network interface
 * abstraction and the lwIP TCP/IP stack, supporting both FreeRTOS and
 * bare-metal environments.
 */

/* lwIP Network Operations Structure */
extern doip_network_ops_t g_lwip_net_ops;

/**
 * @brief Initialize lwIP adapter
 * @return DOIP_RESULT_OK on success
 */
doip_result_t doip_lwip_adapter_init(void);

/**
 * @brief Set socket to non-blocking mode
 * @param sock Socket descriptor
 * @return 0 on success, -1 on failure
 */
int doip_lwip_set_nonblocking(int sock);

/**
 * @brief Set socket options for DoIP communication
 * @param sock Socket descriptor
 * @param is_tcp true for TCP socket, false for UDP
 * @return 0 on success, -1 on failure
 */
int doip_lwip_set_socket_options(int sock, bool is_tcp);

/**
 * @brief Enable broadcast on UDP socket
 * @param sock Socket descriptor
 * @return 0 on success, -1 on failure
 */
int doip_lwip_enable_broadcast(int sock);

/**
 * @brief Get local IP address
 * @param interface_name Network interface name (e.g., "eth0")
 * @param ip_buffer Buffer to store IP address string
 * @param buffer_size Size of ip_buffer
 * @return 0 on success, -1 on failure
 */
int doip_lwip_get_local_ip(const char *interface_name, char *ip_buffer, size_t buffer_size);

#endif /* DOIP_LWIP_ADAPTER_H */
