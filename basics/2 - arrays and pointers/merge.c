/* Dane s¹ dwie tablice liczb ca³kowitych, T1 i T2, o d³ugoœciach odpowiednio m i n. Napisz
program, który na wejœciu otrzymuje wartoœci obu tablic i sortuje je rosn¹co. Nastêpnie
utwórz now¹ tablicê, T3, o d³ugoœci co najwy¿ej m + n, która powstaje przez po³¹czenie tablic T1 i T2 w taki sposób, aby:
1. Zachowaæ rosn¹ce uporz¹dkowanie wartoœci tablicy
2. Wartoœci tablicy T3 by³y unikalne    */

#include <stdio.h>
#include <stdlib.h>

void unique(int* T1, int* T2, int* T3, int n, int m)
{

    int* tmp = malloc(sizeof(int) * (n+m));
    for(int i=0; i< n; i++)
        tmp[i] = T1[i];
    for(int i=n; i< (n+m); i++)
        tmp[i] = T2[i-n];

    bubble_sort(tmp, (m+n));
    printTab(tmp, m+n);

    int id=1;
    T3[0] = tmp[0];

    for(int i=1; i<(m+n); i++)
    {
        if(tmp[i-1] != tmp[i])
        {
            T3[id] = tmp[i];
            id++;
        }
    }
    printTab(T3, id);
}

void swap(int* a, int* b)
{
    int c = *a;
    *a = *b;
    *b = c;
}

void bubble_sort(int* tab, int N){
    for (int i = N - 1; i > 0; --i){
        int sorted = 1;
        for (int j = 0; j < i; ++j){
            if (tab[j] > tab[j + 1]){
                swap(tab + j, tab + j + 1);
                sorted = 0;
            }
        }
        if (sorted)
            return;
    }
}

void read(int* tab, int size)
{
    for(int i=0; i<size; i++)
    {
        int input = 0;
        scanf("%d", &input);
        tab[i] = input;
    }
}

void printTab(int* tab, int n)
{
    for(int i=0; i<n; i++)
        printf("%d ", tab[i]);
    printf("\n");
}

int main()
{
    int m=0, n=0;
    scanf("%d %d", &n, &m);

    int T1 = malloc(sizeof(int) * n);
    int T2 = malloc(sizeof(int) * m);

    read(T1, n);
    read(T2, m);

    int T3 = malloc(sizeof(int) * (n+m));

    unique(T1, T2, T3, n, m);

   return 0;
}
