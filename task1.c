#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>



void opening(const char *name, int indent,FILE * outputFile)
{
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
            fprintf(outputFile, "%*s[%s]  Type-> %d , Length: %dbytes;\n", indent, "", entry->d_name,entry->d_type,entry->d_reclen);
            opening(path, indent + 2,outputFile);
        } else {
            fprintf(outputFile, "%*s- %s  Type->%d , Length: %d bytes;\n", indent, "", entry->d_name,entry->d_type,entry->d_reclen);
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
    while (fgets(buffer, 1000, file1) != NULL) {
        strcat(string1, buffer);
    }

    string2[0] = '\0'; 
    while (fgets(buffer, 1000, file2) != NULL) {
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



int main(int argc, char ** argv){
    
    FILE * outputFile1 = fopen("snapshot1.txt","w");
    FILE * outputFile2 = fopen("snapshot2.txt","w");
    char string1[1000];
    char string2[1000];
    if(outputFile1 == NULL){
        perror("The file does not open.");
        exit(EXIT_FAILURE);
    }
    char* path1 = argv[1];
    char* path2 =argv[2];

    fprintf(outputFile1,"\n\n\nCurrent directory:\n");
    opening(path1, 0,outputFile1);
    opening(path2,0,outputFile2);

    if(compareDirectories(argv[1],argv[2]) == 1){
        printf("The directories are the same.");
    }else{
        printf("The directories are different.");
    }

    readFilesIntoStrings("snapshot1.txt","snapshot1.txt",string1,string2);
    compareStrings(string1,string2);

    fclose(outputFile1);
    fclose(outputFile2);
    return 0;
}

