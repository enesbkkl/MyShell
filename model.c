#include "model.h"
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#define errExit(msg) do { perror(msg); exit(EXIT_FAILURE); } while (0)

ShmBuf* buf_init() {
    int fd = shm_open(SHARED_FILE_PATH, O_CREAT | O_RDWR, 0600);
    if (fd == -1) {
        perror("shm_open");
        return NULL;
    }

    // Shared memory boyutunu ayarla
    if (ftruncate(fd, sizeof(ShmBuf)) == -1) {
        perror("ftruncate");
        close(fd);
        return NULL;
    }

    ShmBuf *shm = mmap(NULL, sizeof(ShmBuf), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (shm == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return NULL;
    }

    // Semaphore initialization (process-shared)
    if (sem_init(&shm->sem, 1, 1) == -1) {
        perror("sem_init");
        munmap(shm, sizeof(ShmBuf));
        close(fd);
        return NULL;
    }

    shm->fd = fd;
    shm->cnt = 0;
    return shm;
}

void buf_cleanup(ShmBuf *shm) {
    if (!shm) return;
    
    sem_destroy(&shm->sem);
    munmap(shm, sizeof(ShmBuf));
    close(shm->fd);
    // shm_unlink()'i MAIN PROCESS'te bir kez çağırın!
}

void execute_command(const char *command) {
    pid_t pid = fork();
    if (pid == 0) { // Child
        execlp("/bin/sh", "sh", "-c", command, NULL);
        errExit("execlp");
    } else if (pid > 0) {
        waitpid(pid, NULL, 0); // Zombie önleme
    }
}

void send_message(const char *msg) {
    ShmBuf *shm = buf_init();
    sem_wait(&shm->sem);
    strncpy(shm->msgbuf, msg, BUF_SIZE);
    shm->cnt = strlen(msg) + 1;
    sem_post(&shm->sem);
    buf_cleanup(shm);
}

char* read_messages() {
    ShmBuf *shm = buf_init();
    sem_wait(&shm->sem);
    char *msg = strdup(shm->msgbuf); // Derin kopya
    sem_post(&shm->sem);
    buf_cleanup(shm);
    return msg;
}