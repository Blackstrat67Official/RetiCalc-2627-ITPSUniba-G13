#include "protocol.h"

int main(void) {
    if (init_sockets() < 0) {
        return -1;
    }

    char server_ip[64];
    int port = 0;

    /* 2) Il client legge da tastiera l'IP dell'host e il numero di porta */
    printf("Inserisci l'indirizzo IP del server (es. 127.0.0.1): ");
    if (scanf("%63s", server_ip) != 1) {
        error_handler("Indirizzo IP non valido.");
        cleanup_sockets();
        return -1;
    }

    printf("Inserisci il numero di porta del server: ");
    if (scanf("%d", &port) != 1 || port <= 0 || port > 65535) {
        error_handler("Numero di porta non valido.");
        cleanup_sockets();
        return -1;
    }

    /* Creazione della socket */
    int c_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (c_socket < 0) {
        error_handler("Creazione della socket fallita.");
        cleanup_sockets();
        return -1;
    }

    struct sockaddr_in sad;
    memset(&sad, 0, sizeof(sad));
    sad.sin_family = AF_INET;
    sad.sin_addr.s_addr = inet_addr(server_ip);
    sad.sin_port = htons((uint16_t)port);

    if (sad.sin_addr.s_addr == INADDR_NONE) {
        error_handler("Formato indirizzo IP non valido.");
        closesocket(c_socket);
        cleanup_sockets();
        return -1;
    }

    /* 3) Il client richiede la connessione al server */
    if (connect(c_socket, (struct sockaddr *)&sad, sizeof(sad)) < 0) {
        error_handler("Connessione al server fallita.");
        closesocket(c_socket);
        cleanup_sockets();
        return -1;
    }

    /* 6) Il client riceve la stringa "connessione avvenuta" e la stampa */
    char buf[BUF_SIZE];
    if (recv_string(c_socket, buf, BUF_SIZE) < 0) {
        error_handler("Errore nella ricezione del messaggio dal server.");
        closesocket(c_socket);
        cleanup_sockets();
        return -1;
    }
    printf("%s\n", buf);

    /* Ciclo di interazione: passi 7 e 10 */
    while (1) {
        int n1 = 0, n2 = 0, n3 = 0;

        /* 7) Il client legge dallo std input tre interi n1 n2 ed n3 e li invia */
        printf("Inserisci tre interi (n1 n2 n3): ");
        if (scanf("%d %d %d", &n1, &n2, &n3) != 3) {
            error_handler("Input non valido: inserire tre numeri interi.");
            int c;
            while ((c = getchar()) != '\n' && c != EOF) {}
            continue;
        }

        if (send_numbers(c_socket, (int32_t)n1, (int32_t)n2, (int32_t)n3) < 0) {
            error_handler("Errore durante l'invio dei dati al server.");
            break;
        }

        /* 10) Il client riceve la stringa dal server e la visualizza */
        if (recv_string(c_socket, buf, BUF_SIZE) < 0) {
            error_handler("Connessione chiusa dal server in modo inatteso.");
            break;
        }

        printf("%s\n", buf);

        /* Se la stringa corrisponde a "QUIT" chiude la connessione e termina */
        if (strcmp(buf, QUIT_MSG) == 0) {
            break;
        }
    }

    closesocket(c_socket);
    cleanup_sockets();
    return 0;
}
