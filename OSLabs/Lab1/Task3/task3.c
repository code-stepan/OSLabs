#include <stdio.h>
#include <stdlib.h>

#define BUFFER_SIZE 4096

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <source> <destination>\n", argv[0]);
        return 1;
    }

    FILE *source = fopen(argv[1], "rb");
    if (!source) {
        perror("Error opening source file");
        return 1;
    }

    FILE *dest = fopen(argv[2], "wb");
    if (!dest) {
        perror("Error opening destination file");
        fclose(source);
        return 1;
    }

    char buffer[BUFFER_SIZE];
    size_t bytes_read;

    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, source)) > 0) {
        size_t bytes_written = fwrite(buffer, 1, bytes_read, dest);
        if (bytes_written != bytes_read) {
            perror("Error writing to destination file");
            break;
        }
    }

    fclose(source);
    fclose(dest);

    printf("File copied successfully from %s to %s\n", argv[1], argv[2]);
    return 0;
}