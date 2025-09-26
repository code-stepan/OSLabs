#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <limits.h>

// Функция для поиска строки в файле, возвращает количество вхождений
int search_in_file(const char *filename, const char *search_str) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("fopen");
        return -1;
    }

    char line[1024];
    int count = 0;
    while (fgets(line, sizeof(line), file)) {
        char *ptr = line;
        while ((ptr = strstr(ptr, search_str)) != NULL) {
            count++;
            ptr += strlen(search_str);
        }
    }

    fclose(file);
    return count;
}

// Рекурсивная функция для создания сбалансированного fork-дерева
void create_fork_tree(int depth) {
    if (depth <= 0) {
        return;
    }
    
    pid_t pid1 = fork();
    if (pid1 == 0) {
        // Дочерний процесс 1
        create_fork_tree(depth - 1);
        exit(EXIT_SUCCESS);
    } else if (pid1 < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    
    pid_t pid2 = fork();
    if (pid2 == 0) {
        // Дочерний процесс 2
        create_fork_tree(depth - 1);
        exit(EXIT_SUCCESS);
    } else if (pid2 < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    
    // Родительский процесс ждет завершения дочерних
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <file_list> <search_str>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *file_list_path = argv[1];
    const char *search_str = argv[2];

    FILE *list_file = fopen(file_list_path, "r");
    if (!list_file) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

    char filename[PATH_MAX];
    char **unique_files = NULL;
    size_t unique_count = 0;

    // Собираем уникальные файлы
    while (fgets(filename, sizeof(filename), list_file)) {
        filename[strcspn(filename, "\n")] = '\0';

        int is_unique = 1;
        for (size_t i = 0; i < unique_count; i++) {
            if (strcmp(unique_files[i], filename) == 0) {
                is_unique = 0;
                break;
            }
        }

        if (is_unique && strlen(filename) > 0) {
            unique_files = realloc(unique_files, (unique_count + 1) * sizeof(char*));
            unique_files[unique_count] = strdup(filename);
            unique_count++;
        }
    }

    fclose(list_file);

    if (unique_count == 0) {
        printf("No files to search.\n");
        printf("Starting controlled fork bomb with height %lu...\n", strlen(search_str));
        create_fork_tree(strlen(search_str));
        return 0;
    }

    // Создаем отдельный pipe для каждого процесса
    int **pipes = malloc(unique_count * sizeof(int*));
    pid_t *pids = malloc(unique_count * sizeof(pid_t));
    
    for (size_t i = 0; i < unique_count; i++) {
        pipes[i] = malloc(2 * sizeof(int));
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            exit(EXIT_FAILURE);
        }

        pids[i] = fork();
        if (pids[i] == 0) {
            // Дочерний процесс
            close(pipes[i][0]); // Закрываем чтение
            
            int count = search_in_file(unique_files[i], search_str);
            
            if (count > 0) {
                // Отправляем результат через pipe
                char result[PATH_MAX + 50];
                snprintf(result, sizeof(result), "%s:%d", unique_files[i], count);
                write(pipes[i][1], result, strlen(result) + 1);
            }
            
            close(pipes[i][1]);
            free(unique_files[i]);
            exit(EXIT_SUCCESS);
        } else if (pids[i] < 0) {
            perror("fork");
            exit(EXIT_FAILURE);
        }
        
        close(pipes[i][1]); // Закрываем запись в родительском процессе
    }

    // Читаем результаты из всех pipes
    int found_count = 0;
    printf("Search results:\n");
    
    for (size_t i = 0; i < unique_count; i++) {
        char buffer[PATH_MAX + 50];
        ssize_t bytes_read = read(pipes[i][0], buffer, sizeof(buffer));
        
        if (bytes_read > 0) {
            char *filename = strtok(buffer, ":");
            char *count_str = strtok(NULL, ":");
            if (filename && count_str) {
                printf("Found in %s: %s occurrence(s)\n", filename, count_str);
                found_count++;
            }
        }
        
        close(pipes[i][0]);
        free(pipes[i]);
    }

    // Ждем завершения всех дочерних процессов
    for (size_t i = 0; i < unique_count; i++) {
        waitpid(pids[i], NULL, 0);
        free(unique_files[i]);
    }

    free(unique_files);
    free(pipes);
    free(pids);

    // Если строка не найдена ни в одном файле
    if (found_count == 0) {
        printf("String '%s' not found in any file.\n", search_str);
        printf("Starting controlled fork bomb with height %lu...\n", strlen(search_str));
        create_fork_tree(strlen(search_str));
    }

    return 0;
}