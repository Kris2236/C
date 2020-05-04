/* Napisz program, który na wejściu otrzymuje macierz kwadratową i sprawdza, czy jest
ona trójkątna dolna. Macierz L jest dolnotrójkątna, jeżeli lij = 0 dla i < j (czyli nad
główną przekątną znajdują się elementy zerowe). */

#include <stdio.h>

printMatrix(int** M, int n)
{
    printf("\n");
    for(int i=0; i<n; i++)
    {
        for(int j=0; j<n; j++)
        {
            printf("%d ", M[i][j]);
        }

        printf("\n");
    }
}

int triangular_upper(int** M, int n)
{
    for(int i=0; i<n; i++)
    {
        for(int j=0; j<n; j++)
        {
            if(M[i][j] != 0 && j > i)
                return 0;
        }
    }

    return 1;
}

int main()
{
    int n = 0;
    scanf("%d", &n);

    int** M = malloc(sizeof(int*) * n);
    for(int i=0; i<n; i++)
    {
        M[i] = malloc(sizeof(int) * n);
    }

    for(int i=0; i<n; i++)
    {
        for(int j=0; j<n; j++)
        {
            int a = 0;
            scanf("%d", &a);
            M[i][j] = a;
        }
    }

    printMatrix(M, n);

    printf(triangular_upper(M, n) ? "YES" : "NO");

    return 0;
}
