//
// Created by itay on 19/09/15.
//

int printf(const char * str, ...);




void func()
{
    int A[200];
    int B[200];
    int C[200];

    // Multiple arrays, no dependencies. This is more complex as arrays access each other, just not the same cells
    // Expected output: can parallelize
    for (int i = 0; i < 100; ++i) {
        B[i] = A[i + 100];
        A[i + 100] = B[i];
    }

    printf("%d <-> %d\n", A[99], B[0]);
}