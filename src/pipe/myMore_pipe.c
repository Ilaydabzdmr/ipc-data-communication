#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>

#define FIFO_NAME "/tmp/ceng302_fifo"
#define MAX_LOG_LENGTH 512

int pipe_fd;
FILE* pipe_stream;

// Signal Handling (Graceful Shutdown)
void handle_sigint(int sig) {
    printf("\n[SIGINT Algilandi] myMore_pipe guvenli sekilde kapatiliyor...\n");
    if (pipe_stream) fclose(pipe_stream);
    unlink(FIFO_NAME); // FIFO'yu sistemden temizle
    exit(0);
}

int main() {
    // Ctrl+C (SIGINT) sinyalini yakala
    signal(SIGINT, handle_sigint);

    // FIFO'yu okuma modunda ac
    pipe_fd = open(FIFO_NAME, O_RDONLY);
    if (pipe_fd == -1) {
        perror("FIFO okuma icin acilamadi, once ureticiyi (myData) calistirin");
        exit(1);
    }

    // fdopen kullanarak satir satir fgets ile okuma kolayligi sagliyoruz
    pipe_stream = fdopen(pipe_fd, "r");
    char buffer[MAX_LOG_LENGTH];
    int line_count = 0;

    while (fgets(buffer, sizeof(buffer), pipe_stream) != NULL) {
        // Ureticilerin isleminin bittigini gosteren isaret
        if (strcmp(buffer, "EOF_MARKER\n") == 0) {
            break;
        }

        // Sadece CRITICAL veya ERROR iceren loglari filtrele
        if (strstr(buffer, "CRITICAL") != NULL || strstr(buffer, "ERROR") != NULL) {
            printf("%s", buffer);
            line_count++;

            // Pagination (10 satirda bir bekleme)
            if (line_count == 10) {
                char input[10];
                // Terminalden kullanici girisi bekle
                if (fgets(input, sizeof(input), stdin) != NULL) {
                    if (input[0] == 'q') {
                        break; // q'ya basilirsa donguden cik
                    }
                    // space veya enter basilirsa dongu devam eder (filtrelemeye devam)
                }
                line_count = 0; // Sayaci sifirla
            }
        }
    }

    // Normal kapanis temizligi
    fclose(pipe_stream);
    unlink(FIFO_NAME);
    return 0;
}