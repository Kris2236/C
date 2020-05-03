#include <stdio.h>

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

int main()
{
    int n = 0;
    int T2[1000];
    int T3[1000];
    int t2 = 0;
    int t3 = 0;

    scanf("%d", &n);
    for (int i = 0; i < n; ++i)
    {
        int temp = 0;
        scanf("%d", &temp);

        if (temp % 2 == 0)
            T2[t2++] = temp;
        else
            T3[t3++] = temp;
    }

    bubble_sort(T2, t2);
    bubble_sort(T3, t3);

    printf("%d %d\n", t2, t3);

    for (int i = 0; i < t2; ++i)
        printf("%d\n", T2[i]);

    for (int i = 0; i < t3; ++i)
        printf("%d\n", T3[i]);

    return 0;
}