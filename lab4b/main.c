#include <stdio.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>

typedef struct sockaddr_in SOCKADDR_IN;

char *sendData(int sfd, char *buffer) {
    int tmp = 0;
    do {
        int byte = send(sfd, buffer + tmp, strlen(buffer) - tmp, 0);
        tmp += byte;
    } while (tmp < strlen(buffer));
}

char *getPWD(int sfd) {
    char command[20] = "pwd\n", *buffer;
    buffer = malloc(1024 * sizeof(char));
    memset(buffer, 0, 1024 * sizeof(char));
    sendData(sfd, command);
    int bytes = recv(sfd, buffer, 1024 * sizeof(char), 0);
    buffer[bytes - 1] = 0;
    return buffer;

}

int main() {
    int sfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    SOCKADDR_IN saddr;

    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(9999);
    saddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sfd, (struct sockaddr *) &saddr, sizeof(saddr)) != 0) {
        close(sfd);
        printf("Khong the ket noi may chu.\n");
        exit(-1);
    }

    char buffer[1024] = {0};
    recv(sfd, buffer, 1024, 0);

    while (1) {
        printf("#%s#", getPWD(sfd));
        fgets(buffer, 1024, stdin);
        sendData(sfd, buffer);
        recv(sfd, buffer, 1024, 0);
        printf("%s", buffer);
    }
    close(sfd);
}
