#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <arpa/inet.h>

#define FAIL (-1)
#define MAX_CLIENT 32

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

int sendData(int sfd, char *buffer) {
    int count = 0, bytes;
    do {
        bytes = (int) send(sfd, buffer + count, strlen(buffer) - count, 0);
        count += bytes;
    } while (bytes >= 0 && count < strlen(buffer));
    return count;
}

int sendNoti(char *address, char *buffer) {
    int sfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    SOCKADDR_IN saddr;
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(6666);
    saddr.sin_addr.s_addr = inet_addr(address);

    if (connect(sfd, (struct sockaddr *) &saddr, sizeof(saddr)) != 0) {
        close(sfd);
        printf("Connect Fail\n");
        exit(1);
    }

    sendData(sfd, buffer);
    close(sfd);
}

int main() {
    signal(SIGINT, signal_handler);
    signal(SIGCHLD, signal_handler);

    int udp_sfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    SOCKADDR_IN saddr, caddr;
    int clen = sizeof(caddr);

    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(5555);
    saddr.sin_addr.s_addr = INADDR_ANY;

    int error = bind(udp_sfd, (SOCKADDR *) &saddr, sizeof saddr);
    if (error == -1) {
        perror("error");
        exit(1);
    }

    while (0 == 0) {
        char buffer[1024] = {0};
        int bytes = recvfrom(udp_sfd, buffer, sizeof(buffer), 0, (SOCKADDR *) &caddr, (unsigned int *) &clen);
        if (bytes > 0) {
            while (buffer[strlen(buffer) - 1] == '\n' || buffer[strlen(buffer) - 1] == '\r') {
                buffer[strlen(buffer) - 1] = 0;
            }

            if (strcmp(buffer, "X") == 0)
                continue;
            if (strncmp(buffer, "GET ", 4) == 0) {
                char filename[200] = {0};
                int port = 0;
                sscanf(buffer, "GET%s%d %*s", filename, &port);
                char temp[1024] = {0};
                sprintf(temp, "GET %s %d", filename, port);
                if (port <= 0 || port > 65535 || strcmp(buffer, temp) != 0) {
                    char *result = "INVALID COMMENT\n";
                    sendNoti(inet_ntoa(caddr.sin_addr), result);
                }

                FILE *f = fopen(filename, "r");
                if (f != NULL) {
                    if (fork() == 0) {
                        close(udp_sfd);

                        int sendfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

                        SOCKADDR_IN saddr_send, caddr_recv;

                        saddr_send.sin_family = AF_INET;
                        saddr_send.sin_port = htons(port);
                        saddr_send.sin_addr.s_addr = INADDR_ANY;

                        error = bind(sendfd, (SOCKADDR *) &saddr_send, sizeof saddr_send);
                        if (error == -1) {
                            close(sendfd);
                            perror("error");
                            exit(1);
                        }

                        listen(sendfd, 1);

                        int cfd = accept(sendfd, (SOCKADDR *) &caddr_recv, (unsigned int *) &clen);
                        if (cfd != FAIL) {
                            fseek(f, 0, SEEK_END);
                            int fsize = (int) ftell(f);
                            fseek(f, 0, SEEK_SET);
                            char *data = (char *) calloc(fsize, 1);
                            fread(data, 1, fsize, f);
                            fclose(f);
                            sendData(cfd, data);
                            free(data);
                            data = NULL;
                            char *result = "DONE\n";
                            sendNoti(inet_ntoa(caddr.sin_addr), result);
                            close(cfd);
                            close(sendfd);
                            exit(0);
                        }
                    }
                } else {
                    char *result = "Permission Denied or File Not Found\n";
                    sendNoti(inet_ntoa(caddr.sin_addr), result);
                }
            } else {
                char *result = "INVALID COMMENT\n";
                sendNoti(inet_ntoa(caddr.sin_addr), result);
            }
        }
    }

}
