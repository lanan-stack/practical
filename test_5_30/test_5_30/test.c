#include <stdio.h>
#include<string.h>
int main() {
    int a, b;
    char arr[] = "";
    scanf("%c", arr);
    int sz = strlen(arr);
    int head = 0;
    int move = 0;
    for (int head = 0; head < sz; head++)
    {
        for (int move = 0; move < sz; move++)
        {
            if (arr[head] != arr[move])
            {
                return arr[head];
            }
        }
    }
    return -1;
}