//
// Created by itay on 19/09/15.
//

int printf(const char * str, ...);

void func()
{
    int A[100];
    int B[100];

    // Simple test: simple loop with multiple arrays, no dependencies
    // Expected output: can parallelize
    for (int i = 0; i < 100; ++i) {
        A[i] = B[99 - i];
    }

    printf("%d <-> %d\n", A[99], B[0]);
}