#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <time.h>
#include <sys/wait.h>

void traverseDir(const char *path, FILE *file) {
    DIR *dir;
    struct dirent *entry;
    struct stat statbuf;
    char timestamp[20];

    if (!(dir = opendir(path))) {
        fprintf(stderr, "Error: Unable to open directory %s\n", path);
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        char filepath[1024];
        snprintf(filepath, sizeof(filepath), "%s/%s", path, entry->d_name);

        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        if (lstat(filepath, &statbuf) < 0) {
            fprintf(stderr, "Error: Unable to get file status %s\n", filepath);
            continue;
        }

        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localtime(&statbuf.st_mtime));

        fprintf(file, "Entry Name: %s\n", entry->d_name);
        fprintf(file, "I-node: %ld\n", (long)statbuf.st_ino);
        fprintf(file, "Permissions: %o\n", statbuf.st_mode & 0777);
        fprintf(file, "Size: %ld bytes\n", statbuf.st_size);
        fprintf(file, "Last Modified: %s\n", timestamp);
        fprintf(file, "--------------------------------\n");

        if(S_ISDIR(statbuf.st_mode)) {
            traverseDir(filepath, file);
        }
    }
    closedir(dir);
}

void compareDirectories(const char *dir1, const char *dir2) {
    printf("Comparing directories %s and %s:\n", dir1, dir2);

    DIR *dp1 = opendir(dir1);
    DIR *dp2 = opendir(dir2);

    if (!dp1 || !dp2) {
        fprintf(stderr, "Error: Unable to open directories\n");
        return;
    }

    struct dirent *entry1, *entry2;
    struct stat statbuf1, statbuf2;

    while ((entry1 = readdir(dp1)) != NULL) {
        char filepath1[1024];
        snprintf(filepath1, sizeof(filepath1), "%s/%s", dir1, entry1->d_name);

        if (strcmp(entry1->d_name, ".") == 0 || strcmp(entry1->d_name, "..") == 0)
            continue;

        if (lstat(filepath1, &statbuf1) < 0) {
            fprintf(stderr, "Error: Unable to get file status %s\n", filepath1);
            continue;
        }

        char filepath2[1024];
        snprintf(filepath2, sizeof(filepath2), "%s/%s", dir2, entry1->d_name);

        if (lstat(filepath2, &statbuf2) < 0) {
            printf("File '%s' in directory %s is missing in directory %s\n", entry1->d_name, dir1, dir2);
            continue;
        }

        if (statbuf1.st_mtime != statbuf2.st_mtime) {
            printf("File '%s' in directory %s has been modified\n", entry1->d_name, dir1);
        }
    }

    rewinddir(dp1);
    while ((entry2 = readdir(dp2)) != NULL) {
        char filepath2[1024];
        snprintf(filepath2, sizeof(filepath2), "%s/%s", dir2, entry2->d_name);

        if (strcmp(entry2->d_name, ".") == 0 || strcmp(entry2->d_name, "..") == 0)
            continue;

        if (lstat(filepath2, &statbuf2) < 0) {
            fprintf(stderr, "Error: Unable to get file status %s\n", filepath2);
            continue;
        }

        char filepath1[1024];
        snprintf(filepath1, sizeof(filepath1), "%s/%s", dir1, entry2->d_name);

        if (lstat(filepath1, &statbuf1) < 0) {
            printf("File '%s' in directory %s is missing in directory %s\n", entry2->d_name, dir2, dir1);
        }
    }

    closedir(dp1);
    closedir(dp2);
}

int main(int argc, char *argv[]) {
    if (argc < 3 || argc > 11) {
        fprintf(stderr, "Usage: %s <directory_path1> <directory_path2> [<directory_path3> ... <directory_path10>]\n", argv[0]);
        return EXIT_FAILURE;
    }

    for (int i = 1; i < argc; i++) {
        pid_t pid = fork();

        if (pid < 0) {
            fprintf(stderr, "Error: Fork failed for directory %s\n", argv[i]);
            continue;
        } else if (pid == 0) { 
            FILE *snapshotFile;
            char filename[20];
            snprintf(filename, sizeof(filename), "snapshot%d.txt", i);

            snapshotFile = fopen(filename, "w");
            if (!snapshotFile) {
                fprintf(stderr, "Error: Unable to create %s\n", filename);
                exit(EXIT_FAILURE);
            }

            traverseDir(argv[i], snapshotFile);

            fclose(snapshotFile);
            printf("Snapshot for '%s' created successfully in %s\n", argv[i], filename);
            exit(EXIT_SUCCESS);
        }
    }

    int status;
    pid_t wpid;
    while ((wpid = wait(&status)) > 0) {
        if (WIFEXITED(status)) {
            printf("The process with PID %d has ended with code %d.\n", (int)wpid, WEXITSTATUS(status));
        } else {
            fprintf(stderr, "The process with PID %d has ended abnormally.\n", (int)wpid);
        }
    }

    if (argc == 3) { 
        compareDirectories(argv[1], argv[2]);
    }

    return EXIT_SUCCESS;
}
