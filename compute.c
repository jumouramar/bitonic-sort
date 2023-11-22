#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <omp.h>

#define MAX(A, B) (((A) > (B)) ? (A) : (B))
#define MIN(A, B) (((A) > (B)) ? (B) : (A))
#define UP 0
#define DOWN 1
#define SIZE 1048576

void bitonic_sort_seq(int start, int length, int *seq, int flag);
void bitonic_sort_par(int start, int length, int *seq, int flag);
void swap(int *a, int *b);

int sub_part;

int main(void) {
    int i, j;
    int n = SIZE;
    int flag;
    int seq[SIZE];

    double startTime, totalTime, endTime; 

    int numThreads, id;

    for (int i = 0; i < n; i++) {
        seq[i] = rand() % 100;
    }

    startTime = omp_get_wtime();

    numThreads =  omp_get_num_threads();

    sub_part = n / numThreads;
    

    for (i = 2; i <= sub_part; i = 2 * i) { // itera sobre potencias de 2
        #pragma omp parallel for shared(i, seq) private(j, flag) // indica q o loop seguinte será executado em paralelo
        for (j = 0; j < n; j += i) {
            if ((j / i) % 2 == 0)
                flag = UP;
            else
                flag = DOWN;
            bitonic_sort_seq(j, i, seq, flag);
        }
    }

    for (i = 2; i <= numThreads; i = 2 * i) {
        for (j = 0; j < numThreads; j += i) {
            if ((j / i) % 2 == 0)
                flag = UP;
            else
                flag = DOWN;
            bitonic_sort_par(j*sub_part, i*sub_part, seq, flag);
        }
        #pragma omp parallel for shared(j)
        for (j = 0; j < numThreads; j++) {
            if (j < i)
                flag = UP;
            else
                flag = DOWN;
            bitonic_sort_seq(j*sub_part, sub_part, seq, flag);
        }
    }

    endTime = omp_get_wtime();
    totalTime = endTime - startTime;

    printf("Tempo total = %.5f sec.\n", totalTime);
}

void bitonic_sort_seq(int start, int length, int *seq, int flag) {
    int i;
    int split_length;

    if (length == 1)
        return;

    if (length % 2 != 0) {
        printf("error\n");
        exit(0);
    }

    split_length = length / 2;

    // bitonic split
    for (i = start; i < start + split_length; i++) {
        if (flag == UP) {
            if (seq[i] > seq[i + split_length])
                swap(&seq[i], &seq[i + split_length]);
        }
        else {
            if (seq[i] < seq[i + split_length])
                swap(&seq[i], &seq[i + split_length]);
        }
    }

    bitonic_sort_seq(start, split_length, seq, flag);
    bitonic_sort_seq(start + split_length, split_length, seq, flag);
}

void bitonic_sort_par(int start, int length, int *seq, int flag) {
    int i;
    int split_length;

    if (length == 1)
        return;

    if (length % 2 != 0) {
        printf("The length of a (sub)sequence is not divided by 2.\n");
        exit(0);
    }

    split_length = length / 2;

    // bitonic split
    #pragma omp parallel for shared(seq, flag, start, split_length) private(i)
    for (i = start; i < start + split_length; i++) {
        int local_flag = flag;  // Make 'flag' private


        if (local_flag == UP) {
            if (seq[i] > seq[i + split_length])
                swap(&seq[i], &seq[i + split_length]);
        }
        else {
            if (seq[i] < seq[i + split_length])
                swap(&seq[i], &seq[i + split_length]);
        }
    }

    if (split_length > sub_part) {
        bitonic_sort_par(start, split_length, seq, flag);
        bitonic_sort_par(start + split_length, split_length, seq, flag);
    }

    return;
}

void swap(int *a, int *b) {
    int t;
    t = *a;
    *a = *b;
    *b = t;
}