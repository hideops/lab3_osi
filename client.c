#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <string.h>
#include <stdio.h>

typedef struct {
    char filename[256];
    char result[64];
} shared_data_t;

int main(int argc, char *argv[]) {
    sem_t *sem = sem_open(argv[2], 0);
    if (sem == SEM_FAILED) exit(1);

    int shm_fd = shm_open(argv[1], O_RDWR, 0644);
    if (shm_fd == -1) exit(1);

    shared_data_t *data = mmap(NULL, sizeof(shared_data_t),
        PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (data == MAP_FAILED) exit(1);

    int fd = open(data->filename, O_RDONLY);
    if (fd == -1) exit(1);

    char buf[256];
    ssize_t r;
    float sum = 0.0f;

    while ((r = read(fd, buf, sizeof(buf) - 1)) > 0) {
        buf[r] = 0;
        char *p = buf;
        while (*p) {
            char *e;
            float v = strtof(p, &e);
            if (p != e) sum += v;
            p = e;
            while (*p == ' ' || *p == '\n' || *p == '\t') p++;
        }
    }

    close(fd);

    snprintf(data->result, sizeof(data->result), "%.2f", sum);
    sem_post(sem);

    munmap(data, sizeof(shared_data_t));
    close(shm_fd);
    sem_close(sem);
    return 0;
}
