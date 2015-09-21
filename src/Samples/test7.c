//
// Created by itay on 19/09/15.
//

int printf(const char * str, ...);


void func()
{
    int A[300];
    int B[100];

    // Multiple arrays, with dependencies
    // Expected output: cannot parallelize
    for (int i = 0; i < 100; ++i) {
        A[i] = B[i];
        B[i] = A[2 * i + 1];
    }

    printf("%d <-> %d\n", A[99], B[0]);
}