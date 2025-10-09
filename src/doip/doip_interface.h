#ifndef DOIP_INTERFACE_H
#define DOIP_INTERFACE_H

#include "doip_protocol.h"

#define DOIP_UDP_DISCOVERY_PORT    13400U
#define DOIP_TCP_DATA_PORT         13400U
#define DOIP_MAX_CONNECTIONS       2U//8U
#define DOIP_RX_BUFFER_SIZE        4096U

/* Network Operations Abstraction */
typedef struct {
    int (*udp_bind)(uint16_t port);
    int (*udp_sendto)(int sock, const uint8_t *data, uint32_t len, 
                      const char *ip, uint16_t port);
    int (*udp_recvfrom)(int sock, uint8_t *buf, uint32_t len, 
                        char *src_ip, uint16_t *src_port);
    int (*tcp_listen)(uint16_t port);
    int (*tcp_accept)(int listen_sock);
    int (*tcp_connect)(const char *ip, uint16_t port);
    int (*tcp_send)(int sock, const uint8_t *data, uint32_t len);
    int (*tcp_recv)(int sock, uint8_t *buf, uint32_t len);
    void (*close_socket)(int sock);
    int (*socket_select)(int max_fd, uint32_t timeout_ms);
} doip_network_ops_t;

/* Connection State */
typedef enum {
    DOIP_CONN_STATE_CLOSED = 0,
    DOIP_CONN_STATE_LISTEN,
    DOIP_CONN_STATE_PENDING_ACTIVATION,
    DOIP_CONN_STATE_ACTIVATED,
    DOIP_CONN_STATE_FINALIZE
} doip_connection_state_t;

/* TCP Connection Context */
typedef struct {
    int socket_fd;
    doip_connection_state_t state;
    uint16_t source_address;
    uint32_t last_activity_time;
    uint8_t rx_buffer[DOIP_RX_BUFFER_SIZE];
    uint32_t rx_buffer_used;
} doip_tcp_connection_t;

/* Network Interface Context */
typedef struct {
    doip_network_ops_t *net_ops;
    int udp_socket;
    int tcp_listen_socket;
    doip_tcp_connection_t connections[DOIP_MAX_CONNECTIONS];
    uint8_t udp_rx_buffer[DOIP_RX_BUFFER_SIZE];
} doip_interface_t;

/* Callback Types */
typedef void (*doip_udp_rx_callback_t)(
    const char *src_ip,
    uint16_t src_port,
    const uint8_t *data,
    uint32_t length,
    void *user_data
);

typedef void (*doip_tcp_rx_callback_t)(
    int connection_id,
    const uint8_t *data,
    uint32_t length,
    void *user_data
);

typedef void (*doip_tcp_connected_callback_t)(
    int connection_id,
    void *user_data
);

typedef void (*doip_tcp_disconnected_callback_t)(
    int connection_id,
    void *user_data
);

/* Function Prototypes */
doip_result_t doip_interface_init(
    doip_interface_t *interface,
    doip_network_ops_t *net_ops
);

doip_result_t doip_interface_start_udp(
    doip_interface_t *interface,
    uint16_t port
);

doip_result_t doip_interface_start_tcp_server(
    doip_interface_t *interface,
    uint16_t port
);

doip_result_t doip_interface_udp_broadcast(
    doip_interface_t *interface,
    const uint8_t *data,
    uint32_t length,
    uint16_t port
);

doip_result_t doip_interface_udp_send(
    doip_interface_t *interface,
    const char *dest_ip,
    uint16_t dest_port,
    const uint8_t *data,
    uint32_t length
);

doip_result_t doip_interface_tcp_send(
    doip_interface_t *interface,
    int connection_id,
    const uint8_t *data,
    uint32_t length
);

doip_result_t doip_interface_process(
    doip_interface_t *interface,
    doip_udp_rx_callback_t udp_callback,
    doip_tcp_rx_callback_t tcp_callback,
    doip_tcp_connected_callback_t connected_callback,
    doip_tcp_disconnected_callback_t disconnected_callback,
    void *user_data
);

void doip_interface_close_connection(
    doip_interface_t *interface,
    int connection_id
);

#endif /* DOIP_INTERFACE_H */
