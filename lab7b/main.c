#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <malloc.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
int client[100] = {0}, count = 0;

void *handle(void *arg) {
    int cfd = *(int *) arg;
    printf("Waiting for data...\n");
    while (0 == 0) {
        char *buffer = malloc(1024);
        int r = (int) recv(cfd, buffer, sizeof(buffer), 0);
        if (r > 0) {
            printf("Received: %s\n", buffer);
            for (int i = 0; i < count; ++i) {
                if (client[i] != cfd) {
                    send(client[i], buffer, sizeof(buffer), 0);
                }
            }
        } else {
            printf("A client has disconnected\n");
            return NULL;
        }
        free(buffer);
    }
}

int main() {
    SOCKADDR_IN saddr, caddr;
    int clen = sizeof(caddr);
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(5000);
    saddr.sin_addr.s_addr = 0;
    int sfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    bind(sfd, (SOCKADDR *) &saddr, sizeof(saddr));
    listen(sfd, 10);
    while (0 == 0) {
        int cfd = accept(sfd, (SOCKADDR *) &caddr, (unsigned int *) &clen);
        pthread_t thread;
        pthread_create(&thread, NULL, &handle, &cfd);
        client[count++] = cfd;
    }
}