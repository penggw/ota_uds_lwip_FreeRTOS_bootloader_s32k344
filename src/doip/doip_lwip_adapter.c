#include "doip_interface.h"
#include "doip_lwip_adapter.h"
#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "debug_print.h"
#include <string.h>

static int lwip_udp_bind(uint16_t port)
{
    int sock;
    struct sockaddr_in addr;
    int opt = 1;

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        return -1;
    }

    /* Enable broadcast */
    setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &opt, sizeof(opt));

    /* Set non-blocking */
    int flags = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, flags | O_NONBLOCK);

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        close(sock);
        return -1;
    }

    return sock;
}

static int lwip_udp_sendto(
    int sock,
    const uint8_t *data,
    uint32_t len,
    const char *ip,
    uint16_t port)
{
    struct sockaddr_in dest_addr;

    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &dest_addr.sin_addr);

    return sendto(sock, data, len, 0,
                  (struct sockaddr *)&dest_addr, sizeof(dest_addr));
}

static int lwip_udp_recvfrom(
    int sock,
    uint8_t *buf,
    uint32_t len,
    char *src_ip,
    uint16_t *src_port)
{
    struct sockaddr_in src_addr;
    socklen_t addr_len = sizeof(src_addr);
    int ret;

    ret = recvfrom(sock, buf, len, 0,
                   (struct sockaddr *)&src_addr, &addr_len);

    if (ret > 0) {
        inet_ntop(AF_INET, &src_addr.sin_addr, src_ip, 16);
        *src_port = ntohs(src_addr.sin_port);
    }

    return ret;
}

static int lwip_tcp_listen(uint16_t port)
{
    int sock;
    struct sockaddr_in addr;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        return -1;
    }

    /* Set non-blocking */
    int flags = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, flags | O_NONBLOCK);

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        close(sock);
        return -1;
    }

    if (listen(sock, 5) < 0) {
        close(sock);
        return -1;
    }

    return sock;
}

static int lwip_tcp_accept(int listen_sock)
{
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    int client_sock;

    client_sock = accept(listen_sock, (struct sockaddr *)&client_addr, &addr_len);

    if (client_sock >= 0) {
        /* Set non-blocking */
        int flags = fcntl(client_sock, F_GETFL, 0);
        fcntl(client_sock, F_SETFL, flags | O_NONBLOCK);
    }

    return client_sock;
}

static int lwip_tcp_connect(const char *ip, uint16_t port)
{
    int sock;
    struct sockaddr_in addr;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        return -1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &addr.sin_addr);

    if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        close(sock);
        return -1;
    }

    return sock;
}

static int lwip_tcp_send(int sock, const uint8_t *data, uint32_t len)
{
    return send(sock, data, len, 0);
}

static int lwip_tcp_recv(int sock, uint8_t *buf, uint32_t len)
{
    return recv(sock, buf, len, 0);
}

static void lwip_close_socket(int sock)
{
    close(sock);
}

doip_result_t doip_lwip_adapter_init(void)
{
    /* Network operations are static, no dynamic initialization needed */
    return DOIP_RESULT_OK;
}

/* Global network operations structure */
doip_network_ops_t g_lwip_net_ops = {
    .udp_bind = lwip_udp_bind,
    .udp_sendto = lwip_udp_sendto,
    .udp_recvfrom = lwip_udp_recvfrom,
    .tcp_listen = lwip_tcp_listen,
    .tcp_accept = lwip_tcp_accept,
    .tcp_connect = lwip_tcp_connect,
    .tcp_send = lwip_tcp_send,
    .tcp_recv = lwip_tcp_recv,
    .close_socket = lwip_close_socket
};
