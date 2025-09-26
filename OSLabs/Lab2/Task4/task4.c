#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <limits.h>

const char* get_file_extension(const char *filename) {
    const char *dot = strrchr(filename, '.');
    if (!dot || dot == filename) {
        return "";
    }
    return dot + 1;
}

void list_directory(const char *path, int depth, int recmin, int recmax) {
    DIR *dir = opendir(path);
    if (!dir) {
        perror("opendir");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char full_path[PATH_MAX];
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);

        for (int i = 0; i < depth; i++) {
            printf("  ");
        }

        struct stat statbuf;
        if (lstat(full_path, &statbuf) == -1) {
            perror("lstat");
            continue;
        }

        if (S_ISDIR(statbuf.st_mode)) {
            printf("%s/\n", entry->d_name);
            // Рекурсивно обходим подкаталог, если глубина не превышает recmax
            if (depth + 1 <= recmax) {
                list_directory(full_path, depth + 1, recmin, recmax);
            }
        }
        else if (depth >= recmin && depth <= recmax) {
            const char *ext = get_file_extension(entry->d_name);
            printf(
                "%s (Extension: %s, Disk Address: %lu)\n",
                entry->d_name,
                ext,
                (unsigned long)statbuf.st_ino
            );
        }
    }

    closedir(dir);
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        fprintf(stderr, "Usage: %s <recmin> <recmax> <directory>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int recmin = atoi(argv[1]);
    int recmax = atoi(argv[2]);

    if (recmin < 0 || recmax < 0 || recmin > recmax) {
        fprintf(stderr, "recmin and recmax must be non-negative, and recmin <= recmax\n");
        exit(EXIT_FAILURE);
    }

    printf("Directory structure for %s (depth %d to %d):\n", argv[3], recmin, recmax);
    list_directory(argv[3], 0, recmin, recmax);

    return 0;
}
