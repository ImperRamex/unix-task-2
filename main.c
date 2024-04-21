#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <getopt.h>
#include <signal.h>

#define LOCK_FILE_EXTENSION ".lck"
static int LOCKS = 0;
static pid_t PID;

void lock_file(const char *filename, char write_buf[5], int w_buf_size) {
    char lockfile[strlen(filename) + strlen(LOCK_FILE_EXTENSION) + 1];
    int fd = -1;
    
    strcpy(lockfile, filename);
    strcat(lockfile, LOCK_FILE_EXTENSION);

    while (fd == -1) {
        fd = open(lockfile, O_CREAT | O_EXCL | O_RDWR, 0640);
    }
    write(fd, write_buf, w_buf_size);
    close(fd);
    
}

void unlock_file(const char *filename) {
    char lockfile[strlen(filename) + strlen(LOCK_FILE_EXTENSION) + 1];
    int pid;
    
    strcpy(lockfile, filename);
    strcat(lockfile, LOCK_FILE_EXTENSION);

    FILE *fp = fopen(lockfile, "r");
    if (fp == NULL) {
        perror("Failed to open lock file");
        exit(EXIT_FAILURE);
    }

    fscanf(fp, "%d", &pid);
    fclose(fp);

    if (pid != getpid()) {
        fprintf(stderr, "Error: Another process has acquired the lock\n");
        exit(EXIT_FAILURE);
    }

    if (unlink(lockfile) == -1) {
        perror("Failed to remove lock file");
        exit(EXIT_FAILURE);
    }
}

void signal_func(int sig) {
    int stat_str_len = snprintf(NULL, 0, "PID %d Succseed locks: %d\n", PID, LOCKS);
    char* stat_str = malloc(stat_str_len+1);
    snprintf(stat_str, stat_str_len+1, "PID %d Succseed locks: %d\n", PID, LOCKS);
    int stat_fd = open("statistic", O_WRONLY | O_APPEND | O_CREAT, 0640);
    write(stat_fd, stat_str, stat_str_len);
    free(stat_str);
    exit(0);
}

int main(int argc, char *argv[]) {
    signal(SIGINT, signal_func);

    const char *filename = argv[1];
    PID = getpid();
    while (1) {
        char write_buf[10];
        sprintf(write_buf, "%d", PID);
        int w_buf_size = strlen(write_buf);
        lock_file(filename,write_buf, w_buf_size);
        sleep(1);
        unlock_file(filename);
        LOCKS++;
    }

    return 0;
}
