#include "doip_interface.h"
#include <string.h>
#include <errno.h>
#include "debug_print.h"

#define INVALID_SOCKET  (-1)

doip_result_t doip_interface_init(
    doip_interface_t *interface,
    doip_network_ops_t *net_ops)
{
    uint32_t i;
    
    if ((interface == NULL) || (net_ops == NULL)) {
        return DOIP_RESULT_INVALID_PARAM;
    }
    
    interface->net_ops = net_ops;
    interface->udp_socket = INVALID_SOCKET;
    interface->tcp_listen_socket = INVALID_SOCKET;
    
    /* Initialize all connections */
    for (i = 0U; i < DOIP_MAX_CONNECTIONS; i++) {
        interface->connections[i].socket_fd = INVALID_SOCKET;
        interface->connections[i].state = DOIP_CONN_STATE_CLOSED;
        interface->connections[i].source_address = 0U;
        interface->connections[i].last_activity_time = 0U;
        interface->connections[i].rx_buffer_used = 0U;
    }
    
    return DOIP_RESULT_OK;
}

doip_result_t doip_interface_start_udp(
    doip_interface_t *interface,
    uint16_t port)
{
    if (interface == NULL) {
        return DOIP_RESULT_INVALID_PARAM;
    }
    
    interface->udp_socket = interface->net_ops->udp_bind(port);
    if (interface->udp_socket < 0) {
        return DOIP_RESULT_ERROR;
    }
    
    return DOIP_RESULT_OK;
}

doip_result_t doip_interface_start_tcp_server(
    doip_interface_t *interface,
    uint16_t port)
{
    if (interface == NULL) {
        return DOIP_RESULT_INVALID_PARAM;
    }
    
    interface->tcp_listen_socket = interface->net_ops->tcp_listen(port);
    if (interface->tcp_listen_socket < 0) {
        return DOIP_RESULT_ERROR;
    }
    
    return DOIP_RESULT_OK;
}

doip_result_t doip_interface_udp_broadcast(
    doip_interface_t *interface,
    const uint8_t *data,
    uint32_t length,
    uint16_t port)
{
    int result;
    
    if ((interface == NULL) || (data == NULL)) {
        return DOIP_RESULT_INVALID_PARAM;
    }
    
    result = interface->net_ops->udp_sendto(
        interface->udp_socket,
        data,
        length,
        "255.255.255.255",
        port
    );
    
    return (result >= 0) ? DOIP_RESULT_OK : DOIP_RESULT_ERROR;
}

doip_result_t doip_interface_udp_send(
    doip_interface_t *interface,
    const char *dest_ip,
    uint16_t dest_port,
    const uint8_t *data,
    uint32_t length)
{
    int result;
    
    if ((interface == NULL) || (dest_ip == NULL) || (data == NULL)) {
        return DOIP_RESULT_INVALID_PARAM;
    }
    
    result = interface->net_ops->udp_sendto(
        interface->udp_socket,
        data,
        length,
        dest_ip,
        dest_port
    );
    
    return (result >= 0) ? DOIP_RESULT_OK : DOIP_RESULT_ERROR;
}

doip_result_t doip_interface_tcp_send(
    doip_interface_t *interface,
    int connection_id,
    const uint8_t *data,
    uint32_t length)
{
    int result;
    
    if ((interface == NULL) || (data == NULL)) {
        return DOIP_RESULT_INVALID_PARAM;
    }
    
    if ((connection_id < 0) || (connection_id >= (int)DOIP_MAX_CONNECTIONS)) {
        return DOIP_RESULT_INVALID_PARAM;
    }
    
    if (interface->connections[connection_id].socket_fd < 0) {
        return DOIP_RESULT_NOT_READY;
    }
    
    result = interface->net_ops->tcp_send(
        interface->connections[connection_id].socket_fd,
        data,
        length
    );
    
    return (result >= 0) ? DOIP_RESULT_OK : DOIP_RESULT_ERROR;
}

static int find_free_connection(doip_interface_t *interface)
{
    uint32_t i;
    
    for (i = 0U; i < DOIP_MAX_CONNECTIONS; i++) {
        if (interface->connections[i].state == DOIP_CONN_STATE_CLOSED) {
            return (int)i;
        }
    }
    
    return INVALID_SOCKET;
}

