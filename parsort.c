#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <omp.h>
#include <sys/time.h>

#define LIMIT_PARALLEL_GRANULARITY 1000 // When going under size 1000 of subarray, i saw it makes sense to make is sequential running for better performance

//
// Helper swap method for quicksort algotithm 
//
void swap(uint64_t *first, uint64_t *second) {
    uint64_t num = *first;
    *first = *second;
    *second = num;
}

//
// Helper partition method for quicksort which part the array by a pivot element
//
int partition(uint64_t array[], int buttom_index, int top_index) {
    int middle_index = buttom_index + (top_index - buttom_index) / 2;
    swap(&array[middle_index], &array[top_index]);
    uint64_t pivot_item = array[top_index];
    int i = (buttom_index - 1);
    for (int j = buttom_index; j < top_index; j++) {
        if (array[j] <= pivot_item) {
            i++;
            swap(&array[i], &array[j]);
        }
    }
    swap(&array[i + 1], &array[top_index]);
    return (i + 1);
}

//
// The quicksort algorithm
// we can see the use of #pragma omp task to initialize a new task which takes advantage of multicors
// We can also notice that we check if the size of the current subarray is too small, and if so no parallalizem is implemented
// I found out in the experiments and researched online and saw that when the array is too small the overhead of context-switch
//  might be too expensive, and it truly improved performance when splitted to cases
//
void quicksort(uint64_t array[], int buttom_index, int top_index) {
    if (buttom_index < top_index) {
        int partition_index = partition(array, buttom_index, top_index);
        
        if ((top_index - buttom_index) > LIMIT_PARALLEL_GRANULARITY) {
            #pragma omp task
            quicksort(array, buttom_index, partition_index - 1);
            
            #pragma omp task
            quicksort(array, partition_index + 1, top_index);
        } 
        else {
            quicksort(array, buttom_index, partition_index - 1);
            quicksort(array, partition_index + 1, top_index);
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) { // Expecting both #cores, filepath
        fprintf(stderr, "Wrong usage with params\n");
        return 1;
    }

    int parallelizem_amount = atoi(argv[1]);
    char *file_path = argv[2];
    FILE *file = fopen(file_path, "r");
    if (!file) {
        perror("Problem with opening given file");
        return 1;
    }

    size_t num_of_numbers = 100000; // Starting with some amount we can allocate, will be reallocated if needed
    size_t current = 0; // Counter for iterating
    uint64_t *numbers_array = NULL; // The memory array for elements to read

    numbers_array = malloc(num_of_numbers * sizeof(uint64_t));
    if (!numbers_array) {
        perror("Problem when allocating array for numbers");
        return 1;
    }

    while (fscanf(file, "%lu", &numbers_array[current]) == 1) {
        current++;
        if (current >= num_of_numbers) { // We have more element then places we have in our array, lets reallocate
            num_of_numbers *= 2;
            numbers_array = realloc(numbers_array, num_of_numbers * sizeof(uint64_t));
            if (!numbers_array) {
                perror("Problem when allocating array for numbers");
                return 1;
            }
        }
    }
    fclose(file);

    struct timeval begin_time, finish_time;
    gettimeofday(&begin_time, NULL); // Start time

    omp_set_num_threads(parallelizem_amount);
    #pragma omp parallel // Using the multicore advantage to parallel execution
    {
        #pragma omp single
        quicksort(numbers_array, 0, current - 1);
    }

    gettimeofday(&finish_time, NULL);
    long total_time = (finish_time.tv_sec - begin_time.tv_sec) * 1000000L + (finish_time.tv_usec - begin_time.tv_usec);


    printf("QuickSort: %ld\n", total_time);
    for (size_t i = 0; i < current; i++) {
        printf("%lu\n", numbers_array[i]);
    }
    free(numbers_array);
    return 0;
}
