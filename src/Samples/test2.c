//
// Created by itay on 19/09/15.
//

int printf(const char * str, ...);

void func()
{
    int arr[100];

    // Simple test: simple loop with array access and no dependencies
    // Expected output: can parallelize
    for (int i = 0; i < 100; ++i) {
        arr[i] = i;
    }

    printf("%d\n", arr[99]);
}