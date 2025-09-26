#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

int is_prime(unsigned int n) {
    if (n <= 1) return 0;
    if (n <= 3) return 1;
    if (n % 2 == 0 || n % 3 == 0) return 0;

    for (int i = 5; i * i <= n; i += 6) {
        if (n % i == 0 || n % (i + 2) == 0) return 0;
    }
    return 1;
}

void xor8(FILE *file) {
    unsigned char result = 0;
    unsigned char byte;

    fseek(file, 0, SEEK_SET);
    while (fread(&byte, 1, 1, file) == 1) {
        result ^= byte;
    }

    printf("XOR8: %u\n", result);
}

void xorodd(FILE *file) {
    unsigned int result = 0;
    unsigned int value;
    int has_prime = 0;

    fseek(file, 0, SEEK_SET);
    while (fread(&value, sizeof(unsigned int), 1, file) == 1) {
        unsigned char *bytes = (unsigned char *)&value;
        has_prime = 0;

        for (int i = 0; i < 4; i++) {
            if (is_prime(bytes[i])) {
                has_prime = 1;
                break;
            }
        }

        if (has_prime) {
            result ^= value;
        }
    }

    printf("XORODD: %u\n", result);
}

void mask_count(FILE *file, unsigned int mask) {
    unsigned int value;
    int count = 0;

    fseek(file, 0, SEEK_SET);
    while (fread(&value, sizeof(unsigned int), 1, file) == 1) {
        if ((value & mask) == mask) {
            count++;
        }
    }

    printf("Mask 0x%X: %d\n", mask, count);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Неверные аргументы\n");
        printf("Команды: xor8, xorodd, mask <hex>\n");
        return 1;
    }

    FILE *file = fopen(argv[1], "rb");
    if (!file) {
        perror("Ошибка открытия файла");
        return 1;
    }

    if (strcmp(argv[2], "xor8") == 0) {
        xor8(file);
    } else if (strcmp(argv[2], "xorodd") == 0) {
        xorodd(file);
    } else if (strcmp(argv[2], "mask") == 0 && argc == 4) {
        unsigned int mask;
        sscanf(argv[3], "%X", &mask);
        mask_count(file, mask);
    } else {
        printf("Неверный флаг\n");
    }

    fclose(file);
    return 0;
}
