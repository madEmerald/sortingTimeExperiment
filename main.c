#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <memory.h>

#define BITS_IN_BYTE 256

#define ARRAY_SIZE(a) sizeof(a) / sizeof(a[0])

#define TIME_TEST(testCode, time) { \
    clock_t start_time = clock (); \
    testCode \
    clock_t end_time = clock (); \
    clock_t sort_time = end_time - start_time; \
    time = (double) sort_time / CLOCKS_PER_SEC; \
}

typedef struct SortFunc {
    long long (*sort)(int *a, size_t n);

    char name[64];
} SortFunc;

typedef struct GeneratingFunc {
    void (*generate)(int *a, size_t n);

    char name[64];
} GeneratingFunc;

double getTime() {
    clock_t start_time = clock();
    clock_t end_time = clock();
    clock_t sort_time = end_time - start_time;
    return (double) sort_time / CLOCKS_PER_SEC;
}

void swap(void *v1, void *v2, size_t size) {
    char *t[size];
    memcpy(t, v1, size);
    memcpy(v1, v2, size);
    memcpy(v2, t, size);
}

void outputArray(int *a, const size_t n) {
    for (size_t i = 0; i < n; i++)
        printf("%d ", a[i]);
    printf("\n");
}

int isOrdered(const int *a, const size_t size) {
    for (int i = 1; i < size; i++)
        if (a[i] < a[i - 1])
            return 0;
    return 1;
}


// ________________Sorts________________

long long bubbleSort(int *a, const size_t size) {
    long long nComps = 0;

    for (size_t i = 0; i < size - 1; i++)
        for (size_t j = size - 1; j > i; j--)
            if (++nComps && a[j - 1] > a[j])
                swap(&a[j - 1], &a[j], sizeof(int));
    return nComps;
}

long long selectionSort(int *a, const size_t size) {
    long long nComps = 0;

    for (size_t i = 0; i < size - 1; i++) {
        size_t minPos = i;
        for (size_t j = i + 1; j < size; j++)
            if (++nComps && a[j] < a[minPos])
                minPos = j;

        swap(&a[i], &a[minPos], sizeof(int));
    }
    return nComps;
}

long long insertionSort(int *a, const size_t size) {
    long long nComps = 0;

    for (size_t i = 1; i < size; i++) {
        int t = a[i];
        size_t j = i;
        while (++nComps && j > 0 && a[j - 1] > t) {
            a[j] = a[j - 1];
            j--;
        }
        a[j] = t;
    }
    return nComps;
}

long long compSort(int *a, const size_t size) {
    long long nComps = 0;

    size_t step = size;
    int swapped = 1;
    while (step > 1 || swapped) {
        if (step > 1)
            step /= 1.24733;

        swapped = 0;
        for (size_t i = 0, j = i + step; j < size; i++, j++)
            if (++nComps && a[i] > a[j]) {
                swap(&a[i], &a[j], sizeof(int));
                swapped = 1;
            }
    }
    return nComps;
}

long long shellSort(int *a, const size_t size) {
    long long nComps = 0;

    for (size_t step = size / 2; step > 0; step /= 2)
        for (size_t i = 0; i < size; i++) {
            size_t j = i;
            while (++nComps && j >= step && a[j - step] > a[j]) {
                swap(&a[j], &a[j - step], sizeof(int));
                j -= step;
            }
        }
    return nComps;
}

void countSort(int *a, const unsigned char *keys, const size_t size) {
    int cnt[BITS_IN_BYTE];
    for (size_t i = 0; i < BITS_IN_BYTE; i++)
        cnt[i] = 0;

    for (size_t i = 0; i < size; i++)
        cnt[keys[i]]++;

    int p[BITS_IN_BYTE];
    p[0] = 0;
    for (size_t i = 1; i < BITS_IN_BYTE; i++) {
        p[i] = p[i - 1] + cnt[i - 1];
    }

    int b[size];
    for (size_t i = 0; i < size; i++)
        b[p[keys[i]]++] = a[i];

    for (size_t i = 0; i < size; i++)
        a[i] = b[i];
}

