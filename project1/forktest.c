#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>
#include <locale.h>
#include <string.h>
#include <ftw.h>
#include <stdio.h>

void recur(DIR *pDir, struct dirent *pDirent, char* path){
    pid_t pid;
    /*
    if (pDir == NULL) {
        printf ("Cannot open directory '%s'\n", v[1]);
        return 1;
    }*/
    while ((pDirent = readdir(pDir)) != NULL) {
        if (pDirent->d_name[0] == '.'){continue;}
        printf("This is %s\n", pDirent->d_name);
        if (strchr(pDirent->d_name, '.') == NULL){
            pid = fork();
            if (pid < 0){
                printf("error\n");
                exit(0);
            } else if (pid == 0){   // child
                printf("child in %s\n", pDirent->d_name);
                strcat(path, "/");
                DIR *newdir = opendir(strcat(path,pDirent->d_name));
                recur(newdir, pDirent, path);
                if ((pDirent = readdir(newdir)) != NULL){
                    printf("KKKK");
                }
            } else if (pid > 0){    // parent
                printf("parent in %s\n", pDirent->d_name);
            }
        }
    }
    closedir (pDir);
}

int main(int c, char *v[]){
    struct dirent *pDirent;
    DIR *pDir;
    pDir = opendir (v[1]);
    char path[200];
    strncpy(path, v[1], strlen(v[1]));
    char* ptr = path;
    recur(pDir, pDirent, ptr);

    return 0;
}