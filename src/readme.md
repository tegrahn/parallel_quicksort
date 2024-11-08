# Run parallel quick sort
## Generate random numbers file using program in random_nums
1. Change directory to the random_nums directory
2. Compile the program `gcc -o rn main.c`
3. Run the program`./rn 10000 100`
4. Generates 10000 integers between 0 and 100 to a file named 10000

## Run sort
1. Ensure you are in the parent directory
1. run `make`
2. run `./sort 10000 random_nums/10000 4`. The first argument is the number of integers to read, the 2nd is the file to read, the third is the number of threads to use.
3. The program will output how long the inital sort, merge and total time. If the list is not sorted `"List is not sorted :("` will be printed. 

# Extra
- To adjust different sorting, pivot or merging strategies adjust the commented sections in sort_funcs.c
- For debugging ensure to use add the -g flag to the makefile, There is a commented CFLAGS for use.
- A stack implmentation is also provided in the sort_funs_stack.c file. This will overflow sorting large lists.
