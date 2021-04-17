#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#define numberSize 9

int check(long int number)
{
    int end = 0;
    char tab[numberSize];

    for(int i = 0; i<numberSize && !end; i++)
    {
        if(number < 10)
            end = 1;

        tab[i] = number % 10;
        number /= 10;

        if(i>0 && tab[i-1] < tab[i])
            return 0;
    }

    return 1;
}

void prime(long int n)
{
    long int number = 2;
    int flag;

    while (number < n)
    {
        flag = 1;

        for (int i = 2; i < sqrt(number); i++)
            if (number % i == 0)
                flag = 0;

        if (flag == 1 && check(number))
            printf("%li\n", number);

        number++;
    }
}


int main()
{

    long int n;

    scanf("%li", &n);

    if(n>2)
        prime(n);

    return 0;
}
