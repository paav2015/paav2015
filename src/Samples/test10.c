//
// Created by itay on 19/09/15.
//

int printf(const char * str, ...);




void func()
{
    int A[100];

    // Modifying the same private variable
    // Expected output: cannot parallelize
    int sum = 0;
    for (int i = 0; i < 100; ++i) {
        A[i] = i;
        sum += A[i];
    }

    printf("Sum: %d\n", sum);
}