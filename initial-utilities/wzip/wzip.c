#include<stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) {
	if(argc == 1) {
		printf("wzip: file1 [file2 ...]\n");
		exit(1);
	}
	int count = 0;
	int counter =0 ;
	char buffer[512];
	
	for(int i = 1; i<argc; i++) {
		FILE * fd = fopen(argv[i], "r");
		if(fd == NULL) {
			printf("wzip: cannot open file\n");
			exit(1);
		}
		counter=0;
		count++;
		while ((fgets(buffer, sizeof(buffer), fd)) != NULL) {
			counter++;
			for(int i = 0; ; i++) {
				if(buffer[i] == buffer[i+1]) {
					count++;
				}
				else {
					if(buffer[i+1]=='\n'){
					fwrite(&count, sizeof(int), 1, stdout);
					fwrite(&buffer[i], sizeof(char), 1, stdout);
					count=1;
					fwrite(&count, sizeof(int), 1, stdout);
					fwrite(&buffer[i+1], sizeof(char), 1, stdout);	
					break;
					}
					fwrite(&count, sizeof(int), 1, stdout);
					fwrite(&buffer[i], sizeof(char), 1, stdout);
					count=1;
                    }
			}
		}
		
	fclose(fd);
	}
	// fwrite(&count, sizeof(int), 1, stdout);
	// fwrite(&current_char, sizeof(char), 1, stdout);
	// exit(0);

}