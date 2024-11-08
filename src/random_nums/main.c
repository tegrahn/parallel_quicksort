#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char* argv[]) {
    if (argc != 3)
    {
        printf("Usage: %s ints to generate, max int \n", argv[0]);
        return 1;
    }
    int N = atoi(argv[1]);  // Number of integers to generate
    char* ints_c = argv[1]; //Maximum int as char* for filename
    int max_int = atoi(argv[2]);     // Maximum int
    int i;
    FILE *file = fopen(ints_c, "w");
    if (file == NULL) {
        printf("Error opening file\n");
        return 1;
    }
    for(i = 0; i < N; i++)  {
        int randy = rand() % max_int;
        fwrite(&randy, sizeof(int), 1, file);
    }
    fclose(file);
    return 0;
}