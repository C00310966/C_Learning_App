#include <stdio.h>
int convertBinarytoDecimal(int binaryNumber)
{
    // TODO: read digits from right to left, weight each by 1,2,4,...
    (void)binaryNumber;
    return 0;
}
int main(void)
{
    int a[] = {0, 1, 10, 110, 111, 10101, 100000};
    for (int i = 0; i < 7; i++)
        printf("%d:%d\n", a[i], convertBinarytoDecimal(a[i]));
}
