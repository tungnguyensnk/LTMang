#include <stdio.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

#define FAIL -1
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
pthread_mutex_t *mutex = NULL;

void *handle(void *arg) {
    int cfd = *(int *) arg;
    char *welcome = "Welcome to TCP server\nType a command\n";
    send(cfd, welcome, strlen(welcome), 0);

    while (1) {
        char buffer[1024] = {0};
        recv(cfd, buffer, sizeof buffer, 0);
        if (buffer[strlen(buffer) - 1] == '\n')
            buffer[strlen(buffer) - 1] = 0;
        sprintf(buffer + strlen(buffer), " > out.txt");
        pthread_mutex_lock(mutex);
        system(buffer);

        FILE *taptin = NULL;
        taptin = fopen("out.txt", "r");
        if (taptin != NULL) {
            char line[500] = "";
            fgets(line, sizeof(line), taptin);
            while (!feof(taptin)) {
                fgets(line, sizeof(line), taptin);
                send(cfd, line, strlen(line), 0);
            }
        }
        fclose(taptin);
        pthread_mutex_unlock(mutex);
        if (strncmp(buffer, "exit", 4) == 0)
            break;
    }
}

int main() {
    mutex = (pthread_mutex_t*)calloc(1, sizeof(pthread_mutex_t));
    pthread_mutex_init(mutex, NULL);
    int sfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    SOCKADDR_IN saddr, caddr;
    int clen = sizeof(caddr);

    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(9999);
    saddr.sin_addr.s_addr = 0;

    int error = bind(sfd, (SOCKADDR *) &saddr, sizeof saddr);
    if (error == -1)
        abort();

    listen(sfd, 10);
    while (0 == 0) {
        int cfd = accept(sfd, (SOCKADDR *) &caddr, (unsigned int *) &clen);
        if (cfd != FAIL) {
            pthread_t thread;
            pthread_create(&thread, NULL, &handle, &cfd);
        }
    }
}
