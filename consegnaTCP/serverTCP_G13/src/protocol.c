#include "protocol.h"

int init_sockets(void) {
#if defined WIN32 || defined _WIN32
    WSADATA wsa_data;
    int result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
    if (result != 0) {
        error_handler("Errore in WSAStartup().");
        return -1;
    }
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
    int len = (int)strlen(str) + 1; /* Include il terminatore '\0' */
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
