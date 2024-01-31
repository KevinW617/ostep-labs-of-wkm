#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void processfile(const char* filename,const char* search_item){
    FILE* file=fopen(filename,"r");
    if(file==NULL){
        printf("wgrep: cannot open file\n");
        exit(1);
    }
    size_t line_length = 0;
    char* line = NULL;
    __ssize_t read;
    while((read=getline(&line,&line_length,file))!=-1){
        if (strlen(search_item) == 0) {
                printf("%s", line);
            }
        if(strstr(line,search_item)!=NULL){
            printf("%s",line);
        }       
    }
    free(line);
    fclose(file);
}

int main(int argc,char *argv[]){
    if(argc==1){
        printf("wgrep: searchterm [file ...]\n");
        return 1;
    }
    const char* search_item = argv[1];
    if(argc==2){
        char* line =NULL;
        size_t line_length =0;
        __ssize_t read;
        while((read=getline(&line,&line_length,stdin))!=-1){
            if (strlen(search_item) == 0) {
                printf("%s", line);
            }
            if(strstr(line,search_item)!=NULL){
                printf("%s",line);
            }
        }
        free(line);
    }
    else{
        for(int i =2;i<argc;i++){
            const char* filename = argv[i];
            processfile(filename,search_item);
        }
    }
    return 0;
}
