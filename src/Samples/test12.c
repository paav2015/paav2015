//
// Created by itay on 19/09/15.
//

int printf(const char * str, ...);


void func()
{
    int A[1000] = { 0 };

    // Array access the same location
    // Expected output: can parallelize
    for (int i = 0; i < 1000; ++i)
    {
        A[i] = A[i] + 1;
    }

}