typedef int intType;

typedef struct {
  int start_idx;
  int split_size;
  int end_idx;
  int size;
} bin_info;

int compare (const void * num1, const void * num2);
void p_quick_sort(int* list, int N, int n_threads);
void global_sort(int* list, int size, int n_bins, bin_info* bin_infos);
int select_pivot(int* list, int n_bins, bin_info* bin_infos);
void merge_bin(int * list, int split, int sort_length);
void list_sort(int* list, int N);
void bubble_sort(int* list, int N);
int qs_partition(int* list, int low, int high);
void quick_sort(int* list, int low, int high);
void quick_i_sort(int* list, int low, int high);
void insertion_sort(int* list, int N);
void merge(int* list_to_sort, int splut, int N);
