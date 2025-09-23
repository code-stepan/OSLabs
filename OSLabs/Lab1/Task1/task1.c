#include <stdio.h>
#include <stdlib.h>

void print_file_info(FILE *file) {
    printf("FILE structure info:\n");
    printf("_flags: %d\n", file->_flags);
    printf("_IO_read_ptr: %p\n", file->_IO_read_ptr);
    printf("_IO_read_end: %p\n", file->_IO_read_end);
    printf("_IO_read_base: %p\n", file->_IO_read_base);
    printf("_IO_write_ptr: %p\n", file->_IO_write_ptr);
    printf("_IO_write_end: %p\n", file->_IO_write_end);
    printf("_IO_write_base: %p\n", file->_IO_write_base);
    printf("_IO_buf_base: %p\n", file->_IO_buf_base);
    printf("_IO_buf_end: %p\n", file->_IO_buf_end);
    printf("_IO_save_base: %p\n", file->_IO_save_base);
    printf("_IO_backup_base: %p\n", file->_IO_backup_base);
    printf("_IO_save_end: %p\n", file->_IO_save_end);
    printf("\n");
}

int main(int argc, char *argv[]) {
	if (argc != 2) {
		printf("Usage: %s <filename>\n", argv[0]);
		return 1;
	}

	unsigned char data[] = {3, 1, 4, 1, 5, 9, 2, 6, 5, 3, 5};
	FILE *file = fopen(argv[1], "wb");
	if (!file) {
		perror("Error creating file");
		return 1;
	}
	
	fwrite(data, sizeof(unsigned char), sizeof(data), file);
	fclose(file);

	file = fopen(argv[1], "rb");
	if (!file) {
		perror("Error opening file");
		return 1;
	}

	unsigned char byte;
	printf("Reading file byte by byte:\n");
	while (fread(&byte, sizeof(unsigned char), 1, file) == 1) {
		printf("Byte: %d\n", byte);
        print_file_info(file);
	}
	fclose(file);

	file = fopen(argv[1], "rb");
	if (!file) {
		perror("Error opening file");
		return 1;
	}

	fseek(file, 3, SEEK_SET);
	unsigned char buffer[4];
	fread(buffer, sizeof(unsigned char), 4, file);

    printf("Buffer content after fread from position 3: ");
    for (int i = 0; i < 4; i++) {
        printf("%d ", buffer[i]);
    }
    printf("\n");

    fclose(file);
	return 0;
}