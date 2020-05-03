#include <stdio.h>
#include <stdlib.h>
#define size 100

void printChar(char ChArr[])
{
    for( int i = 0; i<size && ChArr[i] != '\0'; i++)
        printf("%c",ChArr[i]);
}

int count(char text[size])
{
    int length = 0;
    int end = 0;

    for(int i = 0; i < size; i++)
    {
        if(text[i] == ' ' || text[i] == '\0')
        {
            if(end == 1)
                return length;
            continue;
        }

        else if((text[i] >= 'a' && text[i] <= 'z') || (text[i] >= 'A' && text[i] <= 'Z'))
        {
            length++;
            end = 1;
        }

        else
        {
            printf("Error: first word is not valid!\n");
            return 0;
        }
    }
    return length;
}

int main()
{
    char text[size];
    int i = 0;

    scanf("%[^\n]s", text);

    int step = count(text);

    while(text[i] != "\0" && step != 0)
    {
        if(text[i] == ' ')
            i++;

        else if(text[i] >= 'a' && text[i] <= 'z' && !(text[i] >= 'A' && text[i] <= 'Z'))   //for small letters
        {
            if(text[i] + step <= 'z')
                text[i] += step;

            else
                text[i] += step - 26;

            i++;
        }

        else if(text[i] >= 'A' && text[i] <= 'Z')   //for big letters
        {
            if(text[i] + step <= 'Z')
                text[i] += step;

            else
                text[i] += step - 26;

            i++;
        }

        else
            break;
    }

    printChar(text);
}
