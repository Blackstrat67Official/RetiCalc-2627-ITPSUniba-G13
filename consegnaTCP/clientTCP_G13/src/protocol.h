#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#if defined WIN32 || defined _WIN32
#include <winsock2.h>
typedef int socklen_t;
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#define closesocket close
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define DEFAULT_PORT 5193
#define QLEN 5
#define BUF_SIZE 256

#define WELCOME_MSG "connessione avvenuta"
#define QUIT_MSG "QUIT"

/* Inizializzazione e pulizia libreria socket (portabile Windows/POSIX) */
int init_sockets(void);
void cleanup_sockets(void);
void error_handler(const char *error_message);

/* Funzioni di trasporto affidabile su stream TCP */
int send_all(int sock, const char *buf, int len);
int recv_all(int sock, char *buf, int len);

/* Funzioni applicative del protocollo */
int send_string(int sock, const char *str);
int recv_string(int sock, char *buf, int max_len);
int send_numbers(int sock, int32_t n1, int32_t n2, int32_t n3);
int recv_numbers(int sock, int32_t *n1, int32_t *n2, int32_t *n3);

#endif /* PROTOCOL_H_ */
