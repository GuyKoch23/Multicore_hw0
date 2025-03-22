#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <omp.h>
#include <sys/time.h>

#define LIMIT_PARALLEL_GRANULARITY 1000

void swap(uint64_t *first, uint64_t *second) {
    uint64_t num = *first;
    *first = *second;
    *second = num;
}

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
    if (argc != 3) {
        fprintf(stderr, "Wrong Usage\n");
        return 1;
    }

    int parallelizem_amount = atoi(argv[1]);
    char *file_path = argv[2];
    FILE *file = fopen(file_path, "r");
    if (!file) {
        perror("Cannot open provided file");
        return 1;
    }

    size_t num_of_numbers = 100000;
    size_t current = 0;
    uint64_t *numbers_array = NULL;

    numbers_array = malloc(num_of_numbers * sizeof(uint64_t));
    if (!numbers_array) {
        perror("Memory allocation failed");
        return 1;
    }

    while (fscanf(file, "%lu", &numbers_array[current]) == 1) {
        current++;
        if (current >= num_of_numbers) {
            num_of_numbers *= 2;
            numbers_array = realloc(numbers_array, num_of_numbers * sizeof(uint64_t));
            if (!numbers_array) {
                perror("Memory reallocation failed");
                return 1;
            }
        }
    }
    fclose(file);

    struct timeval begin_time, finish_time;
    gettimeofday(&begin_time, NULL);

    omp_set_num_threads(parallelizem_amount);
    #pragma omp parallel
    {
        #pragma omp single
        quicksort(numbers_array, 0, current - 1);
    }

    gettimeofday(&finish_time, NULL);
    long total_time = (finish_time.tv_sec - begin_time.tv_sec) * 1000000L + (finish_time.tv_usec - begin_time.tv_usec);


    printf("QuickSort: %ld\n", total_time);
    for (size_t i = 0; i < count; i++) {
        printf("%lu\n", numbers[i]);
    }
    free(numbers_array);
    return 0;
}
