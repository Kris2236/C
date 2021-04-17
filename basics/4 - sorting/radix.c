#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef char* String;

String* create_string_array(int n, int maxDigits)
{
	String* set = malloc(sizeof(String) * n);

	for (int i = 0; i < n; ++i)
	{
		// Reserving one extra space for the 0 character.
		set[i] = malloc(sizeof(char) * (maxDigits + 1));
	}

	return set;
}

void free_string_array(String* values, int n)
{
	for (int i = 0; i < n; ++i)
		free(values[i]);

	free(values);
}

void counting_sort(String* values, int size, int digits, int digit)
{
	int* temp = malloc(sizeof(int) * 10);
	String* new_mapping = create_string_array(size, digits);

	for (int i = 0; i < 10; ++i)
		temp[i] = 0;

	for (int i = 0; i < size; ++i)
	{
		temp[values[i][digit] - '0']++;
	}

	for (int i = 1; i < 10; ++i)
	{
		temp[i] += temp[i - 1];
	}

	for (int i = size - 1; i >= 0; --i)
	{
		int digitIndex = values[i][digit] - '0';
		temp[digitIndex] -= 1;
		strcpy(new_mapping[temp[digitIndex]], values[i]);
	}

	for (int i = 0; i < size; ++i)
	{
		strcpy(values[i], new_mapping[i]);
	}

	free(temp);
	free_string_array(new_mapping, size);
}

void radix_sort(String* values, int size, int digits)
{
	for (int i = digits - 1; i >= 0; --i)
		counting_sort(values, size, digits, i);
}

/*
* IO Utility Functions
*/

String* read_set(int n, int k)
{
	String* set = create_string_array(n, k);

	for (int i = 0; i < n; ++i)
		scanf("%s", set[i]);

	return set;
}

void print_set(String* set, int n)
{
	for (int i = 0; i < n; ++i)
		printf("%s\n", set[i]);
}

int main()
{
	int Z = 0;
	scanf("%d", &Z);

	String** sets = malloc(sizeof(String*) * Z);

	// Reading sets and sorting them.
	for (int z = 0; z < Z; ++z)
	{
		int n = 0;
		int k = 0;
		scanf("%d %d", &n, &k);

		sets[z] = read_set(n, k);

		// Sorting
		radix_sort(sets[z], n, k);

		// Printint sorted set
		print_set(sets[z], n);

		// Deallocating memory
		free_string_array(sets[z], n);
	}

	free(sets);
}
