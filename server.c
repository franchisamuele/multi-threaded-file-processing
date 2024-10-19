#include "util.h"
#include "unboundedqueue.h"
#include "functions.h"

#define MAX_CLIENTS 100

void cleanup() {
    printf("Chiusura server\n");
    CHECK_ERR_EQ(unlink("/tmp/635112socket"), -1, "unlink");
    exit(0);
}

void initSignalHandler() {
    sigset_t set;
    sigfillset(&set);
    CHECK_ERR_EQ(pthread_sigmask(SIG_SETMASK, &set, NULL), -1, "pthread_sigmask");

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = cleanup;
    CHECK_ERR_EQ(sigaction(SIGINT, &sa, NULL), -1, "sigaction");

	CHECK_ERR_EQ(pthread_sigmask(SIG_UNBLOCK, &set, NULL), -1, "pthread_sigmask");
}

// Collector
int main() {
    initSignalHandler();
    
    int serverfd;
    // Crea il socket
    CHECK_ERR_EQ(serverfd = socket(AF_UNIX, SOCK_STREAM, 0), -1, "Creating the socket");

    struct sockaddr_un serverAddr;
    serverAddr.sun_family = AF_UNIX;
    strcpy(serverAddr.sun_path, "/tmp/635112socket");

    // Associa un indirizzo al socket
    CHECK_ERR_EQ(bind(serverfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)), -1, "Bind error");

    // Il server si mette in ascolto sul socket (max 10 connessioni in coda)
    CHECK_ERR_EQ(listen(serverfd, 10), -1, "Listening");

    fd_set allFDs, readFDs;
    FD_ZERO(&allFDs);
    FD_SET(serverfd, &allFDs);
    int fdMax = serverfd;

    int clients[MAX_CLIENTS];
    for (int i = 0; i < MAX_CLIENTS; i++) {
        // File descriptor aperto se != -1
        clients[i] = -1;
    }

    printf("%s %12s %12s %20s\n-------------------------------------------------------------------------------------------------------------\n", "n", "avg", "std", "filename");
    for (;;) {
        readFDs = allFDs;
        CHECK_ERR_EQ(select(fdMax+1, &readFDs, NULL, NULL, NULL), -1, "Select");

        if (FD_ISSET(serverfd, &readFDs)) {
            // Un nuovo cliente si è connesso
            int clientfd;
            CHECK_ERR_EQ(clientfd = accept(serverfd, NULL, NULL), -1, "accept")

            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (clients[i] == -1) {
                    clients[i] = clientfd;
                    break;
                }
                if (i == MAX_CLIENTS-1) {
                    printf("Error: too many clients");
                    exit(EXIT_FAILURE);
                }
            }
            
            FD_SET(clientfd, &allFDs);

            if (clientfd > fdMax) {
                fdMax = clientfd;
            }
        }

        for (int i = 0; i < MAX_CLIENTS; i++) {            
            if (clients[i] != -1 && FD_ISSET(clients[i], &readFDs)) {
                int bytes;
                struct data d;
                CHECK_ERR_EQ(bytes = read(clients[i], &d, sizeof(d)), -1, "Reading data from the client");

                // Client disconnesso
                if (bytes == 0) {
                    close(clients[i]);
                    FD_CLR(clients[i], &allFDs);  
                    if (clients[i] == fdMax) {
                        do {
                            fdMax--;
                        } while (!FD_ISSET(fdMax, &allFDs));
                    }
                    clients[i] = -1;
                    continue;
                }

                // Segnale di terminazione
                if (d.n == -1) {
                    // Chiudo la connessione ai client ancora connessi
                    for (int i = 0; i < MAX_CLIENTS; i++) {
                        if (clients[i] != -1)
                            close(clients[i]);
                    }
                    // Chiude il server e termina il processo
                    close(serverfd);
                    cleanup();
                }
                
                // Comportamento di default
                printf("%d %12.2f %12.2f %20s\n", d.n, d.avg, d.std, d.filename);
            }
        }
    }
}