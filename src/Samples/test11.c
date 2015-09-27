//
// Created by itay on 19/09/15.
//

int printf(const char * str, ...);


void func()
{
    int A[10][10];
    int B[10][10];

    // Nested loop, with simple dependencies
    // Expected output: cannot parallelize
    for (int i = 0; i < 10; ++i)
    {
        for (int j = 0; j < 10; ++j)
        {
            B[i][j] = i * j;
        }
        for (int j = 0; j < 10; ++j)
        {
            A[i][j] = B[i][j];
        }
    }

}