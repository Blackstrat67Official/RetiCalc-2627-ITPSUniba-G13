#include "protocol.h"

static void handle_client(int client_socket) {
    /* 5) Il server invia al client la stringa "connessione avvenuta" */
    if (send_string(client_socket, WELCOME_MSG) < 0) {
        error_handler("Errore nell'invio del messaggio di benvenuto.");
        return;
    }

    while (1) {
        int32_t n1 = 0, n2 = 0, n3 = 0;

        /* 8) Il server riceve i tre numeri */
        if (recv_numbers(client_socket, &n1, &n2, &n3) < 0) {
            printf("Client disconnesso.\n");
            break;
        }

        int32_t res = n1 + n2 - n3;

        /* Visualizza sullo std output "la somma di n1+n2-n3=res" */
        printf("la somma di %d+%d-%d=%d\n", n1, n2, n3, res);

        /* 9) Se res = 100 invia "QUIT", altrimenti invia il risultato res */
        char response[BUF_SIZE];
        if (res == 100) {
            snprintf(response, sizeof(response), "%s", QUIT_MSG);
        } else {
            snprintf(response, sizeof(response), "%d", res);
        }

        if (send_string(client_socket, response) < 0) {
            error_handler("Errore nell'invio della risposta al client.");
            break;
        }

        if (res == 100) {
            break;
        }
    }
}

int main(int argc, char *argv[]) {
    int port = DEFAULT_PORT;
    if (argc > 1) {
        int parsed_port = atoi(argv[1]);
        if (parsed_port > 0 && parsed_port <= 65535) {
            port = parsed_port;
        } else {
            printf("Porta non valida. Uso porta di default %d.\n", DEFAULT_PORT);
        }
    }

    if (init_sockets() < 0) {
        return -1;
    }

    /* Creazione della socket */
    int server_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_socket < 0) {
        error_handler("Creazione della socket fallita.");
        cleanup_sockets();
        return -1;
    }

    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt, sizeof(opt));

    /* Binding dell'indirizzo */
    struct sockaddr_in sad;
    memset(&sad, 0, sizeof(sad));
    sad.sin_family = AF_INET;
    sad.sin_addr.s_addr = INADDR_ANY;
    sad.sin_port = htons((uint16_t)port);

    if (bind(server_socket, (struct sockaddr *)&sad, sizeof(sad)) < 0) {
        error_handler("bind() fallita.");
        closesocket(server_socket);
        cleanup_sockets();
        return -1;
    }

    /* 1) Il server e' avviato su una porta a scelta e resta sempre in ascolto */
    if (listen(server_socket, QLEN) < 0) {
        error_handler("listen() fallita.");
        closesocket(server_socket);
        cleanup_sockets();
        return -1;
    }

    printf("Server in ascolto sulla porta %d...\n", port);

    while (1) {
        struct sockaddr_in cad;
        socklen_t client_len = sizeof(cad);

        int client_socket = accept(server_socket, (struct sockaddr *)&cad, &client_len);
        if (client_socket < 0) {
            error_handler("accept() fallita.");
            continue;
        }

        /* 4) Visualizza sullo std output l'IP del client connesso */
        printf("connesso con il client il cui nome IP: %s\n", inet_ntoa(cad.sin_addr));

        handle_client(client_socket);

        /* 11) Chiude la connessione con il singolo client e rimane in ascolto */
        closesocket(client_socket);
    }

    closesocket(server_socket);
    cleanup_sockets();
    return 0;
}
