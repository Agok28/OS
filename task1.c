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
            fprintf(outputFile, "%*s[%s] -> %d , Length: %dbytes;\n", indent, "", entry->d_name,entry->d_type,entry->d_reclen);
            opening(path, indent + 2,outputFile);
        } else {
            fprintf(outputFile, "%*s- %s->%d , Length: %d bytes;\n", indent, "", entry->d_name,entry->d_type,entry->d_reclen);
        }
    }
    closedir(dir);
}

int main(int argc, char ** argv){
    FILE * outputFile = fopen("output.txt","a");
    if(outputFile == NULL){
        perror("The file does not open.");
        exit(EXIT_FAILURE);
    }
    fprintf(outputFile,"\n\n\nCurrent directory:\n");
    opening(".", 0,outputFile);
    fclose(outputFile);
    return 0;
}