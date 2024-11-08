#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sort_funcs.h"

void read_ints(const char *filename, int N, int *list)
{
  FILE *file = fopen(filename, "r");
  if (file == NULL)
  {
    printf("Error opening file %s\n", filename);
    exit(1);
    return;
  }
  int num;
  for (int i = 0; i < N; i++)
  {
    if (fread(&num, sizeof(int), 1, file) != 1)
    {
      printf("Error reading value from file %s\n", filename);
      exit(1);
    }
    list[i] = num;
  }
  // Close the file
  fclose(file);
}

int main(int argc, char* argv[]) {
  //Read in num threads, size of numbers to sort, file_name to sort
  // Check if the correct number of arguments are provided
  if (argc != 4)
  {
    printf("Usage: %s Integers to sort from file, filename relative to .exe, num_threads, \n", argv[0]);
    return 1;
  }
  int N = atoi(argv[1]);  // Number of integers to sort
  char *filename = argv[2];       // File to read numbers
  int n_threads = atoi(argv[3]);     // Number of threads
  
  int* list_to_sort = (int*)malloc(N*sizeof(int));
  read_ints(filename, N, list_to_sort);
  
  // Set-up truly sorted list
  int* true_sort = (int*)malloc(N*sizeof(int));
  memcpy(true_sort, list_to_sort, N * sizeof(int));
  qsort(true_sort, N ,sizeof(int), compare);
  omp_set_nested(1);

  omp_set_num_threads(n_threads);
  // Sort list
  p_quick_sort(list_to_sort,N,n_threads);
  
  // Ensure truly sorted by comparing to inbuilt qsort
  for (int i = 0; i<N; i++){
    if (list_to_sort[i] != true_sort[i]){
      printf("List is not sorted :(");
      return 0;
    }
  }

  free(list_to_sort);
  return 0;
}
