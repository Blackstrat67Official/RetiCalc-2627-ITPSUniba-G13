# Esonero Reti di Calcolatori a.a. 2026-27 - I Assegnazione (Socket TCP)

- **Gruppo:** G13
- **Componenti:**
  - Palmieri Davide (865810)
  - Lusha Endri (865525)
- **Sistema Operativo utilizzato:** Windows 11 (MinGW-w64 GCC)
- **Ambiente di sviluppo:** Eclipse IDE for C/C++ Developers / Visual Studio Code

## Architettura dei progetti
- `serverTCP_G13/src/`:
  - `protocol.h` / `protocol.c`: livello di astrazione portabile Windows/POSIX, validazione IPv4/porte e funzioni `send_all` / `recv_all` per gestire il flusso TCP.
  - `server.c`: server TCP iterativo in ascolto permanente (porta di default `5193` o passata da riga di comando).
- `clientTCP_G13/src/`:
  - `protocol.h` / `protocol.c`: modulo condiviso di rete e serializzazione in *network byte order*.
  - `client.c`: client TCP con validazione dell'input utente e ciclo di invio/ricezione fino alla risposta `QUIT` (`n1 + n2 - n3 = 100`).

## Compilazione da terminale (Windows MinGW)
```bash
gcc serverTCP_G13/src/server.c serverTCP_G13/src/protocol.c -o server.exe -lws2_32
gcc clientTCP_G13/src/client.c clientTCP_G13/src/protocol.c -o client.exe -lws2_32
