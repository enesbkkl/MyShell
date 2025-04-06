#include "model.h"
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#define errExit(msg) do { perror(msg); exit(EXIT_FAILURE); } while (0)

pid_t main_pid;

ShmBuf* buf_init(void) {
    int fd = shm_open(SHARED_FILE_PATH, O_CREAT | O_RDWR, 0600);
    if (fd == -1) {
        perror("shm_open");
        return NULL;
    }

    if (ftruncate(fd, sizeof(ShmBuf) + BUF_SIZE) == -1) {
        perror("ftruncate");
        close(fd);
        return NULL;
    }

    ShmBuf *shm = mmap(NULL, sizeof(ShmBuf) + BUF_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (shm == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return NULL;
    }

    if (sem_init(&shm->sem, 1, 1) == -1) {
        perror("sem_init");
        munmap(shm, sizeof(ShmBuf) + BUF_SIZE);
        close(fd);
        return NULL;
    }

    shm->fd = fd;
    shm->cnt = 0;
    return shm;
}

void buf_cleanup(ShmBuf *shm) {
    if (!shm || shm->fd == -1) return;

    // Semaphore temizliği (doğrudan çağrı)
    sem_destroy(&shm->sem);  // NULL kontrolü KALDIRILDI
    
    // mmap temizliği
    munmap(shm, sizeof(ShmBuf) + BUF_SIZE);  // Boyut sabit

    // shm_unlink sadece ana process'te
    if (getpid() == main_pid) {
        close(shm->fd);
        shm_unlink(SHARED_FILE_PATH);
    }
}  

void execute_command(const char *command) {
    pid_t pid = fork();
    if (pid == 0) { // Child
        execlp("/bin/sh", "sh", "-c", command, NULL);
        perror("execvp failed"); // exec hata mesajı
        exit(EXIT_FAILURE);
    }
    else if (pid > 0) { // parent
        wait(NULL); // child'ı bekle
    }
    else {
        perror("fork failed");
    }
}

void send_message(const char *msg) {
    ShmBuf *shm = buf_init();
    if (!shm) return;
    sem_wait(&shm->sem);
    strncpy(shm->msgbuf, msg, BUF_SIZE);
    shm->cnt = strlen(msg) + 1;
    sem_post(&shm->sem);
    buf_cleanup(shm);
}

char* read_messages() {
    ShmBuf *shm = buf_init();
    if (!shm) return NULL;
    
    char *msg = NULL;
    if (sem_wait(&shm->sem) == 0) { // Semaphore başarılıysa
        msg = strdup(shm->msgbuf);
        sem_post(&shm->sem);
    }
    
    // cleanup YAPMA! (Sadece ana process temizlesin)
    return msg;
}