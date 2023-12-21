/*１．OpenMPを用いて１つの配列にクイックソートと基数ソートハイブリットソートアルゴリズムを作成。　
２．クイックソートで大きいグループと小さいグループに分けられたとき、
小さいグループの最大値の桁数が5桁以下であれば基数ソートを行い、
大きいグループはソートが完了するまでクイックソートを繰り返す。*/
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

#define THRESHOLD 10
#define NUM_THREADS 8
#define MAX_ARRAY_SIZE 100000
#define MAX_RAND 100000

#define RADIX_THRESHOLD 5 //基数ソートを行う桁数

int radix_flag = 1; //基数ソートが行われると0になる

int compare_int(const void *a, const void *b) {
    return (*(int *)a - *(int *)b);
}

void countSort(int array[], int array_size, int exp) {

    int sorted_array[array_size];
    int count[10] = {0};

    for (int i = 0; i < array_size; i++) {
        count[(array[i] / exp) % 10]++;
    }

    for (int i = 1; i < 10; i++) {
        count[i] += count[i - 1];
    }

    for (int i = array_size - 1; i >= 0; i--) {
        sorted_array[count[(array[i] / exp) % 10 ] - 1] = array[i];
        count[(array[i] / exp) % 10]--;
    }

    for (int i = 0; i < array_size; i++) {
        array[i] = sorted_array[i];
    }
}

void radixSort(int *array, int array_size) {

    // find the maximum value in the array
    int max = array[0];
    for(int i = 1; i < array_size; i++) {
        if(array[i] > max) {
            max = array[i];
        }
    }

    // sorting based on 10^i, where i = 1, 2, ...
    for (int exp = 1; max / exp > 0; exp *= 10) {
        countSort(array, array_size, exp);
    }
}

// 配列の中の最大値を見つける関数
int findMax(int *arr, int size) {
    // 配列が空の場合
    if (size == 0) {
        printf("Error: Array is null.\n");
        return -1; // エラーを示す値を返すか、エラー処理を追加することが一般的です
    }

    int max = arr[0]; // 最大値を初期化

    // 配列を走査して最大値を見つける
    for (int i = 1; i < size; i++) {
        if (arr[i] > max) {
            max = arr[i];
        }
    }

    return max;
}

void quickSort_radix(int *arr, int l, int r) {
    // クイックソート
    int length = r - l;

    //radixSortが１度も処理されていない場合
    if(radix_flag == 1){
        //printf("radixSort check message\n");
        // 小さいグループの最大値の桁数が RADIX_THRESHOLD 以下なら基数ソートを行う
        int smallGroupSize_MAX = findMax(arr + l, length);
        if (smallGroupSize_MAX <= RADIX_THRESHOLD) {
            radixSort(arr + l, length);
        }
        radix_flag = 0;
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
    // 小さいグループに対して再帰的に quickSort_radix を呼ぶ
    #pragma omp task
    if (l < rightIndex)
        quickSort_radix(arr, l, rightIndex);

    // 大きいグループに対して再帰的に quickSort_radix を呼ぶ
    #pragma omp task
    if (leftIndex < r)
        quickSort_radix(arr, leftIndex, r);
}

void hybridSort(int *arr, int l, int r) {
    // ハイブリッドソート
    #pragma omp parallel
    #pragma omp single nowait
    #pragma omp num_threads(NUM_THREADS)
    {
        quickSort_radix(arr, l, r);
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
        k = rand() % MAX_RAND + 1;
        arr[i] = k;
        backup[i] = k;
    }

    // ハイブリッドソートの実行
    start_clock = omp_get_wtime();
    hybridSort(arr, 0, n - 1);
    end_clock = omp_get_wtime();

    // ソートされているか確認
    for (i = 0; i < n - 1; i++) {
        // printf("%d ",arr[i]);
        if (arr[i] > arr[i + 1]) {
            printf("Error: Array not sorted.\n");
            break;
        }
    }
    // printf("\n");

    printf("[hybridSort]: %14.6e\n", end_clock - start_clock);

    start_clock = omp_get_wtime();
    radixSort(backup, n);
    end_clock = omp_get_wtime();

    // ソートされているか確認
    for (i = 0; i < n - 1; i++) {
        // printf("%d ",backup[i]);
        if (backup[i] > backup[i + 1]) {
            printf("Error: Backup array not sorted.\n");
            // printf("Error: Backup array not sorted. : %d-%d & %d-%d\n",i,backup[i], i+1, backup[i + 1]);
            break;
        }
    }
    // printf("\n");


    printf("[radixSort]: %14.6e\n", end_clock - start_clock);

    return 0;
}
