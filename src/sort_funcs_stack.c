#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/time.h>
#include <math.h>
#include <omp.h>
#include "sort_funcs.h"

static double get_wall_seconds() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  double seconds = tv.tv_sec + (double)tv.tv_usec / 1000000;
  return seconds;
}

int compare (const void * num1, const void * num2) {
   if(*(int*)num1 > *(int*)num2)
    return 1;
   else
    return -1;
}

void p_quick_sort(int* list, int N, int n_bins) {
  double inital_sort_start = get_wall_seconds();
  //Initial sort and construct initial bin info
  bin_info* bin_infos = (bin_info*)malloc(n_bins * sizeof(bin_info));
  int step_size = N/n_bins;
  // Construct n-1 bins
  for (int i=0; i < n_bins-1; i++){
    bin_infos[i].start_idx = i*step_size;;
    bin_infos[i].size = step_size;
    bin_infos[i].end_idx = bin_infos[i].start_idx + step_size - 1;
  }
  // Construct the final bin as the remainder
  int final_idx = n_bins-1;
  bin_infos[final_idx].start_idx = final_idx*step_size;
  step_size = N - bin_infos[final_idx].start_idx;
  bin_infos[final_idx].size = step_size;
  bin_infos[final_idx].end_idx = bin_infos[final_idx].start_idx + step_size - 1;
  //Initial sort
  #pragma omp parallel for
  for (int i=0; i < n_bins; i++){
    int start_index = bin_infos[i].start_idx;
    int sort_length = bin_infos[i].size;
    list_sort(&list[start_index],sort_length);
  }
  printf("f %f\n",get_wall_seconds()-inital_sort_start);
  //Begin global sort of sorted bins
  global_sort(list, N, n_bins, bin_infos);
  printf("t %f\n",get_wall_seconds()-inital_sort_start);
}

