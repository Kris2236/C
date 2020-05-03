#include <stdio.h>

int main()
{
    int n = 0;
    scanf("%d", &n);

    int success = 1;

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            int a = 0;
            scanf("%d", &a);

            if (i < j && a != 0)
                success = 0;
        }
    }

    printf(success ? "YES" : "NO");

    return 0;
}