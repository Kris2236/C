#include <stdio.h>
#include <stdlib.h>

void swap(int* a, int* b)
{
    int c = *a;
    *a = *b;
    *b = c;
}

void bubble_sort(int tab[], int N)
{
    for (int i = N - 1; i > 0; --i)
    {
        int sorted = 1;

        for (int j = 0; j < i; ++j)
        {
            if (tab[j] > tab[j + 1])
            {
                swap(tab + j, tab + j + 1);
                sorted = 0;
            }
        }

        if (sorted)
            return;
    }
}

int* merge(int* T1, int m, int* T2, int n, int* T3)
{
    if (m == 0 && n == 0)
        return 0;

    int len = 0;
    int p1 = 0;
    int p2 = 0;

    // Assigning the first element
    if (m != 0 && n != 0)
    {
        if (T1[0] < T2[0])
            T3[len++] = T1[p1++];
        else
            T3[len++] = T2[p2++];
    }

    while (p1 < m && p2 < n)
    {
        if (T1[p1] < T2[p2])
        {
            if (T3[len - 1] != T1[p1])
                T3[len++] = T1[p1];
            p1++;
        }
        else
        {
            if (T3[len - 1] != T2[p2])
                T3[len++] = T2[p2];
            p2++;
        }
    }

    // Still something left at the end of T1
    while (p1 < m)
    {
        if (T3[len - 1] != T1[p1])
            T3[len++] = T1[p1];
        p1++;
    }


    // Still something left at the end of T2
    while (p2 < n)
    {
        if (T3[len - 1] != T2[p2])
            T3[len++] = T2[p2];
        p2++;
    }

    return len;
}

int main()
{
    int m = 0;
    int n = 0;

    scanf("%d %d", &m, &n);

    int* T1 = malloc(sizeof(int) * m);
    int* T2 = malloc(sizeof(int) * n);

    for (int i = 0; i < m; ++i)
        scanf("%d", T1 + i);

    for (int i = 0; i < n; ++i)
        scanf("%d", T2 + i);

    bubble_sort(T1, m);
    bubble_sort(T2, n);

    int* T3 = malloc(sizeof(int) * (m + n));
    int t3 = merge(T1, m, T2, n, T3);

    free(T1);
    free(T2);

    for (int i = 0; i < t3; ++i)
        printf("%d\n", T3[i]);

    free(T3);

    return 0;
}