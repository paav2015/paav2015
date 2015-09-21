//
// Created by itay on 19/09/15.
//

int printf(const char * str, ...);

void func()
{

    // Simple test: simple loop with no array access or private variables
    // Expected output: can parallelize
    for (int i = 0; i < 1000; ++i) {
        printf("%d\n", i);
    }
}