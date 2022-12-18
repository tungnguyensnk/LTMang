#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
typedef struct account {
    char addr[100];
    struct account *next;
} account;
account *list = NULL;

account *searchAndAdd(char addr[]) {
    if (list == NULL) {
        list = malloc(sizeof(account));
        sprintf(list->addr, "%s", addr);
        list->next = NULL;
    } else {
        account *tmp = list;
        int isHas = 0;
        while (tmp != NULL) {
            if (strcmp(tmp->addr, addr) == 0)
                isHas = 1;
            tmp = tmp->next;
        }

        if (!isHas) {
            account *new_acc = malloc(sizeof(account));
            sprintf(new_acc->addr, "%s", addr);
            new_acc->next = NULL;
            list->next = new_acc;
        }
    }
    return list;
}

int main() {
    int s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    SOCKADDR_IN saddr, caddr;
    int clen = sizeof(caddr);
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(5000);
    saddr.sin_addr.s_addr = 0;
    bind(s, (SOCKADDR *) &saddr, sizeof(saddr));
    while (1) {
        char buffer[1024] = {0};
        int r = recvfrom(s, buffer, sizeof(buffer), 0, (SOCKADDR *) &caddr, &clen);
        if (r > 0 && strcmp(buffer, "X") != 0) {
            if (strncasecmp(buffer, "REG ", 4) == 0) {
                list = searchAndAdd(inet_ntoa(caddr.sin_addr));
                continue;
            }

            if (strncasecmp(buffer, "CHAT ", 4) == 0) {
                account *tmp = list;
                while (tmp != NULL) {
                    caddr.sin_family = AF_INET;
                    caddr.sin_port = htons(7000);
                    sendto(s, buffer, strlen(buffer), 0, (SOCKADDR *) &caddr, clen);
                    tmp = tmp->next;
                }
                continue;
            }
            printf("%s\n", buffer);
        } else
            break;
    }
    close(s);
}