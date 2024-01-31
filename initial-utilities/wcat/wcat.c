#include <stdio.h>
#include <stdlib.h>

int main(int argc,char *argv[])
{
    if(argc<2){
        exit(0);
    }
    int i=1;
    while(i<argc){
        FILE *p =fopen(argv[1],"r");

    if(p==NULL){
        printf("wcat:cannot open file\n");
        return 1;
    }
    char buffer[100];
    while(fgets(buffer,sizeof(buffer),p)!=NULL){
        printf("%s",buffer);
    }
    fclose(p);
    i++;
    }
    return 0;
}
