#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <arpa/inet.h>

#define FAIL -1

typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;

void signal_handler(int signum) {
    if (signum == SIGINT) {
        exit(0);
    } else if (signum == SIGCHLD) {
        int stat = 0;
        while (waitpid(-1, &stat, WNOHANG) > 0) {
        }
    }
}

void thongBao(struct in_addr addr, char *buffer) {
    int sfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    SOCKADDR_IN saddr;
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(6666);
    saddr.sin_addr.s_addr = inet_addr(inet_ntoa(addr));

    if (connect(sfd, (struct sockaddr *) &saddr, sizeof(saddr)) != 0) {
        close(sfd);
        return;
    }

    send(sfd, buffer, strlen(buffer), 0);
    close(sfd);
}

int main() {
    signal(SIGINT, signal_handler);
    signal(SIGCHLD, signal_handler);

    int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    int sockfd_2 = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    SOCKADDR_IN saddr, caddr, saddr_2, caddr_2;;
    int clen = sizeof(caddr);

    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(5555);
    saddr.sin_addr.s_addr = 0;

    saddr_2.sin_family = AF_INET;
    saddr_2.sin_addr.s_addr = 0;

    int error = bind(sockfd, (SOCKADDR *) &saddr, sizeof saddr);
    if (error == -1) {
        printf("Bind error\n");
        exit(1);
    }

    while (0 == 0) {
        char buffer[1024] = {0};
        int bytes = recvfrom(sockfd, buffer, sizeof(buffer), 0, (SOCKADDR *) &caddr, &clen);
        if (bytes > 0) {
            while (buffer[strlen(buffer) - 1] == '\n' || buffer[strlen(buffer) - 1] == '\r') {
                buffer[strlen(buffer) - 1] = 0;
            }

            if (strncmp(buffer, "GET ", 4) == 0) {
                char file1[200] = {0};
                int port = -1;
                sscanf(buffer, "GET %s %d %*s", file1, &port);
                int count = 0;
                for (int i = 0; i < strlen(buffer); ++i) {
                    if (buffer[i] == ' ')
                        count++;
                }
                if (port <= 0 || port > 65535 || count > 2) {
                    thongBao(caddr.sin_addr, "INVALID COMMENT\n");
                } else {
                    FILE *f = fopen(file1, "rb");
                    if (f != NULL) {
                        if (fork() == 0) {
                            close(sockfd);
                            saddr_2.sin_port = htons(port);

                            if (bind(sockfd_2, (SOCKADDR *) &saddr_2, sizeof saddr_2) == -1) {
                                close(sockfd_2);
                                printf("Bind error\n");
                                exit(1);
                            }

                            listen(sockfd_2, 1);

                            int cfd = accept(sockfd_2, (SOCKADDR *) &caddr_2, &clen);
                            if (cfd != FAIL) {
                                fseek(f, 0, SEEK_END);
                                int fsize = (int) ftell(f);
                                fseek(f, 0, SEEK_SET);
                                char *data = (char *) calloc(fsize, 1);
                                fread(data, 1, fsize, f);
                                fclose(f);
                                send(cfd, data, fsize, 0);
                                free(data);
                                data = NULL;
                                thongBao(caddr.sin_addr, "DONE\n");
                                close(cfd);
                                close(sockfd_2);
                                exit(0);
                            }
                        }
                    } else {
                        thongBao(caddr.sin_addr, "FILE NOT FOUND\n");
                    }
                }
            } else {
                thongBao(caddr.sin_addr, "INVALID COMMENT\n");
            }
        }
    }

}
