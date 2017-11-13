#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char string[40][10] = {"125","127","d","127","128","e","123","124","a.txt","123","125","b","125","126","c.txt"};
static char stringT[5][10];
static int cursor=0;

int contains(char strArray[5][10],char* str){
    for(int i =0;i<5;i++){
      if(strcmp(strArray[i],str)==0)
        return 1;
    }
    return 0;
}
void travel(char* init,int level){
int index=0;
for(;index<15;index+=3){
    if(0==contains(stringT,string[index+1])&&0==strcmp(init,string[index])){
      strcpy(stringT[cursor++],string[index]);
      printf("level %d",level);
      for(int j=0;j<=level;j++)
        printf("---");
        printf("parent: %s pid : %s file : %s \n",string[index],string[index+1],string[index+2]);
      travel(string[index+1],level+1);
    }
  }

}

int main(int c, char *v[]){
int i=0;
char init[4] = "123";
int level=0;
for(;i<15;i+=3){
  if(0==strcmp(init,string[i])){
    strcpy(stringT[cursor++],string[i]);
    printf("level %d",level);
    for(int j=0;j<=level;j++)
      printf("---");
    printf("parent: %s pid : %s file : %s \n",string[i],string[i+1],string[i+2]);
    travel(string[i+1],level+1);
  }
}

}
