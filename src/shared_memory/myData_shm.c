#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <semaphore.h>

#define SHM_NAME "/ceng302_shm"
#define SEM_MUTEX "/ceng302_mutex"
#define SEM_EMPTY "/ceng302_empty"
#define SEM_FULL "/ceng302_full"
#define MAX_LOG_LENGTH 512

// Paylasilan bellek yapisi
typedef struct {
    char message[MAX_LOG_LENGTH];
} shared_memory_t;

shared_memory_t *shm_ptr;
sem_t *sem_mutex, *sem_empty, *sem_full;

// Thread fonksiyonu
void* read_file(void* arg) {
    char* filename = (char*)arg;
    FILE* file = fopen(filename, "r");
    
    if (!file) {
        perror("Dosya acilamadi");
        pthread_exit(NULL);
    }

    char buffer[MAX_LOG_LENGTH];
    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        sem_wait(sem_empty); // Bos yer bekle
        sem_wait(sem_mutex); // Kritik alana gir (Race condition engelleme)
        
        strcpy(shm_ptr->message, buffer); // Veriyi SHM'ye yaz
        
        sem_post(sem_mutex); // Kritik alandan cik
        sem_post(sem_full);  // Tuketiciye veri hazir sinyali ver
    }

    fclose(file);
    pthread_exit(NULL);
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Kullanim: %s <log_dosyasi_1> <log_dosyasi_2> ...\n", argv[0]);
        exit(1);
    }

    // Shared Memory olusturma
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    ftruncate(shm_fd, sizeof(shared_memory_t));
    shm_ptr = mmap(0, sizeof(shared_memory_t), PROT_WRITE, MAP_SHARED, shm_fd, 0);

    // Semoforlari olustur ve baslat
    sem_mutex = sem_open(SEM_MUTEX, O_CREAT, 0666, 1);
    sem_empty = sem_open(SEM_EMPTY, O_CREAT, 0666, 1); // 1 mesajlik tampon
    sem_full = sem_open(SEM_FULL, O_CREAT, 0666, 0);

    int num_files = argc - 1;
    pthread_t threads[num_files];

    // Thread'leri baslat
    for (int i = 0; i < num_files; i++) {
        pthread_create(&threads[i], NULL, read_file, (void*)argv[i + 1]);
    }

    // Thread'lerin bitmesini bekle
    for (int i = 0; i < num_files; i++) {
        pthread_join(threads[i], NULL);
    }

    // Tuketiciye bitis sinyalini gonder
    sem_wait(sem_empty);
    sem_wait(sem_mutex);
    strcpy(shm_ptr->message, "EOF_MARKER\n");
    sem_post(sem_mutex);
    sem_post(sem_full);

    // Uretici tarafinda kaynaklari kapat
    munmap(shm_ptr, sizeof(shared_memory_t));
    close(shm_fd);
    
    return 0;
}