doip_result_t doip_interface_process(
    doip_interface_t *interface,
    doip_udp_rx_callback_t udp_callback,
    doip_tcp_rx_callback_t tcp_callback,
    doip_tcp_connected_callback_t connected_callback,
    doip_tcp_disconnected_callback_t disconnected_callback,
    void *user_data)
{
    uint32_t i;
    int bytes_received;
    char src_ip[16];
    uint16_t src_port;
    
    if (interface == NULL) {
        return DOIP_RESULT_INVALID_PARAM;
    }
    
    /* Process UDP reception */
    if ((interface->udp_socket >= 0) && (udp_callback != NULL)) {
        bytes_received = interface->net_ops->udp_recvfrom(
            interface->udp_socket,
            interface->udp_rx_buffer,
            DOIP_RX_BUFFER_SIZE,
            src_ip,
            &src_port
        );
        
        if (bytes_received > 0) {
            udp_callback(src_ip, src_port, interface->udp_rx_buffer,
                        (uint32_t)bytes_received, user_data);
        }
    }
    
    /* Accept new TCP connections */
    if (interface->tcp_listen_socket >= 0) {
        int new_socket = interface->net_ops->tcp_accept(
            interface->tcp_listen_socket
        );
        
        if (new_socket >= 0) {
            int conn_id = find_free_connection(interface);
            if (conn_id >= 0) {
                interface->connections[conn_id].socket_fd = new_socket;
                interface->connections[conn_id].state = DOIP_CONN_STATE_PENDING_ACTIVATION;
                interface->connections[conn_id].rx_buffer_used = 0U;
                interface->connections[conn_id].last_activity_time = 0U; /* Should use actual time */
                
                if (connected_callback != NULL) {
                    connected_callback(conn_id, user_data);
                }
            } else {
                /* No free connection slot */
                interface->net_ops->close_socket(new_socket);
            }
        }
    }
    
    /* Process existing TCP connections */
    for (i = 0U; i < DOIP_MAX_CONNECTIONS; i++) {
        if (interface->connections[i].socket_fd >= 0) {
            bytes_received = interface->net_ops->tcp_recv(
                interface->connections[i].socket_fd,
                &interface->connections[i].rx_buffer[
                    interface->connections[i].rx_buffer_used],
                DOIP_RX_BUFFER_SIZE - interface->connections[i].rx_buffer_used
            );
            
            if (bytes_received > 0) {
                interface->connections[i].rx_buffer_used += (uint32_t)bytes_received;
                
                /* Process complete DoIP messages */
                while (interface->connections[i].rx_buffer_used >= 8U) {
                    doip_header_t header;
                    uint32_t total_message_size;
                    
                    if (doip_decode_header(
                            interface->connections[i].rx_buffer,
                            interface->connections[i].rx_buffer_used,
                            &header) != DOIP_RESULT_OK) {
                        break;
                    }
                    
                    total_message_size = 8U + header.payload_length;
                    
                    if (interface->connections[i].rx_buffer_used >= total_message_size) {
                        /* Complete message received */
                        if (tcp_callback != NULL) {
                            tcp_callback((int)i,
                                       interface->connections[i].rx_buffer,
                                       total_message_size,
                                       user_data);
                        }
                        
                        /* Remove processed message from buffer */
                        if (interface->connections[i].rx_buffer_used > total_message_size) {
                            (void)memmove(
                                interface->connections[i].rx_buffer,
                                &interface->connections[i].rx_buffer[total_message_size],
                                interface->connections[i].rx_buffer_used - total_message_size
                            );
                        }
                        interface->connections[i].rx_buffer_used -= total_message_size;
                    } else {
                        /* Incomplete message, wait for more data */
                        break;
                    }
                }
            } else if (bytes_received == 0) {
                /* Connection closed */
                if (disconnected_callback != NULL) {
                    disconnected_callback((int)i, user_data);
                }

                interface->net_ops->close_socket(interface->connections[i].socket_fd);
                interface->connections[i].socket_fd = INVALID_SOCKET;
                interface->connections[i].state = DOIP_CONN_STATE_CLOSED;
                interface->connections[i].rx_buffer_used = 0U;
            } else {
                /* Error or would block - continue */
                /* For non-blocking sockets, EAGAIN/EWOULDBLOCK means no data available */
                /* Remove immediate disconnection detection here, let inactivity timer handle it */
            }
        }
    }

    return DOIP_RESULT_OK;
}

void doip_interface_close_connection(
    doip_interface_t *interface,
    int connection_id)
{
    if ((interface == NULL) || (connection_id < 0) ||
        (connection_id >= (int)DOIP_MAX_CONNECTIONS)) {
        return;
    }
    
    if (interface->connections[connection_id].socket_fd >= 0) {
        interface->net_ops->close_socket(
            interface->connections[connection_id].socket_fd
        );
        interface->connections[connection_id].socket_fd = INVALID_SOCKET;
        interface->connections[connection_id].state = DOIP_CONN_STATE_CLOSED;
        interface->connections[connection_id].rx_buffer_used = 0U;
    }
}
