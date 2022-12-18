#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <pthread.h>
#include <unistd.h>
#include <malloc.h>

int step = 0, total = 0, n;

void *my_func(void *arg) {
    int i = *(int *) arg;
    printf("Thread %d: ", i + 1);
    for (int j = 1; j <= step; ++j) {
        total += j + i * step;
        printf("%d ", j + i * step);
    }
    if (i + 1 == n) {
        for (int j = 2; j < (i + 1) * step; ++j) {
            printf("%d ", j + i * step);
            total += j + i * step;
        }
    }
    printf("\n");
    return NULL;
}

int main() {
    int k;
    printf("Enter n: ");
    scanf("%d", &n);
    printf("Enter k: ");
    scanf("%d", &k);
    step = k / n;

    pthread_t thread[n];
    for (int i = 0; i < n; ++i) {
        int *arg = malloc(sizeof(int));
        *arg = i;
        pthread_create(&thread[i], NULL, my_func, arg);
    }
    for (int i = 0; i < n; ++i) {
        pthread_join(thread[i], NULL);
    }
    printf("Total: %d\n", total);
}