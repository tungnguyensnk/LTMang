#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    system("ifconfig > ifconfig.txt");

    FILE *taptin = NULL;
    taptin = fopen("ifconfig.txt", "r");
    if (taptin != NULL) {
        while (!feof(taptin)) {
            char *sub, *tmp, line[500] = "";
            fgets(line, sizeof(line), taptin);
            if ((sub = strstr(line, " flags")) != NULL) {
                sub[0] = '\0';
                printf("%s", line);
                fgets(line, sizeof(line), taptin);
                if ((sub = strstr(line, "inet ")) != NULL) {
                    if ((tmp = strstr(sub, " netmask")) != NULL) {
                        tmp[0] = '\0';
                    }
                    printf(" %s", sub + 5);
                }
                fgets(line, sizeof(line), taptin);
                if ((sub = strstr(line, "inet6 ")) != NULL) {
                    if ((tmp = strstr(sub, " prefixlen")) != NULL) {
                        tmp[0] = '\0';
                    }
                    printf(" %s\n", sub + 6);
                }
            }
        }
    }
    return 0;
}
