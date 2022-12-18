#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

char *html = NULL;

void addstr(char **psrc, const char *dest) {
    int oldLen = *psrc == NULL ? 0 : (int) strlen(*psrc);
    *psrc = (char *) realloc(*psrc, oldLen + strlen(dest) + 1);
    char *src = *psrc;
    memset(src + oldLen, 0, strlen(dest) + 1);
    sprintf(src + strlen(src), "%s", dest);

}

void printTab(int tab) {
    for (int i = 0; i < tab; ++i)
        addstr(&html, "&emsp;");
}

int compare(const struct dirent **e1, const struct dirent **e2) {
    const unsigned char a = (*e1)->d_type;
    const unsigned char b = (*e2)->d_type;
    if (a == DT_DIR && b == DT_REG)
        return -1;
    else if (a == DT_REG && b == DT_DIR)
        return 1;
    else
        return strcmp((*e1)->d_name, (*e2)->d_name);
}

void scanfdir(char path[], int tab) {
    struct dirent **entries = NULL;
    int n = scandir(path, &entries, NULL, compare);

    if (entries != NULL) {
        for (int i = 0; i < n; ++i) {
            char tmp[2048] = "";
            printTab(tab);
            if (entries[i]->d_type == DT_REG) {
                sprintf(tmp, "<a href = \"%s%s\"><i>%s</i></a><br/>\n", path, entries[i]->d_name, entries[i]->d_name);
                addstr(&html, tmp);
            } else {
                addstr(&html, "<br/>");
                sprintf(tmp, "<a href = \"%s%s\"><b>%s</b></a><br/>\n", path, entries[i]->d_name, entries[i]->d_name);
                addstr(&html, tmp);
                if (strcmp(entries[i]->d_name, ".") != 0 && strcmp(entries[i]->d_name, "..") != 0) {
                    char newDir[1024] = "";
                    sprintf(newDir, "%s/%s", path, entries[i]->d_name);
                    scanfdir(newDir, tab++);
                }
                addstr(&html, "<br/>");
            }

            free(entries[i]);
            entries[i] = NULL;
        }
        free(entries);
        entries = NULL;
    } else
        addstr(&html, "<b>Khong co thu muc.</b><br/>");
}

int main() {
    addstr(&html, "<html>\n");
    scanfdir("..", 0);
    addstr(&html, "</html>");
    FILE *fp = fopen("scandir.html", "w");
    fprintf(fp, "%s", html);
    fclose(fp);
    return 0;
}
