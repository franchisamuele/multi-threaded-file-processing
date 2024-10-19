#include "util.h"
#include "unboundedqueue.h"
#include "functions.h"

void* worker(void*);
int socketCreateAndConnect();
struct data processFile(char*);
void recursiveDir(char*, Queue_t*);

/* Args:
 * 1) dirname = directory di partenza da analizzare
 * 2) W = numero di worker che processano i file
*/
int main(int argc, char const *argv[]) {
    if (argc < 3) {
        printf("Non hai passato abbastanza parametri\n");
        exit(EXIT_FAILURE);
    }

    char *dirname = (char*)argv[1];
    const int W = atoi(argv[2]);

    Queue_t *q = initQueue();
    if (q == NULL) {
        perror("Creating q");
        exit(EXIT_FAILURE);
	}

    pthread_t tid[W];
    for (int i = 0; i < W; i++) {
        pthread_create(&tid[i], NULL, worker, (void*)q);
    }

    recursiveDir(dirname, q);
    CHECK_ERR_EQ(push(q, NULL), -1, "push");

    for (int i = 0; i < W; i++) {
        pthread_join(tid[i], NULL);
    }

    deleteQueue(q);

    // Comunica al collector di terminare
    struct data d = { 
        .n = -1
    };
    int serverfd = socketCreateAndConnect();
    CHECK_ERR_NEQ(write(serverfd, &d, sizeof(d)), sizeof(d), "Sending the terminator to the collector");
    close(serverfd);

    return 0;
}

void *worker(void *args) {
    Queue_t *q = (Queue_t*)args;

    int serverfd = socketCreateAndConnect();

    for (;;) {
        char *filename = pop(q);
        // Fine dati da processare
        if (filename == NULL) {
            CHECK_ERR_EQ(push(q, NULL), -1, "push");
            break;
        }

        // Ottiene i dati dal file
        struct data d = processFile(filename);
        free(filename);
        
        // Manda i dati al server (collector)
        CHECK_ERR_NEQ(write(serverfd, &d, sizeof(d)), sizeof(d), "Sending data to the collector");
    }

    close(serverfd);
    return NULL;
}

int socketCreateAndConnect() {
    int serverfd;
    // Crea il socket
    CHECK_ERR_EQ(serverfd = socket(AF_UNIX, SOCK_STREAM, 0), -1, "Creating the socket");

    struct sockaddr_un serverAddr;
    serverAddr.sun_family = AF_UNIX;
    strcpy(serverAddr.sun_path, "/tmp/635112socket");

    int conn = 0;
    while ((conn = connect(serverfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr))) == -1
            && errno == ENOENT) {
        // Tentativo di connessione
        sleep(1);
    }
    if (conn == -1) {
        perror("Connecting to the server");
        exit(EXIT_FAILURE);
    }

    return serverfd;
}

struct data processFile(char *filename) {
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        perror("Errore apertura file");
        exit(EXIT_FAILURE);
    }

    float data[500];

    int n = 0;
    float avg = 0;
    float std = 0;

    float numero;
    while (fscanf(fp, "%f", &numero) != EOF) {
        data[n++] = numero;
        avg += numero;
    }

    avg /= n;

    for (int i = 0; i < n; ++i) {
        std += pow((data[i] - avg), 2);
    }

    std = sqrt(std / n);

    struct data res = { 0 };
    res.n = n;
    res.avg = avg;
    res.std = std;
    strcpy(res.filename, filename);

    fclose(fp);

    return res;
}

void recursiveDir(char *dirname, Queue_t *q) {
    DIR *dir = opendir(dirname);
    if (dir == NULL) {
        perror("opendir");
        exit(EXIT_FAILURE);
    }

    struct dirent *file;
    while ((file = readdir(dir)) != NULL) {
        if (strcmp(file->d_name, ".") == 0 || strcmp(file->d_name, "..") == 0) {
            continue;
        }

        char *path = malloc(MAX_FILENAME_LENGTH * sizeof(char));
        strncpy(path, dirname, MAX_FILENAME_LENGTH);
        strcat(path, "/");
        strcat(path, file->d_name);

        struct stat st;
        CHECK_ERR_EQ(stat(path, &st), -1, "stat");

        if (S_ISDIR(st.st_mode)) {
            recursiveDir(path, q); // Ricorsivamente esplora le sottodirectory
            free(path);
        } else {
            // Controllo se il file ha estensione ".dat"
            int path_len = strlen(path);
            if (path_len >= 4 && strncmp(path + path_len - 4, ".dat", 4) == 0) {
                // Manda il lavoro ai worker
                CHECK_ERR_EQ(push(q, path), -1, "push");
            } else {
                free(path);
            }
        }
    }

    closedir(dir);
}