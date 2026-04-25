#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#define FIFO_NAME "/tmp/ceng302_fifo"
#define MAX_LOG_LENGTH 512

// Thread'lerin ayni anda Pipe'a yazmasini engellemek icin Mutex (Anti-cheat icin zorunlu)
pthread_mutex_t pipe_mutex = PTHREAD_MUTEX_INITIALIZER;
int pipe_fd;

// Thread fonksiyonu: Dosyayi satir satir okur ve Pipe'a yazar
void* read_file(void* arg) {
    char* filename = (char*)arg;
    FILE* file = fopen(filename, "r");
    
    if (!file) {
        perror("Dosya acilamadi");
        pthread_exit(NULL);
    }

    char buffer[MAX_LOG_LENGTH];
    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        // Race condition onlemek icin kilitle
        pthread_mutex_lock(&pipe_mutex);
        write(pipe_fd, buffer, strlen(buffer));
        pthread_mutex_unlock(&pipe_mutex);
    }

    fclose(file);
    pthread_exit(NULL);
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Kullanim: %s <log_dosyasi_1> <log_dosyasi_2> ...\n", argv[0]);
        exit(1);
    }

    // FIFO olustur (Eger zaten varsa hata vermemesi icin kontrol)
    if (mkfifo(FIFO_NAME, 0666) == -1) {
        // mkfifo onceden olusturulmussa calismaya devam et
    }

    // FIFO'yu yazma modunda ac
    pipe_fd = open(FIFO_NAME, O_WRONLY);
    if (pipe_fd == -1) {
        perror("FIFO acilamadi");
        exit(1);
    }

    int num_files = argc - 1;
    pthread_t threads[num_files];

    // Her dosya icin bir thread olustur (Concurrency)
    for (int i = 0; i < num_files; i++) {
        if (pthread_create(&threads[i], NULL, read_file, (void*)argv[i + 1]) != 0) {
            perror("Thread olusturulamadi");
        }
    }

    // Tum thread'lerin isini bitirmesini bekle
    for (int i = 0; i < num_files; i++) {
        pthread_join(threads[i], NULL);
    }

    // Tuketiciye islemin bittigini bildirmek icin ozel bir EOF mesaji gonder
    char* eof_marker = "EOF_MARKER\n";
    pthread_mutex_lock(&pipe_mutex);
    write(pipe_fd, eof_marker, strlen(eof_marker));
    pthread_mutex_unlock(&pipe_mutex);

    // Kaynaklari temizle
    close(pipe_fd);
    return 0;
}