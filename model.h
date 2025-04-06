#ifndef MODEL_H
#define MODEL_H

#include <semaphore.h>
#include <sys/mman.h>

#define BUF_SIZE 4096
#define SHARED_FILE_PATH "mymsgbuf"

extern pid_t main_pid;

typedef struct {
    sem_t sem;
    size_t cnt;
    int fd;
    char msgbuf[BUF_SIZE];
} ShmBuf;

// Shared Memory İşlemleri
ShmBuf* buf_init();
void buf_cleanup(ShmBuf *shm);

// Shell Komut Yürütme
void execute_command(const char *command);

// Mesajlaşma
void send_message(const char *msg);
char* read_messages();

#endif