#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <getopt.h>
#include <stdint.h>


int k = -1;
int pnum = -1;
int mod = -1;  
long long result = 1; 
pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;  

void *factorial_part(void *arg) {
    int thread_id = *((int *)arg);
    int start = (k / pnum) * thread_id + 1;
    int end = (thread_id == pnum - 1) ? k : (k / pnum) * (thread_id + 1);

    long long local_result = 1;
    for (int i = start; i <= end; i++) {
        local_result = (local_result * i) % mod;
    }

    pthread_mutex_lock(&mut);
    result = (result * local_result) % mod;  
    pthread_mutex_unlock(&mut);

    return NULL;
}

int main(int argc, char *argv[]) {
    
        // Парсинг аргументов командной строки
    while (1) {
        static struct option options[] = {
            {"kol", required_argument, 0, 'k'},
            {"pnum", required_argument, 0, 'p'},
            {"mod", required_argument, 0, 'm'},
            {0, 0, 0, 0}
        };

        int option_index = 0;
        int c = getopt_long(argc, argv, "k:p:m:", options, &option_index);

        if (c == -1) break;

        switch (c) {
            case 'k': k = atoi(optarg); break;
            case 'p': pnum = atoi(optarg); break;
            case 'm': mod = atoi(optarg); break;
            case '?':
            printf("[!] Error. Incorrect argument.\n");
            return 1;
        }
    }

    if (k == -1 || mod == -1 || pnum == -1) {
        printf("Usage: %s -k <k_value> --pnum <pnum> --mod <mod>\n", argv[0]);
    return 1;
  }

    if (pnum > k) {
        printf("[!] Error: pnum cannot be greater than k.\n");
        return 1;
    }

    pthread_t threads[pnum];
    int thread_ids[pnum];

    for (int i = 0; i < pnum; i++) {
        thread_ids[i] = i;
        if (pthread_create(&threads[i], NULL, factorial_part, (void *)&thread_ids[i]) != 0) {
            perror("pthread_create");
            return 1;
        }
    }

    for (int i = 0; i < pnum; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            perror("pthread_join");
            return 1;
        }
    }

    printf("[+] Factorial: %lld\n", result);
    return 0;
}