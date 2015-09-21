//
// Created by itay on 19/09/15.
//

int printf(const char * str, ...);


void func()
{
    int A[100];

    // One array, with dependency
    // Expected output: cannot parallelize
    for (int i = 0; i < 99; ++i) {
        A[i] = A[i + 1];
    }
}