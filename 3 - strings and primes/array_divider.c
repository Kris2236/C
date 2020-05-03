#include <stdio.h>
#include <stdlib.h>
#define size 1000

int sum(int left, int right, char tab[])
{
    int result = 0;
    int i =0;
    for (i = left; i <= right; i++)
        result += (int)tab[i] - (int)'0';

    return result;
}

int main()
{
    int n;
    char tab[size];

    scanf("%d", &n);
    scanf("%s", tab);

    if(n<3) // zak³adam ¿e jeœli nie istniej¹ elementy na lewo/prawo to ich suma równie¿
    {
        printf("Error: to small array\n");
        return 0;
    }

    for(int i = 1; i < n-1; i++)
        if(sum(0, i-1, tab) == sum(i+1, n - 1, tab))
            return printf("%d\n", i);

    return 0;
}
