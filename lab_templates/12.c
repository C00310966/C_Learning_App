#include <stdio.h>
void reverse_array(int *arr, int size)
{
    // TODO: swap *left and *right using pointer arithmetic only
    (void)arr;
    (void)size;
}
int main(void)
{
    int a[] = {1, 2, 3, 4, 5}, b[] = {7, 8};
    reverse_array(a, 5);
    reverse_array(b, 2);
    for (int i = 0; i < 5; i++)
        printf("%d%c", a[i], i == 4 ? '\n' : ' ');
    for (int i = 0; i < 2; i++)
        printf("%d%c", b[i], i == 1 ? '\n' : ' ');
}
