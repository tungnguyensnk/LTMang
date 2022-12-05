//http server file
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <malloc.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>

#define INVALID_SOCKET (-1)
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;

void Append(char **psrc, const char *dest) {
    char *src = *psrc;
    int oldLen = src == NULL ? 0 : strlen(src);
    *psrc = (char *) realloc(*psrc, oldLen + strlen(dest) + 1);
    src = *psrc;
    memset(src + oldLen, 0, strlen(dest) + 1);
    sprintf(src + oldLen, "%s", dest);
}

int Compare(const struct dirent **A, const struct dirent **B) {
    if ((*A)->d_type == (*B)->d_type) {
        return 0;
    } else if ((*A)->d_type == DT_DIR) {
        return -1;
    } else
        return 1;
}

void *ClientThread(void *arg) {
    int cfd = *((int *) arg);
    free(arg);
    arg = NULL;
    char *html = NULL;
    char buffer[1024] = {0};
    recv(cfd, buffer, sizeof(buffer), 0);
    char METHOD[8] = {0};
    char PATH[1024] = {0};
    sscanf(buffer, "%s%s", METHOD, PATH);
    while (strstr(PATH, "%20") != NULL) {
        char *tmp = strstr(PATH, "%20");
        tmp[0] = ' ';
        strcpy(tmp + 1, tmp + 3);
    }
    char *ok = "HTTP/1.1 200 OK\r\nContent-Length: %dContent-Type: %s\r\n\r\n";
    char type[25] = {0};
    struct dirent **output = NULL;
    char *rootPath = (char *) calloc(1024, 1);
    strcpy(rootPath, "/mnt/c");
    Append(&rootPath, PATH);
    DIR *directory = opendir(rootPath);
    if (directory == NULL) {
        if (strstr(PATH, "favicon.ico") != NULL) {
            char *notFound = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
            send(cfd, notFound, strlen(notFound), 0);
            close(cfd);
            return NULL;
        }
        if (strstr(PATH, ".jpg") != NULL)
            sprintf(type, "image/jpeg");
        else if (strstr(PATH, ".png") != NULL)
            sprintf(type, "image/png");
        else if (strstr(PATH, ".gif") != NULL)
            sprintf(type, "image/gif");
        else if (strstr(PATH, ".html") != NULL)
            sprintf(type, "text/html");
        else if (strstr(PATH, ".css") != NULL)
            sprintf(type, "text/css");
        else if (strstr(PATH, ".js") != NULL)
            sprintf(type, "application/javascript");
        else if (strstr(PATH, ".mp4") != NULL)
            sprintf(type, "video/mp4");
        else if (strstr(PATH, ".mp3") != NULL)
            sprintf(type, "audio/mpeg");
        else
            sprintf(type, "application/octet-stream");
        char command[1024] = {0};
        sprintf(command, "cp \"%s\" tmp", rootPath);
        system(command);
        FILE *f = fopen("tmp", "rb");
        fseek(f, 0, SEEK_END);
        int size = ftell(f);
        char *data = (char *) calloc(size, 1);
        fseek(f, 0, SEEK_SET);
        fread(data, 1, size, f);
        fclose(f);
        f = NULL;
        char header[1024] = {0};
        sprintf(header, ok, size, type);
        send(cfd, header, strlen(header), 0);
        send(cfd, data, size, 0);
    } else {
        sprintf(type, "text/html");
        Append(&html, "<html>");
        int n = scandir(rootPath, &output, NULL, Compare);
        if (n > 0) {
            for (int i = 0; i < n; i++) {
                if (output[i]->d_type == DT_REG) {
                    char tmp[2048] = {0};
                    if (PATH[strlen(PATH) - 1] == '/')
                        sprintf(tmp, "<a href=\"%s%s\"><i>%s</i></a><br>", PATH, output[i]->d_name, output[i]->d_name);
                    else
                        sprintf(tmp, "<a href=\"%s/%s\"><i>%s</i></a><br>", PATH, output[i]->d_name, output[i]->d_name);
                    Append(&html, tmp);
                }
                if (output[i]->d_type == DT_DIR) {
                    char tmp[2048] = {0};
                    if (PATH[strlen(PATH) - 1] == '/')
                        sprintf(tmp, "<a href=\"%s%s\"><b>%s</b></a><br>", PATH, output[i]->d_name, output[i]->d_name);
                    else
                        sprintf(tmp, "<a href=\"%s/%s\"><b>%s</b></a><br>", PATH, output[i]->d_name, output[i]->d_name);
                    Append(&html, tmp);
                }
            }
        } else {
            Append(&html, "<b>EMPTY FOLDER</b>");
        }
        Append(&html, "</html>");
        char header[1024] = {0};
        sprintf(header, ok, strlen(html), type);
        send(cfd, header, strlen(header), 0);
        send(cfd, html, strlen(html), 0);
    }


    close(cfd);

    free(html);
    html = NULL;
    free(rootPath);
    rootPath = NULL;

    return NULL;
}

int main() {
    int sfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    SOCKADDR_IN saddr, caddr;
    int clen = sizeof(caddr);
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(8888);
    saddr.sin_addr.s_addr = 0;
    if (bind(sfd, (SOCKADDR *) &saddr, sizeof(saddr)) == 0) {
        listen(sfd, 10);
        while (0 == 0) {
            int tmp = accept(sfd, (SOCKADDR *) &caddr, &clen);
            if (tmp != INVALID_SOCKET) {
                pthread_t tid = 0;
                int *arg = (int *) calloc(1, sizeof(int));
                *arg = tmp;
                pthread_create(&tid, NULL, ClientThread, (void *) arg);
            }
        }
    } else
        printf("PORT not available\n");
}