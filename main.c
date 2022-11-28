#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>

typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
int main()
{
    int s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    SOCKADDR_IN saddr, ackaddr;
    ackaddr.sin_family = AF_INET;
    ackaddr.sin_port = htons(6000);
    ackaddr.sin_addr.s_addr = 0;
    bind(s, (SOCKADDR*)&ackaddr, sizeof(ackaddr));

    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(5000);
    saddr.sin_addr.s_addr = inet_addr("172.30.236.220");
    while (1)
    {
        char buffer[1024] = { 0 };
        fgets(buffer, sizeof(buffer), stdin);
        int sent = sendto(s, buffer, strlen(buffer), 0, (SOCKADDR*)&saddr, sizeof(saddr));
        char ack[1024] = { 0 };
        recvfrom(s, ack, sizeof(ack), 0, NULL, NULL);
        printf("ACK: %s\n", ack);
    }
    close(s);
}