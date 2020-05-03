#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>

void swap(int* a, int* b)
{
	int c = *a;
	*a = *b;
	*b = c;
}

int lchild(int index)
{
	return index * 2 + 1;
}

int rchild(int index)
{
	return index * 2 + 2;
}

void heapify_max(int* values, int size, int index)
{
	int max = index;
	int l = lchild(index);
	int r = rchild(index);

	if (l < size && values[l] > values[max])
		max = l;

	if (r < size && values[r] > values[max])
		max = r;

	if (max != index)
	{
		swap(values + index, values + max);
		heapify_max(values, size, max);
	}
}

void build_max_heap(int* values, int size)
{
	int last_parent = size / 2 - 1;

	for (int i = last_parent; i >= 0; --i)
	{
		heapify_max(values, size, i);
	}
}

void heap_sort(int* values, int size)
{
	build_max_heap(values, size);

	for (int i = size - 1; i > 0; --i)
	{
		swap(values, values + i);

		// Heapifying the top-most element.
		heapify_max(values, i, 0);
	}
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
		heap_sort(sets[z], n);

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
