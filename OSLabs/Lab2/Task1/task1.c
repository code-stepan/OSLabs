#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

int main() {
    printf("Current Process ID (PID): %d\n", getpid());
    printf("Parent Process ID (PPID): %d\n", getppid());
    printf("Process Group ID (PGID): %d\n", getpgrp());
    printf("Real User ID (UID): %d\n", getuid());
    printf("Real Group ID (GID): %d\n", getgid());
    printf("Effective User ID (EUID): %d\n", geteuid());
    printf("Effective Group ID (EGID): %d\n", getegid());

    return 0;
}