unsigned char getByte(const int x, const size_t byteNumber) {
    return (x >> (byteNumber * 8)) & 0b11111111;
}

void digitSort(int *a, const size_t size) {
    unsigned char byteValues[size];
    for (size_t byteNumber = 0; byteNumber < sizeof(int); byteNumber++) {
        for (int j = 0; j < size; j++)
            byteValues[j] = getByte(a[j], byteNumber);

        countSort(a, byteValues, size);
    }
}


// __________Generators__________

void generateRandomArray(int *a, size_t n) {
    srand(time(0));
    for (size_t i = 0; i < n; i++)
        a[i] = rand();
}

void generateOrderedArray(int *a, size_t n) {
    for (size_t i = 0; i < n; i++)
        a[i] = i;
}

void generateOrderedBackwards(int *a, size_t n) {
    for (size_t i = 0; i < n; i++)
        a[i] = n - i;
}


// __________________________________________________

void checkTimeAndComparations(long long (*sortFunc)(int *, size_t),
                              void (*generateFunc)(int *, size_t),
                              size_t size, char *experimentName) {
    static size_t runCounter = 1;

    static int innerBuffer[100000];
    generateFunc(innerBuffer, size);
    printf("Run #%zu| ", runCounter++);
    printf("Name: %s\n", experimentName);

    // ?????????? ??????????????
    double time;
    long long nComps;
    TIME_TEST({nComps = sortFunc(innerBuffer, size);}, time);
    printf("Status:");
    if (isOrdered(innerBuffer, size)) {
        printf("OK! Time: %.3fs. | Comparations: %lld\n", time, nComps);

        char filename[256];
        sprintf(filename, "./data/%s.csv", experimentName);
        FILE *f = fopen(filename, "a");
        if (f == NULL) {
            printf("FileOpenError %s", filename);
            exit(1);
        }
        fprintf(f, "%zu; %.3f; %lld\n", size, time, nComps);
        fclose(f);
    } else {
        printf("Wrong!\n");

        // ?????????? ??????????????, ?????????????? ???? ???????? ???????? ????????????????????????
        outputArray(innerBuffer, size);

        exit(1);
    }
}

void timeExperiment() {
    // ???????????????? ?????????????? ????????????????????
    SortFunc sorts[] = {
            {bubbleSort,    "bubbleSort"},
            {selectionSort, "selectionSort"},
            {insertionSort, "insertionSort"},
            {compSort,      "compSort"},
            {shellSort,     "shellSort"}
    };
    const unsigned FUNCS_N = ARRAY_SIZE(sorts);

    // ???????????????? ?????????????? ??????????????????
    GeneratingFunc generatingFuncs[] = {
            // ???????????????????????? ?????????????????? ????????????
            {generateRandomArray,      "random"},
            // ???????????????????????? ???????????? 0, 1, 2, ..., n - 1
            {generateOrderedArray,     "ordered"},
            // ???????????????????????? ???????????? n - 1, n - 2, ..., 0
            {generateOrderedBackwards, "orderedBackwards"}
    };
    const unsigned CASES_N = ARRAY_SIZE(generatingFuncs);

    // ???????????? ???????????????????? ?? ????????
    for (size_t size = 10000; size <= 100000; size += 10000) {
        printf("------------------------------\n");
        printf("Size: %zu\n", size);
        for (int i = 0; i < FUNCS_N; i++) {
            for (int j = 0; j < CASES_N; j++) {
                // ?????????????????? ?????????? ??????????
                static char filename[128];
                sprintf(filename, "%s_%s_timeAndComps",
                        sorts[i].name, generatingFuncs[j].name);
                checkTimeAndComparations(sorts[i].sort, generatingFuncs[j].generate, size, filename);
            }
        }
        printf("\n");
    }
}

int main() {
    timeExperiment();
    //int a[10] = {10, 4, 5, 6, 3, 5, 8, 4, 2, 6};
    //digitSort(a, 10);
    //outputArray(a, 10);

    return 0;
}
