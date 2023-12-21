#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

/*クイックソートで
中央値をもとに大きいグループと小さいグループに分けられたとき
そのグループの配列の要素数が...
10個以下の配列であれば次に行う処理はバブルソートを行い、
10個以上であればクイックソートを行う処理を繰り返す。*/

#define THRESHOLD 10
#define NUM_THREADS 8
#define MAX_ARRAY_SIZE 100000

int compare_int(const void *a, const void *b) {
    return (*(int *)a - *(int *)b);
}

void bubbleSort(int *arr, int n) {
    // バブルソートの実装
    for (int i = 0; i < n - 1; i++)
        for (int j = 0; j < n - i - 1; j++)
            if (arr[j] > arr[j + 1]) {
                int temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
}

void quickSort_bubble(int *arr, int l, int r) {
    // クイックソート
    int length = r - l;
    
    if (length < THRESHOLD) {
        // サイズが THRESHOLD 以下ならバブルソート
        bubbleSort(arr + l, length + 1);
        return;
    }

    int leftIndex = l;
    int rightIndex = r;
    int pivot = arr[leftIndex + (rightIndex - leftIndex) / 2];

    while (leftIndex <= rightIndex) {
        while (arr[leftIndex] < pivot)
            leftIndex++;
        while (arr[rightIndex] > pivot)
            rightIndex--;
        if (leftIndex <= rightIndex) {
            int temp = arr[leftIndex];
            arr[leftIndex] = arr[rightIndex];
            arr[rightIndex] = temp;
            leftIndex++;
            rightIndex--;
        }
    }

    // 基準値（中央値）からグループ分けをし、
    // 小さいグループに対して再帰的に quickSort_bubble を呼ぶ
    #pragma omp task
    if (l < rightIndex)
        quickSort_bubble(arr, l, rightIndex);

    // 大きいグループに対して再帰的に quickSort_bubble を呼ぶ
    #pragma omp task
    if (leftIndex < r)
        quickSort_bubble(arr, leftIndex, r);
}

void hybridSort(int *arr, int l, int r) {
    // ハイブリッドソート
    #pragma omp parallel
    #pragma omp single nowait
    #pragma omp num_threads(NUM_THREADS)
    {
        quickSort_bubble(arr, l, r);
    }
}

int main() {
    srand((unsigned int)time(NULL));

    int i, k, n = MAX_ARRAY_SIZE;
    int arr[n];
    int backup[n];
    double start_clock, end_clock;

    // ランダムなデータで配列を初期化
    for (i = 0; i < n; i++) {
        k = rand() % 100 + 1;
        arr[i] = k;
        backup[i] = k;
    }

    // ハイブリッドソートの実行
    start_clock = omp_get_wtime();
    hybridSort(arr, 0, n - 1);
    end_clock = omp_get_wtime();

    // ソートされているか確認
    for (i = 0; i < n - 1; i++) {
        if (arr[i] > arr[i + 1]) {
            printf("Error: Array not sorted.\n");
            break;
        }
    }

    printf("[hybridSort]: %14.6e\n", end_clock - start_clock);

    // バブルソートでのソート
    start_clock = omp_get_wtime();
    bubbleSort(backup, n);
    end_clock = omp_get_wtime();

    // ソートされているか確認
    for (i = 0; i < n - 1; i++) {
        if (backup[i] > backup[i + 1]) {
            printf("Error: Backup array not sorted.\n");
            break;
        }
    }

    printf("[bubbleSort]: %14.6e\n", end_clock - start_clock);

    return 0;
}
