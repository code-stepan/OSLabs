#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>

const char* get_file_extension(const char *filename) {
    const char *dot = strrchr(filename, '.');
    if (!dot || dot == filename)
        return "";
    return dot + 1;
}

void list_files(const char *path, int current_depth, int recmin, int recmax) {
    DIR *dir = opendir(path);
    if (!dir) {
        perror("opendir");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        // скипаем родительский и текуший каталог
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char full_path[PATH_MAX];
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
        
        struct stat statbuf;
        if (lstat(full_path, &statbuf) == -1) {
            perror("lstat");
            continue;
        }

        if (S_ISDIR(statbuf.st_mode)) {
            if (current_depth < recmax) {
                list_files(full_path, current_depth + 1, recmin, recmax);
            }
        }
        else {
            if (current_depth >= recmin && current_depth <= recmax) {
                const char *ext  = get_file_extension(entry->d_name);
                printf(
                    "File: %s, Extension: %s, Disk address: %lu\n",
                    entry->d_name,
                    ext,
                    (unsigned long)stutbuf.st_ino
                );
            }
        }
    }

    closedir(dir);
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        fprintf(stderr, "Usage: %s <recmin> <recmax> <dir1> [<dir2> ...]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int recmin = atoi(argv[1]);
    int recmax = atoi(argv[2]);

    if (recmin < 0 || recmax < 0 || recmin > recmax) {
        fprintf(stderr, "recmin and recmax must be non-negative, and recmin <= recmax\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 3; i < argc; i++) {
        printf("Directory: %s\n", argv[i]);
        list_files(argv[i], 0, recmin, recmax);
    }

    return 0;
}
