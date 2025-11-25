#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <string.h>
#include <stdio.h>

typedef struct {
    char filename[256];
    char result[64];
} shared_data_t;

int main() {
    char shm_name[64], sem_name[64];
    pid_t pid = getpid();

    sprintf(shm_name, "/sum_shm_%d", pid);
    sprintf(sem_name, "/sum_sem_%d", pid);

    sem_t *sem = sem_open(sem_name, O_CREAT | O_EXCL, 0644, 0);
    if (sem == SEM_FAILED) exit(1);

    int shm_fd = shm_open(shm_name, O_CREAT | O_RDWR, 0644);
    if (shm_fd == -1) exit(1);

    ftruncate(shm_fd, sizeof(shared_data_t));

    shared_data_t *data = mmap(NULL, sizeof(shared_data_t),
        PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (data == MAP_FAILED) exit(1);

    write(1, "введите имя файла: ", strlen("введите имя файла: "));
    ssize_t n = read(0, data->filename, 255);
    if (n <= 0) exit(1);

    if (data->filename[n - 1] == '\n')
        data->filename[n - 1] = 0;
    else
        data->filename[n] = 0;

    pid_t c = fork();
    if (c == 0) {
        execl("./client", "client", shm_name, sem_name, NULL);
        _exit(1);
    }

    sem_wait(sem);

    write(1, "сумма чисел: ", strlen("сумма чисел: "));
    write(1, data->result, strlen(data->result));
    write(1, "\n", 1);

    wait(NULL);

    munmap(data, sizeof(shared_data_t));
    shm_unlink(shm_name);
    sem_close(sem);
    sem_unlink(sem_name);
    return 0;
}
