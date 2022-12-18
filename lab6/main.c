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
#include <signal.h>
#include <sys/wait.h>

#define MAX_CONN_NUM 1024
#define INVALID_SOCKET -1
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;

int g_cfd[MAX_CONN_NUM];
int g_count = 0;

void signal_handler(int signum) {
    printf("Signal: %d\n", signum);
    if (signum == SIGINT) {
        exit(0);
    } else if (signum == SIGCHLD) {
        int stat = 0;
        while (waitpid(-1, &stat, WNOHANG) > 0) {

        }
    } else if (signum == SIGUSR1) {
        for (int i = 0; i < g_count; i++) {
            FILE *f = fopen("data.tmp", "rt");
            char buffer[1024] = {0};
            fgets(buffer, sizeof(buffer), f);
            fclose(f);
            send(g_cfd[i], buffer, strlen(buffer), 0);
        }
    }
}

int main() {
    int parent_id = getpid();
    signal(SIGINT, signal_handler);
    signal(SIGCHLD, signal_handler);
    signal(SIGUSR1, signal_handler);
    SOCKADDR_IN saddr, caddr;
    int clen = sizeof(caddr);
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(5000);
    saddr.sin_addr.s_addr = 0;
    int sfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    bind(sfd, (SOCKADDR *) &saddr, sizeof(saddr));
    listen(sfd, 10);
    while (0 == 0) {
        int cfd = accept(sfd, (SOCKADDR *) &caddr, &clen);
        if (fork() == 0) {
            close(sfd);
            while (0 == 0) {
                printf("Waiting for data...\n");
                char buffer[1024] = {0};
                int r = recv(cfd, buffer, sizeof(buffer), 0);
                if (r > 0) {
                    printf("Received: %s\n", buffer);
                    FILE *f = fopen("data.tmp", "wt");
                    fprintf(f, "%s", buffer);
                    fclose(f);
                    kill(parent_id, SIGUSR1);
                } else {
                    printf("A client has disconnected\n");
                    exit(0);
                }
            }
        } else {
            printf("Client Count: %d\n", g_count + 1);
            printf("CFD: %d\n", cfd);
            g_cfd[g_count++] = cfd;
        }
    }
    return 0;
}