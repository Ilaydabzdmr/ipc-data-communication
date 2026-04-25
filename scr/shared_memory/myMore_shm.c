#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <semaphore.h>
#include <signal.h>

#define SHM_NAME "/ceng302_shm"
#define SEM_MUTEX "/ceng302_mutex"
#define SEM_EMPTY "/ceng302_empty"
#define SEM_FULL "/ceng302_full"
#define MAX_LOG_LENGTH 512

typedef struct {
    char message[MAX_LOG_LENGTH];
} shared_memory_t;

shared_memory_t *shm_ptr;
sem_t *sem_mutex, *sem_empty, *sem_full;
int shm_fd;

// Graceful Shutdown - CTRL+C Sinyal Yakalayici
void handle_sigint(int sig) {
    printf("\n[SIGINT Algilandi] myMore_shm kaynaklari temizliyor...\n");
    munmap(shm_ptr, sizeof(shared_memory_t));
    shm_unlink(SHM_NAME); // Zombie memory onleme
    
    sem_close(sem_mutex); sem_close(sem_empty); sem_close(sem_full);
    sem_unlink(SEM_MUTEX); sem_unlink(SEM_EMPTY); sem_unlink(SEM_FULL);
    exit(0);
}

int main() {
    signal(SIGINT, handle_sigint);

    // Shared Memory'i ac
    shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("SHM acilamadi, once ureticiyi baslatin");
        exit(1);
    }
    shm_ptr = mmap(0, sizeof(shared_memory_t), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

    // Semoforlari ac
    sem_mutex = sem_open(SEM_MUTEX, 0);
    sem_empty = sem_open(SEM_EMPTY, 0);
    sem_full = sem_open(SEM_FULL, 0);

    int line_count = 0;
    char buffer[MAX_LOG_LENGTH];

    while (1) {
        sem_wait(sem_full);  // Okunacak veri gelmesini bekle
        sem_wait(sem_mutex); // Kritik alana gir
        
        strcpy(buffer, shm_ptr->message); // Veriyi kopyala
        
        sem_post(sem_mutex); // Kritik alandan cik
        sem_post(sem_empty); // Ureticiye yeni veri yazabilecegini bildir

        if (strcmp(buffer, "EOF_MARKER\n") == 0) {
            break;
        }

        // Filtreleme
        if (strstr(buffer, "CRITICAL") != NULL || strstr(buffer, "ERROR") != NULL) {
            printf("%s", buffer);
            line_count++;

            // Pagination Mantigi
            if (line_count == 10) {
                char input[10];
                if (fgets(input, sizeof(input), stdin) != NULL) {
                    if (input[0] == 'q') {
                        break;
                    }
                }
                line_count = 0;
            }
        }
    }

    // Normal kapanis sirasinda sistem kaynaklarini unlink et
    munmap(shm_ptr, sizeof(shared_memory_t));
    shm_unlink(SHM_NAME);
    sem_close(sem_mutex); sem_close(sem_empty); sem_close(sem_full);
    sem_unlink(SEM_MUTEX); sem_unlink(SEM_EMPTY); sem_unlink(SEM_FULL);

    return 0;
}