void global_sort(int* list, int size, int n_bins, bin_info* bin_infos){
  if (n_bins == 1 || n_bins == 0){
    free(bin_infos);
    return;
  }

  // Fuse if odd bins (This can occur due to an odd thread selection)
  if (n_bins % 2 == 1) {
    //Fuse the final bin into the second last bin
    int old_bin_size = bin_infos[n_bins-2].size;
    int final_bin_size = bin_infos[n_bins-1].size;
    bin_infos[n_bins-2].size += final_bin_size;
    bin_infos[n_bins-2].end_idx += final_bin_size;
    //Merge to maintain sorted behaviour
    int new_bin_start = bin_infos[n_bins-2].start_idx;
    int new_bin_size = bin_infos[n_bins-2].size;
    merge_bin(&list[new_bin_start], old_bin_size, new_bin_size);
    n_bins --;
  }
  
  //List offset is the start of first bin
  int list_offset = bin_infos[0].start_idx;
  // Select pivot
  double pivot_selection_start = get_wall_seconds();
  int pivot = select_pivot(list, n_bins, bin_infos);
  printf("%d p %f\n",n_bins,get_wall_seconds()-pivot_selection_start);
  

  int current_offset = list_offset;
  // Setup lower bin variables
  bin_info* lower_sort_infos = (bin_info*)malloc(n_bins/2 * sizeof(bin_info));
  int lower_list[size];
  int num_lower_bin = 0;
  int current_lower_bin_size = 0;
  int total_lower_bin_size = 0;
  
  // Setup upper bin variables
  bin_info* upper_sort_infos = (bin_info*)malloc(n_bins/2 * sizeof(bin_info));
  int upper_list[size];
  int num_upper_bin = 0;
  int current_upper_bin_size = 0;
  int total_upper_bin_size = 0;

  //Construct new bins for sorting lower and upper halves of existings bins
  for (int i=0; i < n_bins; i++)
  {
    // If nothing is found then all elements are smaller than the pivot
    int size_lower = 0;
    for (int j=0; j < bin_infos[i].size; j++){
      //Count all elements less than the first greater element
      if (list[bin_infos[i].start_idx+j] > pivot) {
        break;
      }
      size_lower += 1;
    }
    int size_upper = bin_infos[i].size - size_lower;

    // Prepare new lists and construct information for sorting upper and lower lists
    // Copy into the lower list
    memcpy(lower_list + total_lower_bin_size, list + current_offset, size_lower * sizeof(int));
    current_offset += size_lower;
    // Construct lower list information
    current_lower_bin_size += size_lower;
    total_lower_bin_size += size_lower;
    // Every 2 bins are grouped for sorting
    if (i % 2 == 1){
      lower_sort_infos[num_lower_bin].end_idx = total_lower_bin_size - 1;
      lower_sort_infos[num_lower_bin].size = current_lower_bin_size;
      lower_sort_infos[num_lower_bin].start_idx = total_lower_bin_size - current_lower_bin_size;
      num_lower_bin++;
      current_lower_bin_size = 0; 
    }
    else {
      lower_sort_infos[num_lower_bin].split_size = current_lower_bin_size;
    }
    //Copy into the upper list
    memcpy(upper_list + total_upper_bin_size, list + current_offset, size_upper * sizeof(int));
    current_offset+=size_upper;
    current_upper_bin_size += size_upper;
    total_upper_bin_size += size_upper;
    //Every 2 bins are grouped for sorting
    if (i % 2 == 1){
      upper_sort_infos[num_upper_bin].end_idx = total_upper_bin_size - 1;
      upper_sort_infos[num_upper_bin].size = current_upper_bin_size;
      upper_sort_infos[num_upper_bin].start_idx = total_upper_bin_size - current_upper_bin_size;
      num_upper_bin++;
      current_upper_bin_size = 0; 
    }
    else {
      upper_sort_infos[num_upper_bin].split_size = current_upper_bin_size;
    }

  }
  // Done with the old bin_infos
  free(bin_infos);

  #pragma omp parallel
  #pragma omp single nowait
  {
    //Sort lower lists
    double l_r_start = get_wall_seconds();
    #pragma omp task
    {
      //Merge lower bins in parallel
      #pragma omp parallel for
      for (int i=0; i < num_lower_bin; i++){
        int start_index = lower_sort_infos[i].start_idx;;
        int sort_length = lower_sort_infos[i].size;
        int split = lower_sort_infos[i].split_size;
        merge_bin(&lower_list[start_index], split, sort_length);
      }
      //Update the original list with the new lower list and recurse on lower half
      memcpy(list + list_offset ,lower_list, total_lower_bin_size * sizeof(int));

      //Update the sort_infos to point to indexes in the original list
      for (int i=0; i < num_lower_bin; i++){
        lower_sort_infos[i].start_idx += list_offset;
        lower_sort_infos[i].end_idx += list_offset;
      }
      //As each half is independendant it can immeditely move on
      printf("%d l %f\n",n_bins ,get_wall_seconds()-l_r_start);
      global_sort(list, total_lower_bin_size, num_lower_bin, lower_sort_infos);
    }

    //Sort upper lists
    #pragma omp task 
    {
      //Merge upper bins in parallel
      #pragma omp parallel for
      for (int i=0; i < num_upper_bin; i++){
        int start_index = upper_sort_infos[i].start_idx;
        int sort_length = upper_sort_infos[i].size;
        int split = upper_sort_infos[i].split_size;
        merge_bin(&upper_list[start_index],split,sort_length);
      }
      //Update the original list
      memcpy(list + list_offset + total_lower_bin_size, upper_list, total_upper_bin_size * sizeof(int));
      //Update the sort_infos to point to indexes in the original list
      for (int i=0; i < num_lower_bin; i++){
        upper_sort_infos[i].start_idx += list_offset + total_lower_bin_size;
        upper_sort_infos[i].end_idx += list_offset + total_lower_bin_size;
      }
      printf("%d r %f\n",n_bins, get_wall_seconds()-l_r_start);
      //As each half is independendant it can immeditely move on
      global_sort(list, total_upper_bin_size, num_upper_bin, upper_sort_infos);
    }
  }
}

