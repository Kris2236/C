/* Dana jest tablica liczb całkowitych, T1, o długości n. Napisz program, który na wejściu
otrzymuje wartości T1 i dzieli ją na dwie tablice, T2 i T3, tak, by w jednej z nich znalazły
się elementy parzyste a w drugiej nieparzyste. Następnie posortuj obie tablice. */

#include <stdio.h>

void swap(int* a, int* b)
{
    int c = *a;
    *a = *b;
    *b = c;
}

void bubble_sort(int* tab, int N)
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
    int T1[1000];
    int t2 = 0;
    int t3 = 0;

    scanf("%d", &n);
    for (int i = 0; i < n; ++i)
    {
        int input = 0;
        scanf("%d", &input);

        T1[i] = input;

        if (input % 2 == 0)
            t2++;
        else
            t3++;
    }

    int* T2 = malloc(sizeof(int) * t2);
    int* T3 = malloc(sizeof(int) * t3);
    t2 = 0;
    t3 = 0;

    for(int i=0; i<n; i++)
    {

        if (T1[i] % 2 == 0)
        {
            T2[t2] = T1[i];
            t2++;
        }
        else
        {
            T3[t3] = T1[i];
            t3++;
        }
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
