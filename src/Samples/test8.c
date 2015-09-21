//
// Created by itay on 19/09/15.
//

int printf(const char * str, ...);


void func()
{
    int A[10][10];

    // Nested loop, no dependencies
    // Expected output: can parallelize only external loop
    for (int i = 0; i < 10; ++i)
    {
        for (int j = 0; j < 10; ++j)
        {
            A[i][j] = i * j;
        }
    }

}