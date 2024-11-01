#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    pid_t pid = fork();

    if (pid > 0) {
        // Родительский процесс
        printf("Родительский процесс (PID: %d) создал дочерний процесс (PID: %d).\n", getpid(), pid);

        // Ждём завершения дочернего процесса с помощью wait()
        int status;
        wait(&status); // Ожидаем завершения дочернего процесса
        printf("Родительский процесс завершил ожидание дочернего процесса (PID: %d).\n", pid);
        
    } else if (pid == 0) {
        // Дочерний процесс
        printf("Дочерний процесс (PID: %d) завершает работу.\n", getpid());
        exit(0);  // Завершаем дочерний процесс
    } else {
        // Ошибка при создании процесса
        perror("Ошибка при вызове fork");
        return 1;
    }

    return 0;
}