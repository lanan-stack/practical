#define _CRT_SECURE_NO_WARNINGS 1
#include <stdio.h>
#include<string.h>

int main()
{
    //ABCD左旋一个字符得到BCDA  
//ABCD左旋两个字符得到CDAB

    /*char s[100] = "hello world";
    int k = 0;
    printf("请输入你要左旋几个字符：");
    scanf("%d", &k);
    printf("\n");
    int sz = strlen(s);
    while (k--)
    {
        int s1 = 0;
        int s2 = 1;
        char temp = s[s1];
        while (s[s2])
        {
            s[s1] = s[s2];
            s1++;
            s2++;
        }
        s[sz - 1] = temp;
    }
    printf("%s", s);*/
    // 模拟实现库函数strlen
    /*char s[20] = "hello world";
    int s1 = 0;
    int count = 0;
    int sz = strlen(s);
    while (s[s1])
    {
        count++;
        s1++;
    }
    printf("%d\n", sz);
    printf("%d\n", count);*/
   
    // 输入一个整数数组，实现一个函数，
   //来调整该数组中数字的顺序使得数组中所有的奇数位于数组的前半部分
   //所有偶数位于数组的后半部分。
    int arr[10] = { 1,2,3,4,5,6,7,8,9,10 };
    int left = 0;
    int right = sizeof(arr)/sizeof(arr[0])-1;
    int arr2[10] = { 0 };
    for (int i = 0; i < 10; i++)
    {
        if (arr[i] % 2 == 0)//偶数
        {
            arr2[right] = arr[i];
            right--;
        }
        else
        {
            arr2[left] = arr[i];
            left++;
        }
    }
    for (int i = 0; i < sizeof(arr2) / sizeof(arr2[0]); i++)
    {
        printf("%d ", arr2[i]);
    }
    return 0;
}