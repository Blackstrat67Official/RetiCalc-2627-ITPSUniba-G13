#include "protocol.h"

/* Legge una riga da stdin rimuovendo '\r' e '\n' */
static int read_line(char *buf, size_t size) {
    if (fgets(buf, (int)size, stdin) == NULL) {
        return -1;
    }
    size_t len = strcspn(buf, "\r\n");
    if (buf[len] == '\0' && len == size - 1) {
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF) {}
    }
    buf[len] = '\0';
    return 0;
}

/* Estrae esattamente 3 interi a 32 bit dalla riga inserita */
static int parse_three_integers(const char *line, int32_t *n1, int32_t *n2, int32_t *n3) {
    const char *ptr = line;
    int32_t values[3];

    for (int i = 0; i < 3; i++) {
        while (*ptr == ' ' || *ptr == '\t') {
            ptr++;
        }
        if (*ptr == '\0') {
            return -1;
        }

        char *endptr = NULL;
        errno = 0;
        long val = strtol(ptr, &endptr, 10);

        if (errno == ERANGE || endptr == ptr || val < INT32_MIN || val > INT32_MAX) {
            return -1;
        }

        values[i] = (int32_t)val;
        ptr = endptr;
    }

    while (*ptr == ' ' || *ptr == '\t') {
        ptr++;
    }
    if (*ptr != '\0') {
        return -1;
    }

    *n1 = values[0];
    *n2 = values[1];
    *n3 = values[2];
    return 0;
}

int main(void) {
    if (init_sockets() < 0) {
        return -1;
    }

    char input_buf[BUF_SIZE];
    struct sockaddr_in sad;
    memset(&sad, 0, sizeof(sad));
    sad.sin_family = AF_INET;

    /* 2) Il client legge da tastiera l'IP dell'host e il numero di porta */
    printf("Inserisci l'indirizzo IP del server (es. 127.0.0.1): ");
    if (read_line(input_buf, sizeof(input_buf)) < 0 ||
        parse_ipv4_address(input_buf, &sad.sin_addr) < 0) {
        error_handler("Indirizzo IP non valido.");
        cleanup_sockets();
        return -1;
    }

    uint16_t port = 0;
    printf("Inserisci il numero di porta del server: ");
    if (read_line(input_buf, sizeof(input_buf)) < 0 ||
        parse_port(input_buf, &port) < 0) {
        error_handler("Numero di porta non valido.");
        cleanup_sockets();
        return -1;
    }
    sad.sin_port = htons(port);

    int c_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (c_socket < 0) {
        error_handler("Creazione della socket fallita.");
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
        int32_t n1 = 0, n2 = 0, n3 = 0;

        /* 7) Il client legge dallo std input tre interi n1 n2 ed n3 e li invia */
        printf("Inserisci tre interi (n1 n2 n3): ");
        if (read_line(input_buf, sizeof(input_buf)) < 0) {
            error_handler("Fine input rilevata, chiusura client.");
            break;
        }

        if (parse_three_integers(input_buf, &n1, &n2, &n3) < 0) {
            error_handler("Input non valido: inserire esattamente tre numeri interi validi.");
            continue;
        }

        if (send_numbers(c_socket, n1, n2, n3) < 0) {
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
