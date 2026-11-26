#include "protocol.h"

int init_sockets(void) {
#if defined WIN32 || defined _WIN32
    WSADATA wsa_data;
    int result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
    if (result != 0) {
        error_handler("Errore in WSAStartup().");
        return -1;
    }
#else
    /* Evita che una write su socket chiusa dal client termini il server */
    signal(SIGPIPE, SIG_IGN);
#endif
    return 0;
}

void cleanup_sockets(void) {
#if defined WIN32 || defined _WIN32
    WSACleanup();
#endif
}

void error_handler(const char *error_message) {
    printf("%s\n", error_message);
}

int parse_ipv4_address(const char *ip_str, struct in_addr *out_addr) {
    if (ip_str == NULL || out_addr == NULL) {
        return -1;
    }

    unsigned int b1 = 0, b2 = 0, b3 = 0, b4 = 0;
    int consumed = 0;

    if (sscanf(ip_str, "%3u.%3u.%3u.%3u%n", &b1, &b2, &b3, &b4, &consumed) != 4) {
        return -1;
    }

    if (ip_str[consumed] != '\0') {
        return -1;
    }

    if (b1 > 255 || b2 > 255 || b3 > 255 || b4 > 255) {
        return -1;
    }

    for (int i = 0; i < consumed; i++) {
        if ((ip_str[i] < '0' || ip_str[i] > '9') && ip_str[i] != '.') {
            return -1;
        }
    }

    uint32_t host_ip = ((uint32_t)b1 << 24) |
                       ((uint32_t)b2 << 16) |
                       ((uint32_t)b3 << 8)  |
                       (uint32_t)b4;
    out_addr->s_addr = htonl(host_ip);
    return 0;
}

int parse_port(const char *port_str, uint16_t *out_port) {
    if (port_str == NULL || out_port == NULL || *port_str == '\0') {
        return -1;
    }

    if (*port_str < '0' || *port_str > '9') {
        return -1;
    }

    char *endptr = NULL;
    errno = 0;
    long val = strtol(port_str, &endptr, 10);

    if (errno != 0 || *endptr != '\0' || val < 1 || val > 65535) {
        return -1;
    }

    *out_port = (uint16_t)val;
    return 0;
}

int send_all(int sock, const char *buf, int len) {
    int total = 0;
    while (total < len) {
        int sent = send(sock, buf + total, len - total, 0);
        if (sent <= 0) {
            return -1;
        }
        total += sent;
    }
    return total;
}

int recv_all(int sock, char *buf, int len) {
    int total = 0;
    while (total < len) {
        int received = recv(sock, buf + total, len - total, 0);
        if (received <= 0) {
            return -1;
        }
        total += received;
    }
    return total;
}

int send_string(int sock, const char *str) {
    int len = (int)strlen(str) + 1;
    return send_all(sock, str, len);
}

int recv_string(int sock, char *buf, int max_len) {
    int i = 0;
    char ch = '\0';
    while (i < max_len - 1) {
        int n = recv(sock, &ch, 1, 0);
        if (n <= 0) {
            return -1;
        }
        buf[i++] = ch;
        if (ch == '\0') {
            return i;
        }
    }
    buf[max_len - 1] = '\0';
    return i;
}

int send_numbers(int sock, int32_t n1, int32_t n2, int32_t n3) {
    int32_t net_nums[3];
    net_nums[0] = (int32_t)htonl((uint32_t)n1);
    net_nums[1] = (int32_t)htonl((uint32_t)n2);
    net_nums[2] = (int32_t)htonl((uint32_t)n3);
    return send_all(sock, (const char *)net_nums, (int)sizeof(net_nums));
}

int recv_numbers(int sock, int32_t *n1, int32_t *n2, int32_t *n3) {
    int32_t net_nums[3];
    if (recv_all(sock, (char *)net_nums, (int)sizeof(net_nums)) < 0) {
        return -1;
    }
    *n1 = (int32_t)ntohl((uint32_t)net_nums[0]);
    *n2 = (int32_t)ntohl((uint32_t)net_nums[1]);
    *n3 = (int32_t)ntohl((uint32_t)net_nums[2]);
    return 0;
}
