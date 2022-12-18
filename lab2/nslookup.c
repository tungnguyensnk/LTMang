#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define INVALID_SOCKET -1

int main(int argc, char **argv) {
    //char* domain = argv[1];
    char *domain = "vnexpress.net";
    struct hostent *phost = gethostbyname(domain);
    if (phost != NULL) {
        struct hostent host = *phost;
        if (host.h_addrtype == AF_INET) {
            int count = 0;
            struct in_addr addr;
            while (host.h_addr_list[count] != NULL) {
                char *ipBytes = host.h_addr_list[count];
                memcpy(&addr, ipBytes, sizeof(addr));
                char *saddr = inet_ntoa(addr);
                struct in_addr addr1;
                inet_aton(saddr, &addr1);
                printf("%s\n", saddr);
                count += 1;
            }
            if (count > 0) {
                int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
                if (fd != INVALID_SOCKET) {
                    struct in_addr addr;

                    struct sockaddr_in addrin;
                    addrin.sin_family = AF_INET;
                    addrin.sin_port = htons(80);
                    addrin.sin_addr = addr;
                    struct sockaddr *saddr = (struct sockaddr *) &addrin;
                    int error = connect(fd, saddr, sizeof(addrin));
                    if (error == 0) {
                        char buffer[1024] = {0};
                        char *hello = "GET / HTTP/1.1\r\nHost: vnexpress.net\r\n\r\n";
                        int sent = send(fd, hello, strlen(hello), 0);
                        int received = recv(fd, buffer, sizeof(buffer) - 1, 0);
                        printf("%d\n%s\n", received, buffer);
                        sent = send(fd, hello, strlen(hello), 0);
                        received = recv(fd, buffer, sizeof(buffer) - 1, 0);
                        printf("%d\n%s\n", received, buffer);
                    }
                }
            }
        }
    } else
        printf("Failed to lookup\n");
}
