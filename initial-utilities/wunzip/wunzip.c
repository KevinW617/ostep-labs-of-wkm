#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>

void decompress_file(FILE* file) {
    int count;
    char character;

    while (fread(&count, sizeof(int), 1, file) !=0 && fread(&character, sizeof(char), 1, file)!=0) 
    {
        for (int i = 0; i < count; i++) {
            putchar(character);
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("wunzip: file1 [file2 ...]\n");
        return 1;
    }

    for (int i = 1; i < argc; i++) {
        FILE* file = fopen(argv[i], "rb");
        if (file == NULL) {
            printf("wunzip:cannot open file\n");
            exit(1);
        }
        decompress_file(file);
        fclose(file);
    }

    return 0;
}
