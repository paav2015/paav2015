//
// Created by itay on 19/09/15.
//

void func()
{
    int C[100];

    // Same array, step of 2
    // Expected output: can parallelize
    for (int i = 0; i < 100; i += 2) {
        C[i] = C[i - 1];
    }
}