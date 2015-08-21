
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

int main(int argc, char * argv [])
{
	f1(1,2);
	f2(1024);
	f3();

	return 0;
}
