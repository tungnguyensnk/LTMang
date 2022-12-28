#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>

#define INVALID_SOCKET -1
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;

void signal_handler(int signum) {
    if (signum == SIGCHLD) {
        int stat = 0;
        while (waitpid(-1, &stat, WNOHANG) > 0) {

        }
    }
}

int checkIpExist(char *ip) {
    FILE *fp = fopen("client.txt", "r");
    char buf[1024] = {0};
    if (fp == NULL)
        return 0;
    while (fgets(buf, sizeof(buf), fp) != NULL) {
        if (strstr(buf, ip) != NULL) {
            fclose(fp);
            return 1;
        }
    }
    fclose(fp);
    return 0;
}

int main() {
    signal(SIGCHLD, signal_handler);
    SOCKADDR_IN saddr1, saddr2, caddr;
    int clen = sizeof(caddr);
    saddr1.sin_family = AF_INET;
    saddr1.sin_port = htons(5000);
    saddr1.sin_addr.s_addr = 0;
    int tcp_sfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    bind(tcp_sfd, (SOCKADDR *) &saddr1, sizeof(saddr1));
    listen(tcp_sfd, 10);

    int udp_sfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    saddr2.sin_family = AF_INET;
    saddr2.sin_port = htons(4000);
    saddr2.sin_addr.s_addr = INADDR_ANY;
    bind(udp_sfd, (SOCKADDR *) &saddr2, sizeof(saddr2));
    if (fork() == 0) {
        close(tcp_sfd);
        while (0 == 0) {
            char buffer[1024] = {0};
            int r = recvfrom(udp_sfd, buffer, sizeof(buffer), 0, (SOCKADDR *) &caddr, (unsigned int *) &clen);
            if (r > 0) {
                while (buffer[strlen(buffer) - 1] == '\n' || buffer[strlen(buffer) - 1] == '\r') {
                    buffer[strlen(buffer) - 1] = 0;
                }

                if (strcmp(buffer, "X") != 0) {
                    if (checkIpExist(strdup(inet_ntoa(caddr.sin_addr))) == 0) {
                        FILE *fp = fopen("client.txt", "a");
                        fprintf(fp, "%s %s\n", inet_ntoa(caddr.sin_addr), buffer);
                        fclose(fp);
                    }

                    sprintf(buffer, "%s\n", inet_ntoa(saddr2.sin_addr));
                    caddr.sin_port = htons(7000);
                    sendto(udp_sfd, buffer, strlen(buffer), 0, (SOCKADDR *) &caddr, clen);
                }
            } else
                printf("Failed to recvfrom\n");
        }
    } else {
        if (fork() == 0) {
            close(udp_sfd);
            while (0 == 0) {
                int cfd = accept(tcp_sfd, (SOCKADDR *) &caddr, (unsigned int *) &clen);
                if (cfd != INVALID_SOCKET) {
                    FILE *f = fopen("client.txt", "rb");
                    if (f != NULL) {
                        fseek(f, 0, SEEK_END);
                        int fsize = ftell(f);
                        fseek(f, 0, SEEK_SET);
                        char *data = (char *) calloc(fsize, 1);
                        fread(data, 1, fsize, f);
                        fclose(f);
                        send(cfd, data, strlen(data), 0);
                        free(data);
                        data = NULL;
                    } else {
                        char *result = "Permission Denied\n";
                        send(cfd, result, strlen(result), 0);
                    }
                    shutdown(cfd, SHUT_RDWR);
                    close(cfd);
                }
            }
        } else {
            while (0 == 0) {
                sleep(1000);
            }
        }
    }

}