#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>

void swap(int* a, int* b)
{
	int c = *a;
	*a = *b;
	*b = c;
}

int divide(int* values, int l, int r)
{
	// Choosing the last element as the anchor
	int anchor = r;

	int border = l;

	for (int i = l; i <= r; ++i)
	{
		// Skipping the anchor.
		if (i == anchor)
			continue;

		if (values[i] < values[anchor])
		{
			swap(values + i, values + border);
			border++;
		}
	}

	// Putting the anchor in the correct place
	swap(values + anchor, values + border);

	return border;
}

void quick_sort(int* values, int l, int r)
{
	if (l >= r)
		return;

	int border = divide(values, l, r);

	quick_sort(values, l, border - 1);
	quick_sort(values, border + 1, r);
}

/*
* IO Utility Functions
*/

int* read_set(int n)
{
	int* set = malloc(sizeof(int) * n);

	for (int i = 0; i < n; ++i)
	{
		scanf("%d", set + i);
	}

	return set;
}

void print_set(int* set, int n)
{
	for (int i = 0; i < n; ++i)
	{
		printf("%d\n", set[i]);
	}
}

int main()
{
	int Z = 0;
	scanf("%d", &Z);

	int** sets = malloc(sizeof(int*) * Z);

	// Reading sets and sorting them.
	for (int z = 0; z < Z; ++z)
	{
		int n = 0;
		scanf("%d", &n);

		sets[z] = read_set(n);

		// Sorting
		quick_sort(sets[z], 0, n - 1);

		// Printint sorted set
		print_set(sets[z], n);
	}

	// Deallocating memory
	for (int z = 0; z < Z; ++z)
	{
		free(sets[z]);
	}
	free(sets);
}
