#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

int main() {
    system("ls -al > out.txt");

    FILE *taptin = NULL;
    taptin = fopen("out.txt", "r");
    if (taptin != NULL) {
        char line[500] = "", name[500] = "", size[20] = "";
        fgets(line, sizeof(line), taptin);
        while (!feof(taptin)) {
            fgets(line, sizeof(line), taptin);
            if (line[strlen(line) - 1] != '\0')
                line[strlen(line) - 1] = '\0';
            sscanf(line, "%*s%*s%*s%*s%s%*s%*s%s", size, name);
            char *sub = strstr(line, name);
            DIR *directory = opendir(sub + 6);
            if (directory == NULL) {
                printf("<%s><%s>\n", sub + 6, size);
            }
        }
    }
    return 0;
}
