
// Forward declarations, to avoid additional includes
int scanf(const char * format, ...);
int printf(const char * format, ...);
unsigned long strlen(const char * str);

int f1(int a, int b)
{
	return a + b;
}

int f2(int i)
{
	unsigned long j;
	int result;
	for (j = 0; j < i; ++j)
	{
		result += j;
	}

	return result;
}

void f3()
{
	int arr[100];
	unsigned long j;

	for (j = 0; j < 99; ++j)
	{
		arr[j] = (arr[j] + arr[j + 1]) / 2;
	}
}

void f4(int * arr, unsigned long a)
{
	unsigned long i;
	for (i = 0; i < a; i++) {
		//res = res + i;
		arr[i-2] = arr[i];
	}
}

void f5()
{
	unsigned int i = 0;
	int x = 5;
	if (x < 10) {
		x -= 5;
	}

	do {
		++i;
	} while(i % 10 != 0);
}

void f6()
{
    char your_name[100];
    unsigned int i, j;
    unsigned int name_len;

    scanf("%s", your_name);
    printf("Hello, %s\n", your_name);

    name_len = strlen(your_name);

    for (i = 0; i < 1000; ++i)
    {
        unsigned int to_print_pos;
        char to_print_char;

        to_print_pos = i % name_len;
        to_print_char = your_name[i];
        printf("%c\n", to_print_char);
    }

    for (j = 0; j < 1000; ++j)
    {
        unsigned int to_print_pos;
        char to_print_char;

        to_print_pos = name_len - (i % name_len);
        to_print_char = your_name[i];
        printf("%c\n", to_print_char);
    }
}

void f7(int b)
{
    int numbers[100];
    int bumbers[100];
    unsigned long j;

    for (j = 0; j < 25; ++j)
    {
        numbers[j] = b;
        numbers[j * 2 + 1] = bumbers[j * 4];
        bumbers[j * 4] = j;
    }
}

void f8()
{
    int arr[1000];
    for (int i = 0; i < 99; ++i)
    {
        arr[i] = i;
        arr[i * 2] = arr[i];
        arr[i * 10 + 5] = i;
    }
}

int main(int argc, char * argv [])
{
	f1(1,2);
	f2(1024);
	f3();

	return 0;
}