int select_pivot(int* list, int n_bins, bin_info* bin_infos){
  //Strategy 1: Median of first bin
  /*int pivot_idx = bin_infos[0].size/2;
  int pivot = list[bin_infos[0].start_idx+pivot_idx];
  return pivot;*/
  
  //Strategy 2: Mean of all medians
  /*int sum_medians = 0;
  for (int i=0; i<n_bins; i++){
      int pivot_idx = bin_infos[i].size/2;
      sum_medians += list[bin_infos[i].start_idx+pivot_idx];
  }
  int pivot = sum_medians/n_bins;
  return pivot;*/
  
  //Strategy 3: Sort medians and take the mean of the two middlemost
  int medians[n_bins];
  for (int i=0; i<n_bins; i++){
      int pivot_idx = bin_infos[i].size/2;
      medians[i] = list[bin_infos[i].start_idx+pivot_idx];
  }
  insertion_sort(medians, n_bins);
  int pivot = (medians[n_bins/2] + medians[n_bins/2 - 1])/2;
  return pivot;
}

void merge_bin(int* list, int split, int sort_length){
  // Strategy 1 Sort
  //list_sort(list, sort_length);
  // Strategy 2 Merge sorts merge
  merge(list, split, sort_length);
}

void list_sort(int* list, int N){
  // Strategy 1 Bubble sort
  //bubble_sort(list, N);
  // Strategy 2 Quicksort
  //quick_sort(list, 0, N-1);
  // Strategy 3 Quicksort + Insertion
  quick_i_sort(list, 0, N-1);
  // Strategy 4 Qsort
  //qsort(list, N, sizeof(int), compare);
}

void bubble_sort(int* list, int N) {
  int i, j;
  for(i = 0; i < N-1; i++)
    for(j = 1+i; j < N; j++) {
      if(list[i] > list[j]) {
        // Swap
        int tmp = list[i];
        list[i] = list[j];
        list[j] = tmp;
      }
    }
}
/*
https://en.wikipedia.org/wiki/Quicksort
https://stackoverflow.com/questions/7198121/quicksort-and-hoare-partition
*/
void quick_sort(int* list, int low, int high){
  if (low < high){
    int partition_idx = qs_partition(list, low, high);
    quick_sort(list, low, partition_idx);
    quick_sort(list, partition_idx+1, high) ;
  }
}

int qs_partition(int* list, int low, int high){
  int pivot = list[low];
  int i = low-1;
  int j = high+1; 
  int temp;
  while (1){
    //Find element from left bigger
    do  j--; while (list[j] > pivot);
    //Find element from right smaller
    do  i++; while (list[i] < pivot);
    // Swap if found indexes without crossing
    if (i < j) { 
      //Swap indexes
      temp = list[i];
      list[i] = list[j];
      list[j] = temp;
    } 
    // Continue until indexes cross as using the Hoare partition scheme
    else {
      return j;
    }
  }
}

void quick_i_sort(int* list, int low, int high){
  if (high - low < 50){
    insertion_sort(list, high+1);
  }
  if (low < high){
    int partition_idx = qs_partition(list, low, high);
    quick_sort(list, low, partition_idx);
    quick_sort(list, partition_idx+1, high) ;
  }
}

//https://www.geeksforgeeks.org/insertion-sort-algorithm/
void insertion_sort(int* arr, int N)
{
    int i, key, j;
    for (i = 1; i < N; i++) {
        key = arr[i];
        j = i - 1;
        /* Move elements of arr[0..i-1], that are
          greater than key, to one position ahead
          of their current position */
        while (j >= 0 && arr[j] > key) {
            arr[j + 1] = arr[j];
            j = j - 1;
        }
        arr[j + 1] = key;
    }
}

void merge(int* list_to_sort, int split, int N) {
  int list[N];
  int* list1 = &list[0];
  int* list2 = &list[split];
  int n1 = split;
  int n2 = N-split;
  memcpy(list1, list_to_sort, n1 * sizeof(int));
  memcpy(list2, list_to_sort + n1, n2 * sizeof(int));
  int i = 0;
  int i1 = 0;
  int i2 = 0;
  while(i1 < n1 && i2 < n2) {
    if(list1[i1] < list2[i2]) {
      list_to_sort[i] = list1[i1];
      i1++;
    }
    else {
      list_to_sort[i] = list2[i2];
      i2++;
    }
    i++;
  }
  while(i1 < n1)
    list_to_sort[i++] = list1[i1++];
  while(i2 < n2)
    list_to_sort[i++] = list2[i2++];
}

