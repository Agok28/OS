#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>
#include <sys/wait.h>

void opening(const char *name, int indent, FILE *outputFile) {
    DIR *dir;
    struct dirent *entry;

    if (!(dir = opendir(name)))
        return;

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR) {
            char path[1024];
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;
            snprintf(path, sizeof(path), "%s/%s", name, entry->d_name);
            fprintf(outputFile, "%*s[%s]  Type-> Directory;\n", indent, "", entry->d_name);
            opening(path, indent + 2, outputFile);
        } else {
            fprintf(outputFile, "%*s- %s  Type-> File;\n", indent, "", entry->d_name);
        }
    }
    closedir(dir);
}

int compareDirectories(const char *dir1, const char *dir2) {
    struct dirent *entry1;
    struct dirent *entry2;
    DIR *dp1, *dp2;

    dp1 = opendir(dir1);
    dp2 = opendir(dir2);

    if (dp1 == NULL || dp2 == NULL) {
        perror("Error opening directories");
        return -1;
    }

    while ((entry1 = readdir(dp1)) != NULL) {
        entry2 = readdir(dp2);   
        if (entry2 == NULL) {
            closedir(dp1);
            closedir(dp2);
            return 0;
        }
        if (strcmp(entry1->d_name, entry2->d_name) != 0) {
            closedir(dp1);
            closedir(dp2);
            return 0;
        }
    }
    if (readdir(dp2) != NULL) {
        closedir(dp1);
        closedir(dp2);
        return 0;
    }

    closedir(dp1);
    closedir(dp2);
    return 1;
}

void readFilesIntoStrings(const char *filename1, const char *filename2, char *string1, char *string2) {
    FILE *file1 = fopen(filename1, "r");
    FILE *file2 = fopen(filename2, "r");

    if (file1 == NULL || file2 == NULL) {
        perror("Error opening files");
        exit(EXIT_FAILURE);
    }

    char buffer[1000];
    string1[0] = '\0'; 
    while (fgets(buffer, sizeof(buffer), file1) != NULL) {
        strcat(string1, buffer);
    }

    string2[0] = '\0'; 
    while (fgets(buffer, sizeof(buffer), file2) != NULL) {
        strcat(string2, buffer);
    }

    fclose(file1);
    fclose(file2);
}

void compareStrings(const char *str1, const char *str2) {
    if (strcmp(str1, str2) == 0) {
        printf("The strings are equal.\n");
    } else {
        printf("The strings are different.\n");
    }
}

int main(int argc, char **argv) {
    if (argc < 3) {
        printf("Usage: %s <directory1> <directory2> ...\n", argv[0]);
        return 1;
    }

    for (int i = 1; i < argc; i++) {
        int pid = fork();
        if (pid == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {  
            char snapshotFile[50];
            char currentDir[1024];

            snprintf(snapshotFile, sizeof(snapshotFile), "snapshot%d.txt", i);
            snprintf(currentDir, sizeof(currentDir), "%s", argv[i]);

            FILE *outputFile1 = fopen(snapshotFile, "w");
            FILE *outputFile2 = fopen(snapshotFile, "a"); // Open in append mode
            char string1[10000]; // Increased buffer size to accommodate larger files
            char string2[10000];

            if (outputFile1 == NULL || outputFile2 == NULL) {
                perror("The file does not open.");
                exit(EXIT_FAILURE);
            }

            fprintf(outputFile1, "\n\n\nCurrent directory:\n");
            opening(argv[i], 0, outputFile1);

            if (argc > i + 1 && compareDirectories(argv[i], argv[i+1]) == 1) {
            printf("The directories %s and %s are the same.\n", argv[i], argv[i+1]);
        } else {
            printf("The directories %s and %s are different.\n", argv[i], argv[i+1]);
        }

            readFilesIntoStrings(snapshotFile, snapshotFile, string1, string2);
            compareStrings(string1, string2);

            fclose(outputFile1);
            fclose(outputFile2);

            exit(EXIT_SUCCESS);
        }
    }
    int status;
    pid_t wpid;
    while ((wpid = wait(&status)) > 0);

    return 0;
}
